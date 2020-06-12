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
editor   | load            | file          | n/a    | no         | [x]
interobj | scan            |               | n/a    | no         | [x]
interobj | send            | msg           | n/a    | no         | [x]

### Key Features

1. **Per-object namespaces**. Responds to an `import <module>` message in the left inlet which loads a python module in its namespace. Each new import adds modules to the object's namespace (essentially a `globals dict`), which can be different in each instance. There can be many objects each with their own namespace.

2. **Eval Messages**. Responds to an `eval <expression>` message in the left inlet which is evaluated in the context of the namespace and outputs results to the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

3. **Exec Messages**. Responds to an `exec <statement>` message and an `execfile <filepath>` message which executes the statement or the file's code in the object's namespace. This produces no output from the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

4. **Assign Messages**. Responds to an `assign <varname> [x1, x2, ..., xN]` which is equivalent to `<varname> = [x1, x2, ..., xN]` in the python namespace. This is a way of creating variables in the objects python namespace using max message syntax. This produces no output from the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

5. **Anything Messages**. Responds to any kind of messages other than those specifically implemented. Practically it can evaluate (in the `eval` sense above) a message format which is a similar to a python generic function call: `<callable> [arg1 arg2 ... arg_n] [key1=val1 key2=val2 ... keyN=valN]`. This outputs results to the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

6. **Line REPL**. The `py`has two bpatcher line `repls`, one of which embeds a `py` object and another which has an outlet to connect to one. The repls include a convenient menu with all the `py` object's methods and also feature coll-based history via arrow-up/arrow-down recall of all entries in a session. Of course, a coll can made to save all commands if required.

7. **Code Editor**. Double-clicking the `py` object opens a code-editor. This is populated by a `read` message which reads a file into the editor and saves the filepath to an attribute. A `load` message also `reads` the file followed by `execfile`. Saving the text in the editor uses the attribute filepath and execs the saved text to the object's namespace. 

8. **Exposing Max API to Python** A significant part of the max api in `c74support/max-includes` has been converted to a cython `.pxd` file called `api_max.pxd`. This makes it available for a cython implementation file, `api.pyx` which is converted to c-code during builds and embedded in the external. This enables a custom python builtin module called `api` which can be imported by python scripts in `py` objects or via `import` messages to the object. This allows the subset of the max-api which has been wrapped in cython code to be called by python scripts or via messages in a patcher.


### Cython/Python Scripting Example

For example, let's assume we want to enable sending arbitrary messages to other objects in a patcher (a la `thispatcher`).

To simplify things, we want to send a list of floats to a `coll` object:

I would first import the `api` builtin module
```
[ import api]
```
followed by sending this message to the `py` object
```
[ call api.send coll1 5.1 9.2 10.8 11.5 ]
```
or from python code:

```python
import api
api.send('coll1', [5.1, 9.2, 10.8, 11.5])
```

Since I want to use the `call` method which is used for max friendly python function calls, I have to write a small wrapper module function in `api.pyx`:

```python
def send(args):
    name = args[:1] # head
    msg = args[1:]  # tail
    ext = PyExternal()
    ext.send(name, msg)
```

The `api` module has a class called `PyExternal` which encapsulates some common behaviours such as get the name of the caller and having a bunch of useful methods which call the max api directly or indirectly via the external c code.

```cython
cdef class PyExternal:
    cdef px.t_py *obj

    def __cinit__(self, bytes name=__name__.encode('utf-8')):
        self.obj = <px.t_py *>mx.object_findregistered(
            mx.CLASS_BOX, mx.gensym(name))
```

Now for the send method itself. There are at least two ways to implement this:

- As a `send` method in your external. My implementation (for the py object) is 101 lines according to `wc -l` and has the following prototype

```c
void py_send(t_py* x, t_symbol* s, long argc, t_atom* argv)
```

- As a send method in your cython `api.pyx` file so it can be called by python scripts and also via messages. I actually made 3 versions

1. A version which wraps the previously implemented `py_send` and uses a cython Atom extension type : 4 lines

```python
cdef send1(self, str name, list args):
    _args = [name] + args
    cdef PyAtom atom = PyAtom.from_list(_args)
    px.py_send(self.obj, mx.gensym(""), atom.argc, atom.argv)
```

2. A version which implements `py_send` using a cython Atom extension type: 19 lines

```python
cdef send2(self, str name, list args):
    cdef long argc = <long>len(args) + 1
    cdef mx.t_atom argv[PY_MAX_ATOMS]
    _args = [name] + args

    if argc < 1:
        self.error("no arguments given")
        return

    if argc >= PY_MAX_ATOMS - 1:
        self.error("number of args exceeded app limit")
        return

    for i, elem in enumerate(_args):
        if type(elem) == float:
            mx.atom_setfloat(&argv[i], <double>elem)
        elif type(elem) == int:
            mx.atom_setlong((&argv[i]), <long>elem)
        elif type(elem) == str:
            mx.atom_setsym((&argv[i]), mx.gensym(elem.encode('utf-8')))
        else:
            continue

    px.py_send(self.obj, mx.gensym(""), argc, argv)
```

3. A version which re-implements `py_send` in cython: 38 lines 

