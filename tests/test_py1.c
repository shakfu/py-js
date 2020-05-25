
// void py_import(t_py *x, t_symbol *s)
int py_import(char *name, PyObject *globals_dict)
{
    PyObject* x_module = NULL;

    if (name != NULL) {
        x_module = PyImport_ImportModule(name); // x_module borrrowed ref
        if (x_module == NULL) {
            goto error;
        }
        PyDict_SetItemString(globals_dict, name, x_module);
        post("imported: %s", name);
        return 0;
    }

    error:
        if(PyErr_Occurred()) {
            PyObject *ptype, *pvalue, *ptraceback;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);
            const char *pStrErrorMessage = PyUnicode_AsUTF8(pvalue);
            error("PyException('import %s'): %s", s->s_name, pStrErrorMessage);
            Py_XDECREF(ptype);
            Py_XDECREF(pvalue);
            Py_XDECREF(ptraceback);
        }
        return -1
}


// void py_run(t_py *x, t_symbol *s, long argc, t_atom *argv)
int py_run(char *fpath)
{
    PyObject *pval = NULL;
    FILE* fhandle  = NULL;
    int ret = -0;

    if (fpath == NULL) {
        error("No filepath given.");
        goto error;
    }
    post("START run: %s", fpath);

    pval = Py_BuildValue("s", fpath); // new reference
    if (pval == NULL) {
        goto error;
    }

    fhandle = _Py_fopen_obj(pval, "r+");
    if (fhandle == NULL) {
        error("could not open file '%s'", py_argv);
        goto error;
    }

    ret = PyRun_SimpleFile(fhandle, py_argv);
    if (ret == -1){
        goto error;
    }

    // success
    fclose(fhandle);
    Py_DECREF(pval);
    post("END run: %s", fpath);
    return 0;

    error:
        if(PyErr_Occurred()) {
            PyObject *ptype, *pvalue, *ptraceback;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);
            const char *pStrErrorMessage = PyUnicode_AsUTF8(pvalue);
            error("PyException('run %s'): %s", fpath, pStrErrorMessage);
            Py_XDECREF(pval);   
            Py_XDECREF(ptype);
            Py_XDECREF(pvalue);
            Py_XDECREF(ptraceback);
        }
        return -1;
}


// void py_execfile(t_py *x, t_symbol *s, long argc, t_atom *argv)
int py_execfile(char *fpath, PyObject *globals_dict)
{
    PyObject *pval = NULL;
    FILE* fhandle  = NULL;

    if (fpath == NULL) {
        error("No filepath given.");
        goto error;
    }
    post("START execfile: %s", fpath);

    fhandle = fopen(fpath, "r");
    if (fhandle == NULL) {
        error("could not open file '%s'", fpath);
        goto error;
    }

    pval = PyRun_File(fhandle, py_argv, Py_file_input, globals_dict, globals_dict);
    if (pval == NULL) {
        goto error;
    }

    // success cleanup
    fclose(fhandle);
    Py_DECREF(pval);
    post("END execfile: %s", fpath);
    return 0;

    error:
        if(PyErr_Occurred()) {
            PyObject *ptype, *pvalue, *ptraceback;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);
            const char *pStrErrorMessage = PyUnicode_AsUTF8(pvalue);
            error("PyException('execfile %s'): %s", fpath, pStrErrorMessage);
            Py_XDECREF(pval);   
            Py_XDECREF(ptype);
            Py_XDECREF(pvalue);
            Py_XDECREF(ptraceback);
        }
        return -1;
}


int py_exec(char *statement, PyObject *globals_dict)
{
    PyObject *pval = NULL;

    if (statement == NULL) {
        error("no statement to execute");
        goto error;
    }
    post("START exec: %s", statement);

    pval = PyRun_String(statement, Py_single_input, globals_dict, globals_dict);
    if (pval == NULL) {
        goto error;
    }

    // success cleanup
    Py_DECREF(pval);
    post("END exec: %s", statement);
    return 0;

    error:
        if(PyErr_Occurred()) {
            PyObject *ptype, *pvalue, *ptraceback;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);
            const char *pStrErrorMessage = PyUnicode_AsUTF8(pvalue);
            error("PyException('exec %s'): %s", statement, pStrErrorMessage);
            Py_XDECREF(pval);   
            Py_XDECREF(ptype);
            Py_XDECREF(pvalue);
            Py_XDECREF(ptraceback);
        }
        return -1;
}

PyObject *py_eval_obj(char *expression, PyObject *globals_dict)
{
    post("eval: %s", expression);

    PyObject *locals = PyDict_New();
    PyObject *pval = PyRun_String(expression, Py_eval_input, globals_dict, locals);

    if (pval != NULL) {
        return pval;

    else {
        if (PyErr_Occurred()) {
            PyObject *ptype, *pvalue, *ptraceback;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);

            //Get error message
            const char *pStrErrorMessage = PyUnicode_AsUTF8(pvalue);
            error("PyException('eval %s'): %s", expression, pStrErrorMessage);
            Py_XDECREF(ptype);
            Py_XDECREF(pvalue);
            Py_XDECREF(ptraceback);
        }
        return NULL;
    }
}

long py_eval_long(char *expression, PyObject *globals_dict)
{
    PyObject *pval = py_eval_obj(expression, globals_dict);
    if (pval != NULL) {
        if (PyLong_Check(pval)) {
            long result = PyLong_AsLong(pval);
            return result;
        }                
    }
}


int py_eval_int(char *expression, PyObject *globals_dict)
{
    int result = (int) py_eval_long(expression, globals_dict);
    return result;
}


float py_eval_float(char *expression, PyObject *globals_dict)
{
    PyObject *pval = py_eval_obj(expression, globals_dict);
    if (pval != NULL) {
        if (PyFloat_Check(pval)) {
            float result = (float) PyFloat_AsDouble(pval);
            return result;
        }
    }
}

double py_eval_double(char *expression, PyObject *globals_dict)
{
    PyObject *pval = py_eval_obj(expression, globals_dict);
    if (pval != NULL) {
        if (PyFloat_Check(pval)) {
            double result = PyFloat_AsDouble(pval);
            return result;
        }
    }
}

char *py_eval_unicode(char *expression, PyObject *globals_dict)
{
    PyObject *pval = py_eval_obj(expression, globals_dict);
    if (pval != NULL) {
        if (PyUnicode_Check(pval)) {
            const char *result = PyUnicode_AsUTF8(pval);
            return result;
        }
    }
}

long *py_eval_long_seq(char *expression, PyObject *globals_dict)
{
    long *result = NULL;
    PyObject *iter = NULL;
    PyObject *item = NULL;
    int i = 0;

    PyObject *pval = py_eval_obj(expression, globals_dict);
    if (pval != NULL) {
        if (PySequence_Check(pval)) {
            Py_ssize_t length = PySequence_Length(pval);
            result = (long*) malloc(length * sizeof(long));
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


float *py_eval_float_seq(char *expression, PyObject *globals_dict)
{
    float *result = NULL;
    PyObject *iter = NULL;
    PyObject *item = NULL;
    int i = 0;

    PyObject *pval = py_eval_obj(expression, globals_dict);
    if (pval != NULL) {
        if (PySequence_Check(pval)) {
            Py_ssize_t length = PySequence_Length(pval);
            result = (float*) malloc(length * sizeof(float));
            if ((iter = PyObject_GetIter(pval)) != NULL) {
                while ((item = PyIter_Next(iter)) != NULL) {
                    if (PyFloat_Check(item)) {
                        result[i] = (float) PyFloat_AsDouble(item);
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



