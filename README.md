# (minimal) py for max

An attempt to make a simple (and extensible) max external for python3

repo - https://github.com/shakfu/py


## Overview

The `py` object provides a very high level python code interface to max objects.
It has 1 inlet and 2 outlets

It provides the following methods:


category | method          | param(s)      | in/out | changes ns | done 
:------- | :-------------- | :------------ | :----: | :--------: | :--:
core     | import          | module        | in     | yes        | [x]
core     | eval            | expression    | out    | no         | [x]
core     | exec            | statement     | in     | yes        | [x]
core     | execfile        | file          | in     | yes        | [x]
extra    | assign          | var, data     | in     | yes        | [x]
extra    | call (anything) | var(s), data  | out    | no         | [x]
editor   | read            | file          | n/a    | no         | [x]
editor   | load            | file          | yes    | no         | [x]


Basic Features

1. Per-object namespaces. It responds to an `import <module>` message in
   the left inlet which loads a python module in its namespace. Each new
   import (like python) adds to the namespace.

2. Eval Messages. It responds to an `eval <expression>` message in the
   left inlet which is evaluated in the context of the namespace and outputs 
   results to the left outlet and outputs a bang from the right outlet to signal
   end of evaluation.

3. Executing code string or a file into a namespace (modifying it)


```
    globals:
        object_count: number of active py objects
        registry: lookup for script and objects names

    py interpreter object
        attributes
            name:  unique name
            file:  file to load into editor
            debug: switch debug logging on and off

            (planned)
            patcher: parent patcher object
            box: box object??

        messages
            import <module>   : python import to object namespace
            eval <expression> : python 'eval' semantics
            exec <statement>  : python 'exec' semantics
            execfile <path>   : python 'execfile' semantics 
            read <path>       : read text file into editor
            load <path>       : combo of read <path> -> execfile <path>
```



## Building

Only tested on OS X at present. Should be relatively easy to port to windows.

The following is required:


### xcode

Not sure if full xcode is required, perhaps with only the command line tools)

```
$ xcode-select --install
```

otherwise download xcode from the app store.


### max-sdk

The py external is developed as a max package with the max-sdk as a subfolder. This should really be incorporated as a git-module, but that's not implemented right now, so just download the [max-sdk](https://github.com/Cycling74/max-sdk.git) as `maxsdk` (i.e. without a dash in the name) into the source directory of this package:

- py
  - docs
  - ...
  - source
    - **maxsdk**
    - py


### python3

I'm using MacOS Mojave 10.14.6, and the latest python3 version which can be installed as follows:

```
$ brew install python
```

see: https://installpython3.com/mac


### cython (optional)

[Cython](https://cython.org) is using for wrapping the max api. You could de-couple the cython generated c code from the external and it would work fine since it developed directly using the python c-api, but you would lose the nice feature of calling the max api from python scripts.

Install cython as follows:

```
pip install cython

```

### Build it

In the root of the package:

```
make -C source/py build
```

or in the `py/sources/py` directory

```
make build
```

### Sidenote about building on a Mac

You will find that make will fail with 1 error, and it will be a codesigning error that is particular to Apple's process. You can ignore this error (provided there areno other errors), since since infuriatingly this error appears and disapears when it feels like. You will be surprised that the same code that produced a codesign error builds successfully without one!


### Develop it

The coding style for this project can applied automatically during the build process with `clang-format`. On OS X, you can easily install using brew:

```
$ brew install clang-format
```

The style used in this project is specified in the `.clang-format` file.



## BUGS

- [ ] space in `eval` without quotes will cause a crash!


## TODO

- [ ] Add file location feature (try pkg/examples/scripts then absolute paths)
- [ ] Refactor 'py_eval' to make it more consistent with the others
- [ ] Refactor conversion logic from object methods
- [ ] Add bpatcher line repl
- [ ] Autoload default code

- [ ] Check out the reference for 'thispatcher'
- [ ] Implement send to named objects 
      (see: https://cycling74.com/forums/error-handling-with-object_method_typed)
- [ ] Implement section on two-way globals setting and reading (from python and c)
      in https://pythonextensionpatterns.readthedocs.io/en/latest/module_globals.html

- [ ] If attr has same name as method (the import saga), crash. fixed by making them different.

- [ ] try to build a cython extension types as a max external class
- [x] Convert py into a js extension class
      - proof of concept done, but requires a different 'nobox' typy of class and data passing via arrays and attributes instead of outlets. But can be done!

### Done

- [x] Add `call (anything)` method to call python callables in a namespace
- [x] add python scripts to 'examples/scripts'
- [x] Add .maxref.xml to docs
- [x] Add right inlet bang after eval op ends
- [x] refactor error handling code (if possible)
- [x] `import` statement in eval causes a segmentation fault.
       see: https://docs.python.org/3/c-api/intro.html exception handling example
       -> needed to changed Py_DECREF to Py_XDECREF in error handling code
- [x] Global object/dict/ref mgmt (so two external can exist without Py_Finalize() causing a crash
- [x] Add text edit object
    - [x] enable code to be run from editor
- [x] Add cythonized access to max c-api..?
- [x] Refactor eval code from py_eval into a function to allow for exec and execfile or PyRun_File scenarios
- [x] Add line repl
    - [x] Add up-arrow last line recall (great for 'random.random()')
- [x] Refactor into functions
- [x] make exec work! (needs globals in both slots: `PyRun_String(py_argv, Py_single_input, x->p_globals, x->p_globals)`



## CHANGELOG

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
- [x] Edit default with text editor




