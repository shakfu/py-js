# TODO


## Fix Deprecated API

```python
[ 27%] Building C object source/projects/mxpy/CMakeFiles/mxpy.dir/mxpy.c.o
/Users/sa/Downloads/projects/py-js/source/projects/mxpy/mxpy.c:391:5: warning: 'Py_SetProgramName' is deprecated [-Wdeprecated-declarations]
    Py_SetProgramName(program);
    ^
/usr/local/opt/python@3.11/Frameworks/Python.framework/Versions/3.11/include/python3.11/pylifecycle.h:37:1: note: 'Py_SetProgramName' has been explicitly marked deprecated here
Py_DEPRECATED(3.11) PyAPI_FUNC(void) Py_SetProgramName(const wchar_t *);
^
/usr/local/opt/python@3.11/Frameworks/Python.framework/Versions/3.11/include/python3.11/pyport.h:336:54: note: expanded from macro 'Py_DEPRECATED'
#define Py_DEPRECATED(VERSION_UNUSED) __attribute__((__deprecated__))
                                                     ^
/Users/sa/Downloads/projects/py-js/source/projects/mxpy/mxpy.c:396:5: warning: 'PySys_SetArgv' is deprecated [-Wdeprecated-declarations]
    PySys_SetArgv(0, &arg0);
    ^
/usr/local/opt/python@3.11/Frameworks/Python.framework/Versions/3.11/include/python3.11/sysmodule.h:13:1: note: 'PySys_SetArgv' has been explicitly marked deprecated here
Py_DEPRECATED(3.11) PyAPI_FUNC(void) PySys_SetArgv(int, wchar_t **);
^
/usr/local/opt/python@3.11/Frameworks/Python.framework/Versions/3.11/include/python3.11/pyport.h:336:54: note: expanded from macro 'Py_DEPRECATED'
#define Py_DEPRECATED(VERSION_UNUSED) __attribute__((__deprecated__))
```                                                     ^
