# (minimal) py for max

An attempt to make a simple (and extensible) max external for python


## Overview
```

    py.c - basic experiment in minimal max object for calling python code

    repo - github.com/shakfu/py

    This object has 1 inlet and 2 outlets

    Basic Features

    1.  Per-object namespaces. It responds to an 'import <module>' message in
        the left inlet which loads a python module in its namespace. Each new
        import (like python) adds to the namespace.

    2.  Eval Messages. It responds to an 'eval <expression>' message in the
        left inlet  which is evaluated in the namespace and outputs results
        to the left outlet and outputs a bang from the right outlet to signal
        end of evaluation.

    py interpreter object
        attributes
            imports
            code

        messages
            import <module> [adds to @imports]
            eval <code> or eval @file <path>
            exec <code> or exec @file <path>
            run  <code> or run  @file <path>

            (phase 2)
            load file <path> -> into code (for persistence) and texeditor edits

            (phase N)
            embed ipython kernel? (-;


```



## Features

### v0.1

- [x] Implementation of a few high level python api functions in max (eval, exec) to allow the evaluation of python code in a python `globals` namespace associated with the py object.
- [x] Each py object has its own python 'globals' namespace and responds to the following msgs
	- [x] `import <module>`: adds module to the namespace
	- [x] `eval <expression>`: evaluate expression within the context of the namespace (cannot modify ns)
	- [x] `exec <statement>`: executes statement into the namespace (can modify ns)
	- [x] `execfile <file.py>`: executes python file into the namespace (can modify ns)
	- [x] `run <file.py>`: executes python file into the namespace (can modify ns)

- [x] Extensible by embedded cython based python extensions which can call a library of wrapped max_api functions in python code. There is a proof of concept of the python code in the namsepace calling the max api `post` function successfully.
- [x] Exposing of good portion of the max api to cython scripting
- [ ] Autoload default code
- [ ] Edit default with text editor


## Directory Structure

- py
	- clippings
	- docs
		- refpages (.maxref.xml)
	- externals (.mxo)
	- extras
	- fonts (.otf)
	- help (.maxhelp)
	- interfaces (svg)
	- jsextensions (.mxo / .js)
	- jsui (.js)
	- media (.png/...)
	- object-icons (.svg)
	- object-prototypes (.maxproto)
	- patchers
	- snippets (.maxsnip)
	- styles (.maxstyle)
	- templates



## Building

The external is being developed using the max-sdk-8.0.3 package (which is installed where packages should be installed in Max 8).

In my case, the `py` external is developed as a project in the `msx-sdk/sources/basics` folder. Feel free to adjust the `Makefile` if your directory structure is different.

Only tested on OS X at present.


## TODO

- [x] `import` statement in eval, exec or run causes a segmentation fault.
       see: https://docs.python.org/3/c-api/intro.html exception handling example
- [ ] Convert py into a js extension class
- [ ] Refactor conversion logic from object methods
- [ ] Check out the reference for 'thispatcher'
- [ ] Implement section on two-way globals setting and reading (from python and c)
	  in https://pythonextensionpatterns.readthedocs.io/en/latest/module_globals.html
- [ ] send example (see: https://cycling74.com/forums/error-handling-with-object_method_typed)
- [ ] Add right inlet bang after eval op ends
- [ ] If attr has same name as method (the import saga), crash. fixed by making them different.

## Done

- [x] Global object/dict/ref mgmt (so two external can exist without Py_Finalize() causing a crash
- [x] Add text edit object
	- [ ] enable code to be run from editor
- [x] Add `@run <script>`
- [x] Add cythonize access to max c-api..?
- [x] Refactor eval code from py_eval into a function to allow for exec and execfile or PyRun_File scenarios
- [x] Add line repl
	- [x] Add up-arrow last line recall (great for 'random.random()')
- [x] Refactor into functions
- [x] make exec work! (needs globals in both slots: `PyRun_String(py_argv, Py_single_input, x->p_globals, x->p_globals)`


