#include "ext.h"
#include <Python.h>


static PyObject* mxgui_post(PyObject* self __attribute__((unused)),
                                PyObject* args);


static PyObject* mxgui_post(PyObject* self __attribute__((unused)),
                            PyObject* args)
{
    post("mxgui_post start");

    char* text;
    if (!PyArg_ParseTuple(args, "s", &text)) {
        post("Warning: unprintable object posted to the console from a python "
             "object.");
        return NULL;
    } else {
        post(text);
        Py_RETURN_NONE;
    }
}

static PyMethodDef mxgui_methods[] = { { "post", mxgui_post, METH_VARARGS,
                                         "Print a string to the Pd console." },
                                       { NULL, NULL, 0, NULL } };

static struct PyModuleDef mxguimodule = {
    PyModuleDef_HEAD_INIT, "mxgui", /* name of module */
    NULL,                           /* module documentation, may be NULL */
    -1, /* size of per-interpreter state of the module,
            or -1 if the module keeps state in global variables. */
    mxgui_methods
};

PyMODINIT_FUNC PyInit_mxgui(void)
{

    post("PyMODINIT_FUNC start");

    return PyModule_Create(&mxguimodule);
}
