# python c-api reference counting info

legend:

```text
n: new reference
b: borrowed reference
s: steals references
-: n/a
```

functions:

## c-api functions not using refcounting

```c
- Py_DecodeLocale()
- Py_Finalize()
- Py_Initialize()
- Py_InitializeFromConfig()
- Py_IsInitialized()
- Py_IsNone()
- Py_SetPythonHome() // PY_37
- PyByteArray_Check()
- PyBytes_Check()
- PyConfig_Clear()
- PyConfig_InitPythonConfig()
- PyDict_SetItemString()
- PyErr_Clear()
- PyErr_ExceptionMatches()
- PyErr_NormalizeException() // deprecated since 3.12
- PyFloat_AsDouble()
- PyFloat_Check()
- PyGILState_Ensure()
- PyGILState_Release()
- PyImport_AppendInittab()
- PyList_Append()
- PyList_Size()
- PyLong_AsLong()
- PyLong_Check()
- PyMem_RawFree()    // PY_37
- PySequence_Check()
- PySequence_Length()
- PyStatus_Exception()
- PyUnicode_AsUTF8() // but PyUnicode_AsUTF8String() is available
- PyUnicode_Check()
```

## c-api functions returning borrowed references

```c
b PyDict_GetItemString()
b PyErr_Occurred()
b PyEval_GetBuiltins() // deprecated since 3.13
b PyImport_AddModule()
b PyList_GetItem()
b PyModule_GetDict()
b PySys_GetObject()
```


## c-api functions returning new references

```c
n Py_CompileString()
n PyErr_Fetch()	// Deprecated since version 3.12: Use PyErr_GetRaisedException()
n PyEval_EvalCode()
n PyFloat_FromDouble()
n PyFloat_FromString() // unused
n PyImport_ImportModule()
n PyIter_Next()
n PyList_New()
n PyLong_FromLong()
n PyObject_CallFunctionObjArgs()
n PyObject_GetIter()
n PyObject_Repr()
n PyRun_File()
n PyRun_String()
n PyUnicode_FromString()
```


## Py_CLEAR

From the definition of `Py_CLEAR` in cpython:

```c
/* Safely decref `op` and set `op` to NULL, especially useful in tp_clear
 * and tp_dealloc implementations.
 *
 * Note that "the obvious" code can be deadly:
 *
 *     Py_XDECREF(op);
 *     op = NULL;
 *
 * Typically, `op` is something like self->containee, and `self` is done
 * using its `containee` member.  In the code sequence above, suppose
 * `containee` is non-NULL with a refcount of 1.  Its refcount falls to
 * 0 on the first line, which can trigger an arbitrary amount of code,
 * possibly including finalizers (like __del__ methods or weakref callbacks)
 * coded in Python, which in turn can release the GIL and allow other threads
 * to run, etc.  Such code may even invoke methods of `self` again, or cause
 * cyclic gc to trigger, but-- oops! --self->containee still points to the
 * object being torn down, and it may be in an insane state while being torn
 * down.  This has in fact been a rich historic source of miserable (rare &
 * hard-to-diagnose) segfaulting (and other) bugs.
 *
 * The safe way is:
 *
 *      Py_CLEAR(op);
 *
 * That arranges to set `op` to NULL _before_ decref'ing, so that any code
 * triggered as a side-effect of `op` getting torn down no longer believes
 * `op` points to a valid object.
 *
 * There are cases where it's safe to use the naive code, but they're brittle.
 * For example, if `op` points to a Python integer, you know that destroying
 * one of those can't cause problems -- but in part that relies on that
 * Python integers aren't currently weakly referencable.  Best practice is
 * to use Py_CLEAR() even if you can't think of a reason for why you need to.
 *
 * gh-98724: Use a temporary variable to only evaluate the macro argument once,
 * to avoid the duplication of side effects if the argument has side effects.
 *
 * gh-99701: If the PyObject* type is used with casting arguments to PyObject*,
 * the code can be miscompiled with strict aliasing because of type punning.
 * With strict aliasing, a compiler considers that two pointers of different
 * types cannot read or write the same memory which enables optimization
 * opportunities.
 *
 * If available, use _Py_TYPEOF() to use the 'op' type for temporary variables,
 * and so avoid type punning. Otherwise, use memcpy() which causes type erasure
 * and so prevents the compiler to reuse an old cached 'op' value after
 * Py_CLEAR().
 */
#ifdef _Py_TYPEOF
#define Py_CLEAR(op) \
    do { \
        _Py_TYPEOF(op)* _tmp_op_ptr = &(op); \
        _Py_TYPEOF(op) _tmp_old_op = (*_tmp_op_ptr); \
        if (_tmp_old_op != NULL) { \
            *_tmp_op_ptr = _Py_NULL; \
            Py_DECREF(_tmp_old_op); \
        } \
    } while (0)
#else
#define Py_CLEAR(op) \
    do { \
        PyObject **_tmp_op_ptr = _Py_CAST(PyObject**, &(op)); \
        PyObject *_tmp_old_op = (*_tmp_op_ptr); \
        if (_tmp_old_op != NULL) { \
            PyObject *_null_ptr = _Py_NULL; \
            memcpy(_tmp_op_ptr, &_null_ptr, sizeof(PyObject*)); \
            Py_DECREF(_tmp_old_op); \
        } \
    } while (0)
#endif
```
