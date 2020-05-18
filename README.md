# (minimal) py for max

An attempt to make a simple (and extensible) max external for python


## Features

### v0.1

- [x] implementation of a few high level python api functions in max (eval, exec) to 
	  allow the evaluation of python code in a python `globals` namespace associated with the py object.
- [x] each py object has its own python 'globals' namespace, that it behaves as virtual module or script, which responds to follows msgs
	- [x] `import <module>`: adds module to the namespace
	- [x] `eval <code> || eval @file <script>`: `eval <code> or <script>` in namespace
	- [ ] `exec <code> || exec @file <script>`: inject into namespace (i.e. python `exec`)

- [x] exensible by embedded cython based python extension to include maxapi functions in python code so you could (hypothetically) e.g. create objects, send messages, etc.. There is a proof of concept of the python code in the namsepace calling the max api `post` function successfully.
- [x] exposing of good portiong of the max api to cython scripting
- [ ] load default code
- [ ] edit default with text editor


## Building

The external is being developed  using the max-sdk-8.0.3 package (which is installed where packages should be installed in Max 8).

In my case, I just happened to be developing the `py` external as a folder in the  `msx-sdk/sources/basics` folder. Feel free to adjust the `Makefile` if your directory structure is different.

Only tested on OS X at present.


## TODO

- [ ] PyIncref and Defref borrowed references
- [ ] FIX: An 'import statement' in eval, exec or run causes a segmentation fault. see: https://docs.python.org/3/c-api/intro.html exception handling example
- [ ] Implement section on to-way globals seeting and reading (from python and c) in https://pythonextensionpatterns.readthedocs.io/en/latest/module_globals.html
- [x] refactor into functions
- [x] eval crashes with statement (e.g x=10)
- [x] make exec work!
	  Needs globals in both globals and locals param slots:
	  ```c
	  pval = PyRun_String(py_argv, Py_single_input, x->p_globals, x->p_globals); 
      ```
- [x] add line repl
- [ ] add right inlet bang after eval op ends
- [ ] add text edit object
- [ ] if attr has same name as method (the import saga), crash. fixed by making them different (should be another better way.)
- [x] add `@run <script>`
- [x] add cythonize access to max c-api..?
- [x] refactor eval code from py_eval into a function to allow for exec and execfile or PyRun_File scenarios




## Development Notes


### Python C API Patters

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




### Object Reference


It looks like `obex` is a type `hashtab` (Hash Table), which can be used for storing object references?


### Find named object

see: https://cycling74.com/forums/find-named-object-and-send-it-a-message

	I'm looking at some of the patcher scripting stuff in the api.  iterator.c is a good guide, but I do want to check if there's  a simple method similar to "getnamed" in javascript, such that I don't have to iterate through all the boxes in a patcher.
	Something along the lines of

```c
	t_object *desiredobject = jpatcher_get_namedobject("scriptedname");
```
	and you could then just pass desiredobject into the various jbox goodies.

and the answer

```c
t_max_err err;
t_object *patcher = NULL;
t_object *yourobject = NULL;

// get the patcher
err = object_obex_lookup(yourobjectpointer, gensym("#P"), &patcher);

// get the object that you're looking for.
yourobject = (t_object *)object_method(patcher, gensym("getnamedbox"), gensym("theobjectname"));
```

----

see: https://cycling74.com/forums/error-handling-with-object_method_typed

Avoising crashes when sending:

	For messages which are internally defined as A_GIMME the correct call to use is object_method_typed(). But for other messages, say one with A_FLOAT as the argument, you will likely want to use object_method(). 


----

see: https://cycling74.com/forums/messnamed-equivalent-send-function-in-c-send-to-named-object


Question: 

	"want to send values to a named object, as there is in javascript with 'messnamed'... send values to receive objects."

Answer:

	all send's and receive's with the same name reference a single nobox object called "through", which you can simply retrieving by looking at the s_thing field of the name symbol — I mean, if you need to retrieve the through object associated to the "foo" symbol, just look for gensym("foo")->s_thing.
	Once you have the object, just send it a message with object_method().

```c
t_max_err object_send_method_typed(void *x, t_symbol *name, t_symbol *s, long ac, t_atom *av, t_atom *rv)
{
   t_object *thing = name->s_thing;

   if (!thing) {
       return MAX_ERR_INVALID_PTR;
   }
   if (NOGOOD(thing)) {
       return MAX_ERR_INVALID_PTR;
   }
   if (!object_classname_compare(thing, gensym("through"))) {
       return MAX_ERR_GENERIC;
   }
   return object_method_typed(thing, s, ac, av, rv);
}
```

### Dynamic Outlets?

see: https://cycling74.com/forums/dynamic-inlets-outlets

```c
// start the transaction with the box
t_object *b = NULL;
object_obex_lookup(x, _sym_pound_B, (t_object **)&b);
object_method(b, gensym("dynlet_begin"));

// update outlets with one or both of these calls
//outlet_delete(outlet_nth((t_object*)x, i));
//outlet_append((t_object*)x, NULL, gensym("signal"));

// end the transaction with the box
object_method(b, gensym("dynlet_end"));
```

### Is a real REPL possible?

**Is it possible to launch a python interactive loop / repl from the py external?**

Ideally it would be fantastic to launch a python repl per py external instance (and of course full access to the globals namespace and the embedded cython wrapping of the max api) which would allow one to script ma via an enteractive loop (real livecoding)

- What about embedding the jupyter kernel? (tried the main python-based kernel -> caused Max to crash). Perhaps need to launch with nogil? or in a different thread or process?

- What about integrating the new jupyter c++-based kernl [xeus](https://github.com/jupyter-xeus/xeus) or its python implementation [xeus-python](https://github.com/jupyter-xeus/xeus-python)?

- What about just using `osc` via `[udpreceive]`? see [python-osc](https://github.com/attwad/python-osc) 

- Iain Duncan's [Schema for Max](https://github.com/iainctduncan/scheme-for-max) has a novel repl which work nicely. Could be another way



### Outlets

- outlet creation order is important in `outlet_new(x, NULL)`?


### Evaluation

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


### Alternative access

- websockets: https://websockets.readthedocs.io/en/stable/intro.html


