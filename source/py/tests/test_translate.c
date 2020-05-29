void handle_py_error(void)
{
    if (PyErr_Occurred()) {
        PyErr_Print();
    }
}

int py_import(char* name, PyObject* globals_dict)
{
    PyObject* x_module = NULL;

    if (name != NULL) {
        x_module = PyImport_ImportModule(name); // x_module borrrowed ref
        if (x_module == NULL) {
            goto error;
        }
        PyDict_SetItemString(globals_dict, name, x_module);
        return 0;
    }

error:
    handle_py_error();
    return -1;
}

int py_exec(char* statement, PyObject* globals_dict)
{
    PyObject* pval = NULL;

    if (statement == NULL) {
        goto error;
    }

    pval = PyRun_String(statement, Py_single_input, globals_dict,
                        globals_dict);
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

int py_execfile(char* fpath, PyObject* globals_dict)
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

    pval = PyRun_File(fhandle, fpath, Py_file_input, globals_dict,
                      globals_dict);
    if (pval == NULL) {
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

int py_run(char* fpath)
{
    PyObject* pval = NULL;
    FILE* fhandle = NULL;
    int ret = -0;

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

    ret = PyRun_SimpleFile(fhandle, py_argv);
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

PyObject* py_eval_obj(char* expression, PyObject* globals_dict)
{
    PyObject* pval = PyRun_String(expression, Py_eval_input, globals_dict,
                                  globals_dict);
    if (pval != NULL) {
        return pval;
    } else {
        handle_py_error();
        return NULL;
    }
}

long py_eval_long(char* expression, PyObject* globals_dict)
{

    PyObject* pval = NULL;

    pval = py_eval_obj(expression, globals_dict);

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
    return NULL;
}

double py_eval_double(char* expression, PyObject* globals_dict)
{

    PyObject* pval = NULL;

    pval = py_eval_obj(expression, globals_dict);

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
    return NULL;
}

char* py_eval_unicode(char* expression, PyObject* globals_dict)
{

    PyObject* pval = NULL;

    pval = py_eval_obj(expression, globals_dict);

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

long* py_eval_long_seq(char* expression, PyObject* globals_dict)
{
    long* result = NULL;
    PyObject* iter = NULL;
    PyObject* item = NULL;
    int i = 0;

    PyObject* pval = py_eval_obj(expression, globals_dict);
    if (pval != NULL) {
        if (PySequence_Check(pval)) {
            Py_ssize_t length = PySequence_Length(pval);
            result = (long*)malloc(length * sizeof(long));
            if ((iter = PyObject_GetIter(pval)) != NULL) {
                while ((item = PyIter_Next(iter)) != NULL) {
                    if (PyLong_Check(item)) {
                        result[i] = PyLong_AsLong(item);
                        i++;
                    }
                    Py_DECREF(item);
                }
            }
        }
    }
    Py_XDECREF(pval);
    return result;
}

float* py_eval_float_seq(char* expression, PyObject* globals_dict)
{
    float* result = NULL;
    PyObject* iter = NULL;
    PyObject* item = NULL;
    int i = 0;

    PyObject* pval = py_eval_obj(expression, globals_dict);
    if (pval != NULL) {
        if (PySequence_Check(pval)) {
            Py_ssize_t length = PySequence_Length(pval);
            result = (float*)malloc(length * sizeof(float));
            if ((iter = PyObject_GetIter(pval)) != NULL) {
                while ((item = PyIter_Next(iter)) != NULL) {
                    if (PyFloat_Check(item)) {
                        result[i] = (float)PyFloat_AsDouble(item);
                        i++;
                    }
                    Py_DECREF(item);
                }
            }
        }
    }
    Py_XDECREF(pval);
    return result;
}
