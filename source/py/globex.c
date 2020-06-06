#define PY_SSIZE_T_CLEAN
#include "Python.h"

const char* NAME_INT = "INT";
const char* NAME_STR = "STR";
const char* NAME_LST = "LST";
const char* NAME_TUP = "TUP";
const char* NAME_MAP = "MAP";



static PyMethodDef globex_methods[] = {
    { NULL, NULL, 0, NULL } /* Sentinel */
};

static PyModuleDef globex_module = {
    PyModuleDef_HEAD_INIT,
    "globex",
    "Examples of global values in a module.",
    -1,
    globex_methods, /* globex_methods */
    NULL,
    NULL,
    NULL,
    NULL,
};

/* Add a dict of {str : int, ...}.
 * Returns 0 on success, 1 on failure.
 */
int _add_map_to_module(PyObject* module)
{
    int ret = 0;
    PyObject* pMap = NULL;

    pMap = PyDict_New();
    if (!pMap) {
        goto except;
    }
    /* Load map. */
    if (PyDict_SetItem(pMap, PyBytes_FromString("66"), PyLong_FromLong(66))) {
        goto except;
    }
    if (PyDict_SetItem(pMap, PyBytes_FromString("123"),
                       PyLong_FromLong(123))) {
        goto except;
    }
    /* Add map to module. */
    if (PyModule_AddObject(module, NAME_MAP, pMap)) {
        goto except;
    }
    ret = 0;
    goto finally;
except:
    Py_XDECREF(pMap);
    ret = 1;
finally:
    return ret;
}

PyMODINIT_FUNC PyInit_globex(void)
{
    PyObject* m = NULL;

    m = PyModule_Create(&globex_module);

    if (m == NULL) {
        goto except;
    }
    /* Adding module globals */
    if (PyModule_AddIntConstant(m, NAME_INT, 42)) {
        goto except;
    }
    if (PyModule_AddStringConstant(m, NAME_STR, "String value")) {
        goto except;
    }
    if (PyModule_AddObject(m, NAME_TUP, Py_BuildValue("iii", 66, 68, 73))) {
        goto except;
    }
    if (PyModule_AddObject(m, NAME_LST, Py_BuildValue("[iii]", 66, 68, 73))) {
        goto except;
    }
    /* An invented convenience function for this dict. */
    if (_add_map_to_module(m)) {
        goto except;
    }

    goto finally;
except:
    Py_XDECREF(m);
    m = NULL;
finally:
    return m;
}