```python
cdef send3(self, str name, str msg, list args):
    cdef mx.t_object* obj = NULL
    cdef mx.t_symbol* msg_sym = mx.gensym(msg.encode('utf-8'))
    cdef mx.t_hashtab* registry = px.get_global_registry()
    cdef mx.t_max_err err
    cdef mx.t_atom argv[PY_MAX_ATOMS]
    cdef long argc = <long>len(args)

    if argc < 1:
        self.error("no arguments given")
        return

    if argc >= PY_MAX_ATOMS:
        self.error("number of args exceeded app limit")
        return

    for i, elem in enumerate(args):
        if type(elem) == float:
            mx.atom_setfloat(&argv[i], <double>elem)
        elif type(elem) == int:
            mx.atom_setlong((&argv[i]), <long>elem)
        elif type(elem) == str:
            mx.atom_setsym((&argv[i]), mx.gensym(elem.encode('utf-8')))
        else:
            continue

    # if registry is empty, scan it
    if (mx.hashtab_getsize(registry) == 0):
        self.log("registry empty, scanning...")
        self.scan()

    # lookup name in registry
    err = mx.hashtab_lookup(registry, mx.gensym(name.encode('utf-8')), &obj)

    if ((err != mx.MAX_ERR_NONE) or (obj == NULL)):
        self.error("no object found with name")
        return

    err = mx.object_method_typed(obj, msg_sym, argc, argv, NULL)

    if (err != mx.MAX_ERR_NONE):
        self.error("send failed")
        mx.outlet_bang(<void*>self.obj.p_outlet_middle)
    else:
        self.log("send succeeded")
        mx.outlet_bang(<void*>self.obj.p_outlet_right)
```



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

[Cython](https://cython.org) is using for wrapping the max api. You could de-couple the cython generated c code from the external and it would work fine since it is developed directly using the python c-api, but you would lose the nice feature of calling the max api from python scripts running inside py objects.

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

You will find that make will fail with 1 error, and it will be a codesigning error that is particular to Apple's process. While this can mostly ignored (unless your only focus is codesigning the external).

The issue I found is that if one develops the external in the natural place of testing (on a mac at least) in $HOME/Documents/Max 8/Packages/external and iCloud Drive is switched on then the latter interferes with the code signing step on xcodebuild.

The solution is to move the external project folder to a non iCloud drive folder (such as $HOME/Downloads for example) and then run "xattr -cr ." in the the project directory to remove the detritus (ironically which Apple's system is itself creating) and then it should succeed (provided you have your Info.plist and bundle id correctly specified). 

I've tried this twice and  and it works (for  "sign to run locally" case and for the "Development" case).

### Develop it

The coding style for this project can applied automatically during the build process with `clang-format`. On OS X, you can easily install using brew:

```
$ brew install clang-format
```

The style used in this project is specified in the `.clang-format` file.


## BUGS

- [ ] Sending from the `api` make max unstable. Keep it simple and investigate.

## TODO


core

- [ ] revisit `py_error` and `py_log` which is a source of many errors
- [ ] create new `py_anything` with heuristics to decide whether to delegate to `py_call` or `py_code`.


extension

- [ ] create type conversion method in `api.pyx` which could serve python code and also c-code calling python code

attributes & infrastructure

- [ ] add `autoload` attribute to trigger autoload (`load` msg) of code editor code
- [ ] for `pythonpath` add file location feature (try pkg/examples/scripts then absolute paths)
      ```c
      PyObject *sysPath = PySys_GetObject((char*)"path");
      PyList_Append(sysPath, PyString_FromString("."));
      ```
- [ ] add set/get for attributes as appropriate to trigger actions or methods calls after changes (NO REASON for using this found so far)

testing

- [ ] complete comprehensive test suite
  - [ ] complete c test suite
  - [ ] complete max test suite
  - [ ] convert `py_coll_tester` into bpatcher that can be fed by `py_repl` 


future experiments

- [ ] Consider local python install in `misc`
- [ ] Convert py into a js extension class
      - proof of concept done, but requires a different 'nobox' typy of class and data passing via arrays and attributes instead of outlets. But can be done!
- [ ] try to build a cython extension types as a max external class


## CHANGELOG

### v0.1

#### Features

Core Features

- [x] enhance `py_exec` method to create a single string from argv so it can import easily
- [x] enhance `py_anything` method to eval if identifier is not a callable yet exists in ns
- [x] Refactor 'py_eval' to make it more consistent with the others

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

- [x] check whether setting a normal attr name, can also set scripting name
- [x] Implement 'send' msg, which sends typed messages to (script) named objects
      (see: https://cycling74.com/forums/error-handling-with-object_method_typed)
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

- [x] fixed 'send' which did not have enough error checking and was crashing frequently
- [x] fixed STRANGE bug, single quotes in `py_log` cased a crash in `py_scan_callback`, it's based on post but post alone with the same does not cause a crash. Should simplify logging!
- [x] globex remains after all objects are freed.
      solution: `PyXDECREF x->p_globals` on `py_free`
- [x] space in `eval` without quotes will cause a crash!
- [x] space in path causes "sprintf" type debugging in execfile to crash max!
- [x] codesigning errors are due to Package being developed in Documents/...
  which causes issues. If it's a non icloud exposed folder it works ok.
- [x] make exec work! (needs globals in both slots: `PyRun_String(py_argv, Py_single_input, x->p_globals, x->p_globals)`
- [x] `import` statement in eval causes a segmentation fault.
       see: https://docs.python.org/3/c-api/intro.html exception handling example
       -> needed to changed Py_DECREF to Py_XDECREF in error handling code

- [x] do not give attr has same name as method (the import saga) as this will crash. fix by making them different.