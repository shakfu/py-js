#import <stdio.h>
#import <string.h>
#import <xpc/xpc.h>
#import <Python.h>
#include <datetime.h>

typedef enum {
    XPYC_TYPE_NONE,
    XPYC_TYPE_CONNECTION,
    XPYC_TYPE_ENDPOINT,
    XPYC_TYPE_BOOL,
    XPYC_TYPE_INT64,
    XPYC_TYPE_UINT64,
    XPYC_TYPE_DOUBLE,
    XPYC_TYPE_DATE,
    XPYC_TYPE_DATA,
    XPYC_TYPE_STRING,
    XPYC_TYPE_UUID,
    XPYC_TYPE_FD,
    XPYC_TYPE_SHMEM,
    XPYC_TYPE_ARRAY,
    XPYC_TYPE_DICTIONARY,
    XPYC_TYPE_ERROR,
    XPYC_TYPE_RICH_ERROR
} ValueType;


ValueType get_pyobject_type(PyObject* obj);
void py_init(void);
void py_free(void);
void evaluate_python(xpc_object_t reply, const char* code);


struct Memory {
    PyObject* globals;
};

struct Memory mem;



ValueType get_pyobject_type(PyObject* obj)
{
    if (PyBool_Check(obj)) {
        return XPYC_TYPE_BOOL;
    }
    else if (PyLong_Check(obj)) {
        return XPYC_TYPE_INT64;
    }
    else if (PyFloat_Check(obj)) {
        return XPYC_TYPE_DOUBLE;
    }
    else if (PyBytes_Check(obj)) {
        return XPYC_TYPE_DATA;
    }
    else if (PyUnicode_Check(obj)) {
        return XPYC_TYPE_STRING;
    }
    else if (PyTuple_Check(obj)) {
        return XPYC_TYPE_NONE;
    }
    else if (PyList_Check(obj)) {
        return XPYC_TYPE_NONE;
    }
    else if (PySet_Check(obj)) {
        return XPYC_TYPE_NONE;
    }
    else if (PyDict_Check(obj)) {
        return XPYC_TYPE_DICTIONARY;
    }
    else if (PyDateTime_Check(obj)) {
        return XPYC_TYPE_DATE;
    }
    else if (PyUnicode_Check(obj)) {
        return XPYC_TYPE_NONE;
    }
    else if (obj == Py_None) {
        return XPYC_TYPE_NONE;
    }
    else {
        return XPYC_TYPE_NONE;
    }
}


void py_init(void)
{
    static int has_run = 0;
    if (!has_run) {
        wchar_t* python_home = NULL;
        PyStatus status;

        PyConfig config;
        PyConfig_InitPythonConfig(&config);
        config.parse_argv = 0; // Disable parsing command line arguments
        config.isolated = 0;   // default is disabled
        config.home = python_home;

        status = Py_InitializeFromConfig(&config);
        if (PyStatus_Exception(status)) {
            PyConfig_Clear(&config);
            printf("could not initialize python\n");
        }

        PyConfig_Clear(&config);

        PyObject* main_mod = PyImport_AddModule("__main__"); // borrowed
        mem.globals = PyModule_GetDict(main_mod); // borrowed reference
        PyObject* builtins = PyEval_GetBuiltins(); // borrowed
        PyDict_SetItemString(mem.globals, "__builtins__", builtins);
        has_run = 1;     
    }
}

void py_free(void)
{
    Py_XDECREF(mem.globals);
    Py_Finalize();
}

void evaluate_python(xpc_object_t reply, const char* code)
{
    py_init(); // run once

    if (strcmp(code, "XPYC_EXIT") == 0) {
        // called by external_free method to cleanup python
        py_free();
        return;
    }

    PyObject* co = NULL;
    PyObject* pval = NULL;
    int is_eval = 1;

    co = Py_CompileString(code, "<string>", Py_eval_input);

    if (PyErr_ExceptionMatches(PyExc_SyntaxError)) {
        PyErr_Clear();
        co = Py_CompileString(code, "<string>", Py_file_input);
        is_eval = 0;
    }

    if (co == NULL) { // can be eval-co or exec-co or NULL here
        goto error;
    }

    pval = PyEval_EvalCode(co, mem.globals, mem.globals);
    if (pval == NULL) {
        goto error;
    }
    Py_DECREF(co);

    if (is_eval) {
        if (PyLong_Check(pval)) {
            ValueType value_type = get_pyobject_type(pval);
            long result = PyLong_AsLong(pval);
            xpc_dictionary_set_int64(reply, "result_type", (long)value_type);
            xpc_dictionary_set_int64(reply, "result", result);

        } else if(PyFloat_Check(pval)) {
            double result = PyFloat_AsDouble(pval);
            ValueType value_type = get_pyobject_type(pval);
            xpc_dictionary_set_int64(reply, "result_type", (long)value_type);
            xpc_dictionary_set_double(reply, "result", result);

        } else if(PyUnicode_Check(pval)) {
            const char* result = PyUnicode_AsUTF8(pval);
            if (result != NULL) {
                ValueType value_type = get_pyobject_type(pval);
                xpc_dictionary_set_int64(reply, "result_type", (long)value_type);
                xpc_dictionary_set_string(reply, "result", result);
            }
        }
        Py_XDECREF(pval);
    }
    else {
        // exec
        xpc_dictionary_set_int64(reply, "result_type", (long)XPYC_TYPE_NONE);
    }

    return;

error:
    xpc_dictionary_set_int64(reply, "result_type", (long)XPYC_TYPE_ERROR);
    xpc_dictionary_set_string(reply, "result", "could not eval or exec code");
}

int main(int argc, const char *argv[])
{
    xpc_rich_error_t error;
    
    xpc_listener_t listener = xpc_listener_create("xpyc.PythonService", NULL, 0, ^(xpc_session_t  _Nonnull peer) {
        
        xpc_session_set_incoming_message_handler(peer, ^(xpc_object_t  _Nonnull message) {

            const char* code = xpc_dictionary_get_string(message, "code");            
            xpc_object_t reply = xpc_dictionary_create_reply(message);

            evaluate_python(reply, code);

            xpc_rich_error_t replyError = xpc_session_send_message(peer, reply);
            if (replyError) {
                printf("Reply failed, error: %s", xpc_rich_error_copy_description(replyError));
            }

        });
    }, &error);
    
    printf("Created listener: %s", xpc_listener_copy_description(listener));
    
    // Resuming the serviceListener starts this service. This method does not return.
    dispatch_main();

    return 0;
}
