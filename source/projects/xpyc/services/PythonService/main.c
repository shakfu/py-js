#import <stdio.h>
#import <xpc/xpc.h>
#import <Python.h>
#include <datetime.h>

void evaluate_python(xpc_object_t reply, const char* code);

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

void evaluate_python(xpc_object_t reply, const char* code)
{
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
    PyObject* globals = PyModule_GetDict(main_mod); // borrowed reference
    PyObject* builtins = PyEval_GetBuiltins(); // borrowed
    PyDict_SetItemString(globals, "__builtins__", builtins);

    PyObject* pval = PyRun_String(code, Py_eval_input, globals, globals);
    long long_result = PyLong_AsLong(pval);

    ValueType value_type = get_pyobject_type(pval);
    xpc_dictionary_set_int64(reply, "result_type", (long)value_type);
    xpc_dictionary_set_int64(reply, "result", long_result);

    // printf("result: %ld\n", long_result);

    Py_XDECREF(pval);
    Py_XDECREF(globals);
    Py_Finalize();
}

int main(int argc, const char *argv[])
{
    xpc_rich_error_t error;
    
    // Set up the xpc_listener for this service. It will handle all incoming connections.
    xpc_listener_t listener = xpc_listener_create("xpyc.PythonService", NULL, 0, ^(xpc_session_t  _Nonnull peer) {
        
        // Set the incoming message handler to a block that receives the message and performs the service's task.
        xpc_session_set_incoming_message_handler(peer, ^(xpc_object_t  _Nonnull message) {
            const char* code = xpc_dictionary_get_string(message, "code");
//            int64_t firstNumber = xpc_dictionary_get_int64(message, "firstNumber");
//            int64_t secondNumber = xpc_dictionary_get_int64(message, "secondNumber");
            
            // Create a reply and send it back to the client.
            xpc_object_t reply = xpc_dictionary_create_reply(message);

            evaluate_python(reply, code);
            // xpc_dictionary_set_int64(reply, "result", firstNumber + secondNumber);
            // xpc_dictionary_set_int64(reply, "result", evaluate_python(code));
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
