# (minimal) py for max

An attempt to make a simple (and extensible) max external for python


## Features

### v0.1

- [x] implementation of a few high level python api functions in max (eval, exec) to 
	    allow the evaluation of python code in a python `globals` namespace associated with the py object.
- [x] each py object has its own python 'globals' namespace, that it behaves as virtual module or script, which responds
      to following msgs
	- [x] `import <module>`: adds module to the namespace
	- [x] `eval <code> || eval @file <script>`: `eval <code> or <script>` in namespace
	- [ ] `exec <code> || exec @file <script>`: inject into namespace (i.e. python `exec`)

- [x] exensible by embedded cython based python extension to include maxapi functions in python code so you could 
      (hypothetically) e.g. create objects, send messages, etc.. There is a proof of concept of the python code in the namsepace calling the max api `post` function successfully.
- [x] exposing of good portiong of the max api to cython scripting
- [ ] load default code
- [ ] edit default with text editor


## Building

The external is being developed  using the max-sdk-8.0.3 package (which is installed where packages should be installed in Max 8).

In my case, the `py` external is developed as a project in the  `msx-sdk/sources/basics` folder. Feel free to adjust the `Makefile` if your directory structure is different.

Only tested on OS X at present.


## TODO

- [ ] apply 'repr' to eval strings 
      see `PyObject* PyObject_Repr(PyObject *o)` returns New reference.
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




