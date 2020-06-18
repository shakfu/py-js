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
        autoload: load file at start
        pythonpath: add path to python sys.path
        debug: switch debug logging on and off

    methods
        import <module>     : python import to object namespace
        eval <expression>   : python 'eval' semantics
        exec <statement>    : python 'exec' semantics
        execfile <path>     : python 'execfile' semantics
        assign <var> [data] : max msg assignments to object namespace
        call (py callable)  : max friendly python function calling
        read <path>         : read text file into editor
        load <path>         : combo of read <path> -> execfile <path>
        send <msg>          : send an arbitrary message to a named object

    inlets
        single inlet        : primary input (anything)

    outlets
        left outlet         : primary output (anything)
        middle outlet       : bang on failure
        right outlet        : bang on success 
```


## Overview

The `py` object provides a minimal, high level max interface to python modules and a high-level python interface to max objects.


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

1. **Per-object namespaces**. Each `py` object has a unique name (which can be set by the user or provided automatically), and responds to an `import <module>` message which loads a python module in its namespace (essentially a `globals` dictionary), which can be different for each instance.

2. **Eval Messages**. Responds to an `eval <expression>` message in the left inlet which is evaluated in the context of the namespace and outputs results to the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

3. **Exec Messages**. Responds to an `exec <statement>` message and an `execfile <filepath>` message which executes the statement or the file's code in the object's namespace. This produces no output from the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

4. **Assign Messages**. Responds to an `assign <varname> [x1, x2, ..., xN]` which is equivalent to `<varname> = [x1, x2, ..., xN]` in the python namespace. This is a way of creating variables in the objects python namespace using max message syntax. This produces no output from the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

5. **Call Messages**. Responds to a `call <func> arg1 arg2 ... argN` kind of message where `func` is a python callable in the py object's namespace. This corresponds to the python `callable(*args)` syntax. This makes it easier to call python functions in a max-friendly way. If the callable does not variable arguments, it will alternatively try to apply the arguments as a list i.e. `call func(args)`. Future work will try make `call` correspond to a python generic function call: `<callable> [arg1 arg2 ... arg_n] [key1=val1 key2=val2 ... keyN=valN]`. This outputs results to the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

6. **Code Messages**. Responds to a `code <expression || statement>` message. Arbitrary python code (expression or statement) can be used here, because the whole message body is converted to a string, the complexity of the code is only limited by max's parsing and excaping rules. (EXPERIMENTAL and evolving).

7. **Send Message**. Responds to a `send <object-name> <msg> <msg-body>` message. Used to send *typed* messages to any named object. Evokes a `scan` for the patcher's objects if a `registry` of names is empty.

8. **Line REPL**. The `py`has two bpatcher line `repls`, one of which embeds a `py` object and another which has an outlet to connect to one. The repls include a convenient menu with all the `py` object's methods and also feature coll-based history via arrow-up/arrow-down recall of all entries in a session. Of course, a coll can made to save all commands if required.

9. **Code Editor**. Double-clicking the `py` object opens a code-editor. This is populated by a `read` message which reads a file into the editor and saves the filepath to an attribute. A `load` message also `reads` the file followed by `execfile`. Saving the text in the editor uses the attribute filepath and execs the saved text to the object's namespace. 

10. **Exposing Max API to Python** A significant part of the max api in `c74support/max-includes` has been converted to a cython `.pxd` file called `api_max.pxd`. This makes it available for a cython implementation file, `api.pyx` which is converted to c-code during builds and embedded in the external. This enables a custom python builtin module called `api` which can be imported by python scripts in `py` objects or via `import` messages to the object. This allows the subset of the max-api which has been wrapped in cython code to be called by python scripts or via messages in a patcher.


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

- [ ] PyAtom extension is still buggy and can intermittently cause crashes
- [ ] Sending from the `api` can make max unstable. Keep it simple and investigate.

## TODO

### Core
### Extension
### Attributes & Infrastructure

- [ ] add `autoload` attribute to trigger autoload (`load` msg) of code editor code
- [ ] for `pythonpath` add file location feature (try pkg/examples/scripts then absolute
      paths)
      ```c
      PyObject *sysPath = PySys_GetObject((char*)"path");
      PyList_Append(sysPath, PyString_FromString("."));
      ```
- [ ] add set/get for attributes as appropriate to trigger actions or methods calls
      after changes (NO REASON for using this found so far)


### Testing

- [ ] complete comprehensive test suite
  - [ ] complete c test suite
  - [ ] complete max test suite
  - [ ] convert `py_coll_tester` into bpatcher that can be fed by `py_repl` 


### Future Experiments

- [ ] create new `py_anything` with heuristics to decide whether to delegate to `py_call` 
      or `py_code`.

- [ ] Convert py into a js extension class
      - proof of concept done, but requires a different 'nobox' type of class and data passing via arrays and attributes instead of outlets. But can be done!

- [ ] Try to launch an ipython shell somehow 

- [ ] try to build a cython extension type as a max external class (-:


## CHANGELOG

### v0.1

#### Features

Core Features

- [x] branch `embed-pkg`, embeds a local python install with a zipped stdlib in `support`
      already successfully tested embedding the python distro in the external itself.
- [x] made it possible to get the py object's name from any module in its namespace!
- [x] enhance `py_exec` method to create a single string from argv so it can import easily
- [x] enhance `py_anything` method to eval if identifier is not a callable yet exists in ns
- [x] Refactor 'py_eval' to make it more consistent with the others

- [x] Implementation of a few high level python api functions in max (eval, exec) to allow
      the evaluation of python code in a python `globals` namespace associated with the 
      py object.
- [x] Each py object has its own python 'globals' namespace and responds to the following
      msgs
    - [x] `import <module>`: adds module to the namespace
    - [x] `eval <expression>`: evaluate expression within the context of the namespace
          (cannot modify ns)
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

- [x] Create type conversion method in `api.pyx` which could serve python code
      and also c-code calling python code
- [x] Implement section on two-way globals setting and reading (from python and c)
      in https://pythonextensionpatterns.readthedocs.io/en/latest/module_globals.html
      (deferred for now)
- [x] Add bpatcher line repl
- [x] add python scripts to 'examples/scripts'
- [x] Add cythonized access to max c-api..?
- [x] Extensible by embedded cython based python extensions which can call a library
      of wrapped max_api functions in python code. There is a proof of concept of the
      python code in the namsepace calling the max api `post` function successfully.
- [x] Exposing of good portion of the max api to cython scripting


Architectural

- [x] Global object/dict/ref mgmt (so two externals can exist without Py_Finalize()
      causing a crash


Documentation

- [x] Add .maxref.xml to docs


Code Quality

- [x] refactor error handling code (if possible)
- [x] Refactor eval code from py_eval into a function to allow for exec and execfile
      or PyRun_File scenarios
- [x] Refactor into functions


Testing

- [x] pytest testing harness
- [x] make test between test_translate and test_py2 which includes references to a the struct which is missing in the former

#### Bug Fixes

- [x] fixed 'send' which did not have enough error checking and was crashing frequently
- [x] fixed STRANGE bug, single quotes in `py_log` cased a crash in `py_scan_callback`,
      it's based on post but post alone with the same does not cause a crash. Should simplify logging!
- [x] globex remains after all objects are freed.
      solution: `PyXDECREF x->p_globals` on `py_free`
- [x] space in `eval` without quotes will cause a crash!
- [x] space in path causes "sprintf" type debugging in execfile to crash max!
- [x] codesigning errors are due to Package being developed in Documents/...
      which causes issues. If it's a non icloud exposed folder it works ok.
- [x] make exec work! (needs globals in both slots:
     `PyRun_String(py_argv, Py_single_input, x->p_globals, x->p_globals)`
- [x] `import` statement in eval causes a segmentation fault.
       see: https://docs.python.org/3/c-api/intro.html exception handling example
       -> needed to changed Py_DECREF to Py_XDECREF in error handling code

- [x] do not give attr has same name as method (the import saga) as this will crash.
      fix by making them different.


## Prior Art and Thanks

Every now and then when I was developing a patch in Max, I yearned for some simple python function or the other, like the `any` and `all` builtins, for example, and I would then spend more time I wanted researching a Max workaround. 

Somehow, I did't really get into Javascript, so I looked around for a python external and I found Thomas Grill's [py/pyext – Python scripting objects for Pure Data and Max](https://grrrr.org/research/software/py/) which looked promising but then I read that the 'available Max port is not actively maintained.' It's written in C++ and needs an additional c++ flext layer (http://grrrr.org/ext/flext) to compile. But I was further dissuaded as it was then not very active and only python 2 which seemed difficult to swallow. Ironically, this project has become more active recently, so the above may no longer apply.

Around the time of the beginning of the covid-19 lockdown, I stumbled upon Iain Duncan's [Scheme for Max](https://github.com/iainctduncan/scheme-for-max) project, and I was quite inspired by his efforts to embed a scheme as a Max external.

So during the lockdown period, with less distractions than usual, I decided to try to make a minimal python3 external and learn the max sdk and how to write more than a few lines of c code that won't crash and also the python c-api.

I also to thank Luigi Castelli for his invaluable help in the Max forums.
