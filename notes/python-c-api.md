# Python C API


## Setting Module Globals in Embedded Python Extensions

see: https://pythonextensionpatterns.readthedocs.io/en/latest/module_globals.html



## Evaluation types

- `Py_eval_input` is equivalent to the built-in eval -- it evaluates an expression.
- `Py_file_input` is equivalent to exec -- It executes Python code, but does not return anything.
- `Py_single_input` evaluates an expression and prints its value -- used in the interpreter.

- lack of execfile in python 3 (see https://stackoverflow.com/questions/436198/what-is-an-alternative-to-execfile-in-python-3)

```python
execfile("somefile.py", global_vars, local_vars)

# as

with open("somefile.py") as f:
    code = compile(f.read(), "somefile.py", 'exec')
    exec(code, global_vars, local_vars)
```

## Defensive Programming
see: https://pythonextensionpatterns.readthedocs.io/en/latest/canonical_function.html

Mmmm....

```c
static PyObject *function(PyObject *arg_1) {
    PyObject *obj_a    = NULL;
    PyObject *ret      = NULL;

    goto try;
try:
    assert(! PyErr_Occurred());
    assert(arg_1);
    Py_INCREF(arg_1);

    /* obj_a = ...; */
    if (! obj_a) {
        PyErr_SetString(PyExc_ValueError, "Ooops.");
        goto except;
    }
    /* Only do this if obj_a is a borrowed reference. */
    Py_INCREF(obj_a);

    /* More of your code to do stuff with obj_a. */

    /* Return object creation, ret must be a new reference. */
    /* ret = ...; */
    if (! ret) {
        PyErr_SetString(PyExc_ValueError, "Ooops again.");
        goto except;
    }
    assert(! PyErr_Occurred());
    assert(ret);
    goto finally;
except:
    Py_XDECREF(ret);
    assert(PyErr_Occurred());
    ret = NULL;
finally:
    /* Only do this if obj_a is a borrowed reference. */
    Py_XDECREF(obj_a);
    Py_DECREF(arg_1);
    return ret;
}
```


