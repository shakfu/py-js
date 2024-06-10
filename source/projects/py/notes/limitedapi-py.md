# Python Limited API Compliance for `py`

## Not Compliant

```c
Py_InitializeFromConfig()
PyByteArray_Check()
PyBytes_Check()
PyConfig_Clear()
PyConfig_InitPythonConfig()
PyLong_Check()
PyMem_RawFree()    // PY_37
PyRun_String()
PyStatus_Exception()
PyUnicode_AsUTF8() // but PyUnicode_AsUTF8String() is available
PyUnicode_Check()
```

## Compliant

```c
Py_CompileString()
Py_Finalize()
Py_Initialize()
Py_IsNone()
Py_SetPythonHome()
PyDict_GetItemString()
PyDict_SetItemString()
PyErr_Fetch()
PyErr_NormalizeException()
PyErr_Occurred()
PyEval_EvalCode()
PyEval_GetBuiltins()
PyFloat_AsDouble()
PyFloat_Check()
PyFloat_FromDouble()
PyGILState_Ensure()
PyGILState_Release()
PyImport_AddModule()
PyImport_AppendInittab()
PyList_Append()
PyList_New()
PyList_Size()
PyLong_AsLong()
PyLong_FromLong()
PyModule_GetDict()
PyObject_CallFunctionObjArgs()
PyObject_GetIter()
PyObject_Repr()
PySequence_Check()
PySequence_Length()
PySys_GetObject()
PyUnicode_FromString()
```
