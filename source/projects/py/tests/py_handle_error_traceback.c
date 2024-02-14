// buggy py_handle_error
// traceback part is not working reliably

/**
 * @brief Generic python error handler with tracebacks
 *
 * @param x pointer to object struct
 * @param fmt format string
 * @param ... other args
 *
 * see: https://stackoverflow.com/questions/56430908/cpython-print-traceback
 */

void py_handle_error(t_py* x, char* fmt, ...)
{

    if (!PyErr_Occurred())
        return;

    // char msg[PY_MAX_ELEMS];
    char msg[PY_MAX_ELEMS];

    va_list va;
    va_start(va, fmt);
    vsnprintf(msg, PY_MAX_ELEMS, fmt, va);
    va_end(va);

    // get error info
    PyObject *ptype, *pvalue, *ptraceback;
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);
    Py_XDECREF(ptype);

    PyObject* pvalue_pstr = PyObject_Repr(pvalue);
    const char* pvalue_str = PyUnicode_AsUTF8(pvalue_pstr);
    Py_XDECREF(pvalue);
    Py_XDECREF(pvalue_pstr);

    if (ptraceback != NULL && PyTraceBack_Check(ptraceback)) {
        char traceback_str[PY_MAX_ERROR];
        char linebuffer[PY_MAX_ELEMS];

        PyTracebackObject* trace_root = (PyTracebackObject*)ptraceback;
        PyTracebackObject* ptrace = trace_root;

        while (ptrace != NULL) {
            PyFrameObject* frame = ptrace->tb_frame;
            if (!frame)
                goto no_traceback;
            PyCodeObject* code = PyFrame_GetCode(frame);
            if (!code)
                goto no_traceback;

            int linenum = PyFrame_GetLineNumber(frame);
            const char* codename = PyUnicode_AsUTF8(code->co_name);
            const char* filename = PyUnicode_AsUTF8(code->co_filename);

            snprintf_zero(linebuffer, PY_MAX_ELEMS, "at %s (%s:%d); \n",
                          codename, filename, linenum);
            strncat_zero(traceback_str, linebuffer, PY_MAX_ERROR);
            ptrace = ptrace->tb_next;
        }
        object_error((t_object*)x, "[ERROR] (%s) %s: %s\n%s",
                     x->p_name->s_name, msg, pvalue_str, traceback_str);
        Py_XDECREF(ptraceback);
        return;
    }

no_traceback:
    object_error((t_object*)x, "[ERROR] (%s) %s: %s", x->p_name->s_name, msg,
                 pvalue_str);
    Py_XDECREF(ptraceback);
}
