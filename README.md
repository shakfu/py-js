# minimal py for max

An attempt to make a simple (and extensible) max external for python3

repo - https://github.com/shakfu/py


## Summary

```
globals:
    object_count: number of active py objects
    registry: global lookup for script and object names

py interpreter object
    attributes
        name:  unique name
        file:  file to load into editor
        debug: switch debug logging on and off
        patcher: parent patcher object
        box: parent box object

    messages
        import <module>     : python import to object namespace
        eval <expression>   : python 'eval' semantics
        exec <statement>    : python 'exec' semantics
        execfile <path>     : python 'execfile' semantics
        assign <var> [data] : max msg assignments to object namespace
        call (anything)     : max friendly python function calling
        read <path>         : read text file into editor
        load <path>         : combo of read <path> -> execfile <path>
        send <msg>          : send an arbitrary message to a named object
```


## Overview

The `py` object provides a very high level python code interface to max objects. It has 1 inlet and 3 outlets, with the left providing main object output, the right outlet sending a bang on success, and the middle sending a bang on failure.

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
interobj | scan            |               | n/a    | no         | [x]
interobj | send            | msg           | n/a    | no         | [x]


### Key Features

1. **Per-object namespaces**. Responds to an `import <module>` message in the left inlet which loads a python module in its namespace. Each new import adds modules to the object's namespace (essentially a `globals dict`), which can be different in each instance.

2. **Eval Messages**. Responds to an `eval <expression>` message in the left inlet which is evaluated in the context of the namespace and outputs results to the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

3. **Exec Messages**. Responds to an `exec <statement>` message and an `execfile <filepath>` message which executes the statement or the file's code in the object's namespace. This produces no output from the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

4. **Assign Messages**. Responds to an `assign <varname> [x1, x2, ..., xN]` which is equivalent to `<varname> = [x1, x2, ..., xN]` in the python namespace. This is a way of creating variables in the objects python namespace using max message syntax. This produces no output from the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

5. **Anything Messages**. Responds to any kind of messages other than those specifically implemented. Practically it can evaluate (in the `eval` sense above) a message format which is a similar to a python generic function call: `<callable> [arg1 arg2 ... arg_n] [key1=val1 key2=val2 ... keyN=valN]`. This outputs results to the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

6. **Line REPL**. The `py`has two bpatcher line `repls`, one of which embeds a `py` object and another which has an outlet to connect to one. The repls include a convenient menu with all the `py` object's methods and also feature coll-based history via arrow-up/arrow-down recall of all entries in a session. Of course, a coll can made to save all commands if required.

7. **Code Editor**. Double-clicking the `py` object opens a code-editor. This is populated by a `read` message which reads a file into the editor and saves the filepath to an attribute. A `load` message also `reads` the file followed by `execfile`. Saving the text in the editor uses the attribute filepath and execs the saved text to the object's namespace. 

8. **Exposing Max API to Python** A significant part of the max api in `c74support/max-includes` has been converted to a cython `.pxd` file called `api_max.pxd`. This makes it available for a cython implementation file, `api.pyx` which is converted to c-code during builds and is embedded in the external. This enables a custom python builtin module called `api` which can be imported by python scripts in `py` objects and also via `import` messages. What this effectively means is that python scripts in `py` objects can directly call max c-api functions.

9. **Globals Exchange**. The `py` external has a special builtin python module called `globex` which exposes globals which can be read and written from the python script side and also from the c external side.


## Building

Only tested on OS X at present. Should be relatively easy to port to windows.

The following is required:


### xcode

Not sure if full xcode is required, perhaps only the command line tools are sufficient

```
$ xcode-select --install
```

otherwise download xcode from the app store.


### py external source and maxsdk

The py external is developed as a max package with the max-sdk as a subfolder. This is incorporated as a git-module:

```
$ git clone https://github.com/shakfu/py.git
```

Then cd into the newly cloned source directory and run the following to get the max-sdk

```
$ git submodule init
$ git submodule update 

```

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

almost done

