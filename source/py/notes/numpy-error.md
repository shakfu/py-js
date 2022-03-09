# numpy error

The following error is given when using numpy with statically compiled python3 externals.

```
[py __main__] import numpy: ImportError('
IMPORTANT: PLEASE READ THIS FOR ADVICE ON HOW TO SOLVE THIS ISSUE!\n\nImporting the numpy C-extensions failed. This error can happen for\nmany reasons, often due to issues with your setup or how NumPy was\ninstalled.\n\nWe have compiled some common reasons and troubleshooting tips at:\n\n    https://numpy.org/devdocs/user/troubleshooting-importerror.html

Please note and check the following:

* The Python version is: Python3.9 from "/Applications/Studio/Max.app/Contents/MacOS/Max"
* The NumPy version is: "1.22.3"

and make sure that they are the versions you expect.

Please carefully study the documentation linked above for further help.

Original error was: dlopen(~/Downloads/projects/py-js/externals/py.mxo/Contents/Resources/lib/python3.9/site-packages/numpy/core/_multiarray_umath.cpython-39-darwin.so, 2): Symbol not found: _PyBaseObject_Type
Referenced from: ~/Downloads/projects/py-js/externals/py.mxo/Contents/Resources/lib/python3.9/site-packages/numpy/core/_multiarray_umath.cpython-39-darwin.so
Expected in: flat namespace\n in ~/Downloads/projects/py-js/externals/py.mxo/Contents/Resources/lib/python3.9/site-packages/numpy/core/_multiarray_umath.cpython-39-darwin.so
')
```


## Possible Causes

- static compilation of python library


## References

- [this forum post](https://www.tutorialfor.com/questions-309157.htm)

- [What does rdynamic doe](https://stackoverflow.com/questions/36692315/what-exactly-does-rdynamic-do-and-when-exactly-is-it-needed)