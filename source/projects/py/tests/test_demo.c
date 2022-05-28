#include <Python.h>


void py_handle_error()
{
    if (PyErr_Occurred()) {

        // get error info
        PyObject *ptype, *pvalue, *ptraceback;
        PyErr_Fetch(&ptype, &pvalue, &ptraceback);
        PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);
        Py_XDECREF(ptype);

        PyObject* pvalue_pstr = PyObject_Repr(pvalue);
        const char* pvalue_str = PyUnicode_AsUTF8(pvalue_pstr);
        Py_XDECREF(pvalue);
        Py_XDECREF(pvalue_pstr);
        Py_XDECREF(ptraceback);

        puts(pvalue_str);
    }
}

int main(int argc, char* argv[])
{

    wchar_t *program = Py_DecodeLocale(argv[0], NULL);
    if (program == NULL) {
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
        exit(1);
    }

    Py_SetProgramName(program);
    Py_Initialize();

    PyObject *globals = PyDict_New();
    PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());

    if (argc > 1 && argv[1] != NULL) {
        PyObject *pval = PyRun_String(argv[1], Py_single_input, globals, globals);
        if (pval == NULL) {
            py_handle_error();
        } else {
            Py_XDECREF(pval);
        }
    } else {
        puts("usage: test_demo <py_code>\n");
    }

    Py_Finalize();
    PyMem_RawFree(program);
    return 0;
}