- [80%] Implement send to named objects
      (see: https://cycling74.com/forums/error-handling-with-object_method_typed)

core

- [ ] enhance `py_exec` method to create a single string from argv so it can import easily
- [x] enhance `py_anything` method to eval if identifier is not a callable yet exists in ns
- [ ] Refactor 'py_eval' to make it more consistent with the others
- [ ] Refactor conversion logic from object methods
- [ ] idea: shift type conversion to python (cython) api calls which should be easier than doing it in c

attributes & infrastructure

- [ ] add `autoload` attribute to trigger autoload (`load` msg) of code editor code
- [ ] add set/get for attributes as appropriate to trigger actions or methods calls after changes
- [ ] Add file location feature (try pkg/examples/scripts then absolute paths) for pythonpath
      ```c
      PyObject *sysPath = PySys_GetObject((char*)"path");
      PyList_Append(sysPath, PyString_FromString("."));
      ```

testing

- [ ] complete comprehensive test suite
  - [ ] complete c test suite
  - [ ] complete max test suite
  - [ ] convert `py_coll_tester` into bpatcher that can be fed by `py_repl` 
  - [ ] BUG: if attr has same name as method (the import saga), crash. fix by making them different. Investigate.


future experiments

- [ ] Convert py into a js extension class
      - proof of concept done, but requires a different 'nobox' typy of class and data passing via arrays and attributes instead of outlets. But can be done!
- [ ] try to build a cython extension types as a max external class


### DONE



## CHANGELOG


### v0.1

#### Features


Core Features

- [x] Implementation of a few high level python api functions in max (eval, exec) to allow the evaluation of python code in a python `globals` namespace associated with the py object.
- [x] Each py object has its own python 'globals' namespace and responds to the following msgs
    - [x] `import <module>`: adds module to the namespace
    - [x] `eval <expression>`: evaluate expression within the context of the namespace (cannot modify ns)
    - [x] `exec <statement>`: executes statement into the namespace (can modify ns)
    - [x] `execfile <file.py>`: executes python file into the namespace (can modify ns)
    - [x] `run <file.py>`: executes python file into the namespace (can modify ns)
- [x] Add right inlet bang after eval op ends
- [x] add third (middle) outlet which bangs on an error


Core Extra Features

- [x] Add `call (anything)` method to call python callables in a namespace


Code Editor (Usability)

- [x] Edit default with text editor
- [x] Add text edit object
    - [x] enable code to be run from editor


Line REPL (Usability)

- [x] Add line repl
    - [x] Add up-arrow last line recall (great for 'random.random()')


Extensibility

- [x] Implement section on two-way globals setting and reading (from python and c)
      in https://pythonextensionpatterns.readthedocs.io/en/latest/module_globals.html
- [x] Add bpatcher line repl
- [x] add python scripts to 'examples/scripts'
- [x] Add cythonized access to max c-api..?
- [x] Extensible by embedded cython based python extensions which can call a library of wrapped max_api functions in python code. There is a proof of concept of the python code in the namsepace calling the max api `post` function successfully.
- [x] Exposing of good portion of the max api to cython scripting


Architectural

- [x] Global object/dict/ref mgmt (so two externals can exist without Py_Finalize() causing a crash


Documentation

- [x] Add .maxref.xml to docs


Code Quality

- [x] refactor error handling code (if possible)
- [x] Refactor eval code from py_eval into a function to allow for exec and execfile or PyRun_File scenarios
- [x] Refactor into functions


Testing

- [x] pytest testing harness
- [x] make test between test_translate and test_py2 which includes references to a the struct which is missing in the former



#### Bug Fixes

- [x] space in path causes "sprintf" type debugging in execfile to crash max!
- [x] codesigning errors are due to Package being developed in Documents/...
  which causes issues. If it's a non icloud exposed folder it works ok.
- [x] make exec work! (needs globals in both slots: `PyRun_String(py_argv, Py_single_input, x->p_globals, x->p_globals)`
- [x] `import` statement in eval causes a segmentation fault.
       see: https://docs.python.org/3/c-api/intro.html exception handling example
       -> needed to changed Py_DECREF to Py_XDECREF in error handling code

