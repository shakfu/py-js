
/* python */
#define PY_SSIZE_T_CLEAN
#include <Python.h>

/* --------------------------------------- */
// types

typedef struct _py {
    PyObject* p_globals;
} t_py;


#define PY_MAX_LONG 256
#define PY_MAX_FLOAT PY_MAX_LONG

/* --------------------------------------- */
// forward func declarations

int py_import(t_py* x, char* args);
// int py_eval(t_py* x, char* args);
int py_exec(t_py* x, char* args);
int py_execfile(t_py* x, char* args);
int py_run(t_py* x, char* args);

int main(int argc, char* argv[])
{
    t_py obj = { .p_globals = NULL };
    t_py* x = &obj;

    Py_Initialize();

    // python init
    PyObject* module = PyImport_AddModule("__main__"); // borrowed reference
    x->p_globals = PyModule_GetDict(module); // borrowed reference

    if (argc > 2) {
        if (strcmp(argv[1], "import") == 0)
            py_import(x, argv[2]);
        // if (strcmp(argv[1], "eval") == 0)
        //     py_eval(x, argv[2]);
        if (strcmp(argv[1], "exec") == 0)
            py_exec(x, argv[2]);
        if (strcmp(argv[1], "execfile") == 0)
            py_execfile(x, argv[2]);
        if (strcmp(argv[1], "run") == 0)
            py_run(x, argv[2]);
    } else {
        printf("usage: test [import, eval, exec, execfile, run] args\n");
    }

    Py_FinalizeEx();
    return 0;
}


void handle_py_error(void)
{
    if (PyErr_Occurred()) {
        PyErr_Print();
    }
}

int py_import(t_py* x, char* name)
{
    PyObject* x_module = NULL;

    if (name != NULL) {
        x_module = PyImport_ImportModule(name); // x_module borrrowed ref
        if (x_module == NULL) {
            goto error;
        }
        PyDict_SetItemString(x->p_globals, name, x_module);
        return 0;
    }

error:
    handle_py_error();
    return -1;
}

int py_exec(t_py* x, char* statement)
{
    PyObject* pval = NULL;

    if (statement == NULL) {
        goto error;
    }

    pval = PyRun_String(statement, Py_single_input, x->p_globals,
                        x->p_globals);
    if (pval == NULL) {
        goto error;
    }

    Py_DECREF(pval);
    return 0;

error:
    handle_py_error();
    Py_XDECREF(pval);
    return -1;
}

int py_execfile(t_py* x, char* fpath)
{
    PyObject* pval = NULL;
    FILE* fhandle = NULL;

    if (fpath == NULL) {
        goto error;
    }

    fhandle = fopen(fpath, "r");
    if (fhandle == NULL) {
        goto error;
    }

    pval = PyRun_File(fhandle, fpath, Py_file_input, x->p_globals,
                      x->p_globals);

    if (pval == NULL) {
        fclose(fhandle);
        goto error;
    }

    // success cleanup
    fclose(fhandle);
    Py_DECREF(pval);
    return 0;

error:
    handle_py_error();
    Py_XDECREF(pval);
    return -1;
}


int py_run(t_py* x, char* fpath)
{
    PyObject* pval = NULL;
    FILE* fhandle = NULL;
    int ret = 0;

    if (fpath == NULL) {
        goto error;
    }

    pval = Py_BuildValue("s", fpath); // new reference
    if (pval == NULL) {
        goto error;
    }

    fhandle = _Py_fopen_obj(pval, "r+");
    if (fhandle == NULL) {
        goto error;
    }

    ret = PyRun_SimpleFile(fhandle, fpath);
    if (ret == -1) {
        goto error;
    }

    // success
    fclose(fhandle);
    Py_DECREF(pval);
    return 0;

error:
    handle_py_error();
    Py_XDECREF(pval);
    return -1;
}


PyObject* py_eval_obj(t_py* x, char* expression)
{
    PyObject* pval = PyRun_String(expression, Py_eval_input, x->p_globals,
                                  x->p_globals);
    if (pval != NULL) {
        return pval;
    } else {
        handle_py_error();
        return NULL;
    }
}


long py_eval_long(t_py* x, char* expression)
{

    PyObject* pval = NULL;

    pval = py_eval_obj(x, expression);

    if (pval == NULL) {
        goto error;
    }

    if (!PyLong_Check(pval)) {
        goto error;
    }

    long result = PyLong_AsLong(pval);

    Py_XDECREF(pval);
    return result;

error:
    handle_py_error();
    Py_XDECREF(pval);
    return -1;
}


double py_eval_double(t_py* x, char* expression)
{

    PyObject* pval = NULL;

    pval = py_eval_obj(x, expression);

    if (pval == NULL) {
        goto error;
    }

    if (!PyFloat_Check(pval)) {
        goto error;
    }

    double result = PyFloat_AsDouble(pval);

    Py_XDECREF(pval);
    return result;

error:
    handle_py_error();
    Py_XDECREF(pval);
    return -1;
}


const char* py_eval_unicode(t_py* x, char* expression)
{
    PyObject* pval = NULL;

    pval = py_eval_obj(x, expression);

    if (pval == NULL) {
        goto error;
    }

    if (!PyUnicode_Check(pval)) {
        goto error;
    }

    const char* result = PyUnicode_AsUTF8(pval);

    Py_XDECREF(pval);
    return result;

error:
    handle_py_error();
    Py_XDECREF(pval);
    return NULL;
}

// caller must free
long* py_eval_long_seq(t_py* x, char* expression)
{
    long *result;
    PyObject* iter = NULL;
    PyObject* item = NULL;
    int i = 0;

    PyObject* pval = py_eval_obj(x, expression);
    if (pval == NULL) {
        goto error;
    }

    if (!PySequence_Check(pval)) {
        goto error;
     }

    Py_ssize_t length = PySequence_Length(pval);
    if (length == -1) {
        goto error;
    }

    result = (long*)malloc(length * sizeof(long));
    if (result == NULL) {
        goto error;
    }

    if ((iter = PyObject_GetIter(pval)) != NULL) {
        while ((item = PyIter_Next(iter)) != NULL) {
            if (PyLong_Check(item)) {
                if (PyLong_AsLong(item) != -1) {
                    result[i] = PyLong_AsLong(item);
                    i++;              
                }    
            }
            Py_DECREF(item);
        }
    }

    // success
    Py_XDECREF(pval);
    return result; // caller must free

    error:
        handle_py_error();
        Py_XDECREF(pval);
        return NULL;
}

// caller must free
float* py_eval_float_seq(t_py* x, char* expression)
{
    float *result;
    PyObject* iter = NULL;
    PyObject* item = NULL;
    int i = 0;

    PyObject* pval = py_eval_obj(x, expression);
    if (pval == NULL) {
        goto error;
    }

    if (!PySequence_Check(pval)) {
        goto error;
    }

    Py_ssize_t length = PySequence_Length(pval);
    if (length == -1) {
        goto error;
    }

    result = (float*)malloc(length * sizeof(float));
    if (result == NULL) {
        goto error;
    }

    if ((iter = PyObject_GetIter(pval)) != NULL) {
        while ((item = PyIter_Next(iter)) != NULL) {
            if (PyFloat_Check(item)) {
                if (PyFloat_AsDouble(item) != -1) {
                    result[i] = (float)PyFloat_AsDouble(item);
                    i++;              
                }    
            }
            Py_DECREF(item);
        }
    }

    // success
    Py_XDECREF(pval);
    return result; // caller must free

    error:
        handle_py_error();
        Py_XDECREF(pval);
        return NULL;
}

