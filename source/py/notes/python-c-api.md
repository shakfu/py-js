# Python C API

## Good Example of Embedding

https://gist.github.com/nad2000/9f69c5096e10c34acddb




## Calling Methods from Pythons C API with Keywords

# https://lists.gt.net/python/dev/573726


/* Equivalent to PyObject_CallMethod but accepts keyword args. The 
format... arguments should produce a dictionary that will be passed 
as keyword arguments to obj.method. 

```c
/*
Usage example: 
PyObject *res = call_method(lst, "sort", "{s:O}", "key", keyfun)); 
*/ 

PyObject * 
call_method(PyObject *obj, const char *methname, char *format, ...) 
{ 
va_list va; 
PyObject *meth = NULL, *args = NULL, *kwds = NULL, *ret = NULL; 

args = PyTuple_New(0); 
if (!args) 
goto out; 
meth = PyObject_GetAttrString(obj, methname); 
if (!meth) 
goto out; 

va_start(va, format); 
kwds = Py_VaBuildValue(format, va); 
va_end(va); 
if (!kwds) 
goto out; 

ret = PyObject_Call(meth, args, kwds); 
out: 
Py_XDECREF(meth); 
Py_XDECREF(args); 
Py_XDECREF(kwds); 
return ret; 
} 
```



## Build Arguments to `PyObject_CallObject` Dynamically

see: https://stackoverflow.com/questions/55987022/when-extending-python-with-c-how-do-one-dynamically-build-a-complex-structure-i


You're not supposed to use those functions to build dynamically-sized data structures. The FAQ says to use `PyTuple_Pack` instead of Py_BuildValue for an arbitrary-sized tuple, but that's wrong too; I don't know why it says that. `PyTuple_Pack` has the same varargs issues as `Py_BuildValue.`

To build a variable-length tuple from C, use `PyTuple_New` to construct an uninitialized tuple of the desired length, then loop over the indices and set the elements with `PyTuple_SET_ITEM`. Yes, this mutates the tuple. `PyTuple_SET_ITEM` is only safe to use to initialize fresh tuples that have not yet been exposed to other code. Also, be aware that `PyTuple_SET_ITEM` steals a reference to the new element.

For example, to build a tuple of integers from 0 to n-1:

```c
PyObject *tup = PyTuple_New(n);
for (int i = 0; i < n; i++) {
    // Note that PyTuple_SET_ITEM steals the reference we get from PyLong_FromLong.
    PyTuple_SET_ITEM(tup, i, PyLong_FromLong(i));
}
```

To build a variable-length list from C, you can do the same thing with `PyList_New` and `PyList_SET_ITEM`, or you can construct an empty list with `PyList_New(0)` and append items with `PyList_Append`, much like you would use `[]` and append in Python if you didn't have list comprehensions or sequence multiplication.



## What stdlib includes are included in `<Python.h>`

```
<stdio.h>, <string.h>, <errno.h>, <limits.h>, <assert.h> and <stdlib.h>
```

## How do I call an object’s method from C?

see: https://docs.python.org/3/faq/extending.html#how-do-i-call-an-object-s-method-from-c

The PyObject_CallMethod() function can be used to call an arbitrary method of an object. The parameters are the object, the name of the method to call, a format string like that used with Py_BuildValue(), and the argument values:

```c
PyObject *
PyObject_CallMethod(PyObject *object, const char *method_name,
                    const char *arg_format, ...);
```

This works for any object that has methods – whether built-in or user-defined. You are responsible for eventually Py_DECREF()’ing the return value.

To call, e.g., a file object’s “seek” method with arguments 10, 0 (assuming the file object pointer is “f”):

```c
res = PyObject_CallMethod(f, "seek", "(ii)", 10, 0);
if (res == NULL) {
        ... an exception occurred ...
}
else {
        Py_DECREF(res);
}
```
Note that since `PyObject_CallObject()` always wants a tuple for the argument list, to call a function without arguments, pass “()” for the format, and to call a function with one argument, surround the argument in parentheses, e.g. “(i)”.

## Redirecting stdout to a variable


see: https://docs.python.org/3/faq/extending.html#how-do-i-catch-the-output-from-pyerr-print-or-anything-that-prints-to-stdout-stderr

In Python code, define an object that supports the write() method. Assign this object to `sys.stdout` and `sys.stderr`. Call `print_error`, or just allow the standard traceback mechanism to work. Then, the output will go wherever your write() method sends it.

The easiest way to do this is to use the io.StringIO class:

```python
>>>
>>> import io, sys
>>> sys.stdout = io.StringIO()
>>> print('foo')
>>> print('hello world!')
>>> sys.stderr.write(sys.stdout.getvalue())
foo
hello world!
```

also see: https://stackoverflow.com/questions/4307187/how-to-catch-python-stdout-in-c-code

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

