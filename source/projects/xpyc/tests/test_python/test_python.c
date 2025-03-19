
#include <stdio.h>
#include <Python.h>

int main()
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

    PyObject* pval = PyRun_String("1+1", Py_eval_input, globals, globals);

    long long_result = PyLong_AsLong(pval);

    printf("result: %ld\n", long_result);

	Py_XDECREF(pval);
	Py_XDECREF(globals);
    return Py_FinalizeEx();
}
