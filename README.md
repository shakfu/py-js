# py/js: python3 objects for max

Simple (and extensible) python3 externals for max

repo - https://github.com/shakfu/py-js


## Summary

This is an pre-alpha level project which provides two max externals:


###1. `py` external

```
globals
    obj_count                    : number of active py objects
    registry                     : global registry to lookup object names

patchers
    subpatchers
        py_repl                  : a basic single line repl for py
        py_repl_plus             : embeds a py object in a py_repl

py max external
    attributes
        name                     : unique object name
        file                     : file to load into editor
        autoload                 : load file at start
        pythonpath               : add path to python sys.path
        debug                    : switch debug logging on/off

    methods (messages) 
        core
            import <module>      : python import to object namespace
            eval <expression>    : python 'eval' semantics
            exec <statement>     : python 'exec' semantics
            execfile <path>      : python 'execfile' semantics
        
        extra
            assign <var> [arg]   : max msg assignments to py object namespace
            call <pyfunc> [arg]  : max friendly python function calling
            pipe <arg> [pyfunc]  : process a py/max value via a pipe of py funcs
            code <expr|stmt>     : alternative way to eval or exec py code
            anything <expr|stmt> : anything version of the code method 
        
        code editor
            read <path>          : read text file into editor
            load <path>          : combo of read <path> -> execfile <path>
     
        interobject
            scan                 : scan patcher and store names of child objects
            send <msg>           : send an arbitrary message to a named object

        meta
            count                : give a int count of current live py objects

    inlets
        single inlet             : primary input (anything)

    outlets
        left outlet              : primary output (anything)
        middle outlet            : bang on failure
        right outlet             : bang on success 
```

###2. `pyjs` external (experimental)

```
pyjs max external (jsextension)
    attributes
        name                     : unique object name
        file                     : file to load in object namespace
        pythonpath               : add path to python sys.path
        debug                    : switch debug logging on/off
    
    methods
        core
            code <expr|stmt>     : eval/exec/import python code (see above)
            execfile <path>      : python 'execfile' semantics
            exec <stmnt>         : python 'exec' semantics
            eval <expression>    : python 'eval' semantics

        in-code
            eval_to_json <expr>  : python 'eval' returns json


```

## Overview

`py/js`, started out as an attempt (during a covid-19 lockdown), to get a develop a basic python3 external for max, and then evolved into a more ambitious framework for using python3 in max.

There are two implementation 'flavours':

1. The `py` external provides a minimal, high level two-way interface between max and python in a way that feels natural to both languages.

2. The `pyjs` max jsextension  provides a `PyJS` class for the `js` object, which makes available a python intepreter which works nicely with javascript code. This 'flavour' is newer, and less documented.

In addition there are essentially 3 deployment variations:

1. `py` external or `pyjs` jsextension which links to a system installed python (homebrew and built from source). This has the benefit of re-using your existing python modules. This is the default option.

2. `py` external or `pyjs` jsextension in a Max package: in this variation, a dedicated python distribution (zipped or otherwise is placed in the `support` folder of the `py/js` package (or any other package) and is linked to the `py` external or `pyjs` extension (or both). This can possibly make it usable in standalones (untested).

3. `py` external or `pyjs` jsextension as a container for a python distribution. In this variation, a whole python distribution (zipped or otherwise) is stored inside the external/jsextension object, which can makes it very portable and usable in standalones.


Deployment Scenario  | `py` | `pyjs`
:------------------- | :--: | :--------:
Link to sys python   | x    | x 
Embed in package     | x    | untested
Embed in external    | x    | untested

In addition, to the deployment variations above, onc can choose to use the cython-generated max c-api wrapper or not, which allows regular python code to directly access the max-c-api and script Max objects.



### Key Features


The more mature `py` external has the following c-level methods:

category | method   | param(s)      | in/out | can change ns 
:------- | :--------| :------------ | :----: | :------------: 
core     | import   | module        | in     | yes
core     | eval     | expression    | out    | no
core     | exec     | statement     | in     | yes
core     | execfile | file          | in     | yes
extra    | assign   | var, data     | in     | yes
extra    | call     | var(s), data  | out    | no
extra    | code     | expr or stmt  | out?   | yes
extra    | anything | expr or stmt  | out?   | yes
extra    | pipe     | var, funcs    | out    | no
editor   | read     | file          | n/a    | no
editor   | load     | file          | n/a    | no
interobj | scan     |               | n/a    | no
interobj | send     | name, msg, .. | n/a    | no
meta     | count    |               | n/a    | no


The more recently developed `pyjs` external implements the following c-level methods:

category | method   | param(s)      | in/out | can change ns 
:------- | :--------| :------------ | :----: | :------------: 
core     | code     | expr or stmt  | out?   | yes
core     | eval     | expression    | out    | no
core     | exec     | statement     | in     | yes
core     | execfile | file          | in     | yes
in-code  | eval_to_json| expression | out    | no

The `code` method here (also available in the `py` object in a more experimental category) which can import/exec/eval python code is 'core' in this implementation.


#### Core

The `py` object's *core* features have a one-to-one correspondance to python's very high layer as specified [here](https://docs.python.org/3/c-api/veryhigh.html).

- **Per-object namespaces**. Each `py` object has a unique name (which can be set by the user or provided automatically), and responds to an `import <module>` message which loads a python module in its namespace (essentially a `globals` dictionary), which can be different for each instance.

- **Eval Messages**. Responds to an `eval <expression>` message in the left inlet which is evaluated in the context of the namespace and outputs results to the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

- **Exec Messages**. Responds to an `exec <statement>` message and an `execfile <filepath>` message which executes the statement or the file's code in the object's namespace. This produces no output from the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.


#### Extra

An *extra* feature makes the `py` object play nice in the max/msp ecosystem:

- **Assign Messages**. Responds to an `assign <varname> [x1, x2, ..., xN]` which is equivalent to `<varname> = [x1, x2, ..., xN]` in the python namespace. This is a way of creating variables in the objects python namespace using max message syntax. This produces no output from the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

- **Call Messages**. Responds to a `call <func> arg1 arg2 ... argN` kind of message where `func` is a python callable in the py object's namespace. This corresponds to the python `callable(*args)` syntax. This makes it easier to call python functions in a max-friendly way. If the callable does not variable arguments, it will alternatively try to apply the arguments as a list i.e. `call func(args)`. Future work will try make `call` correspond to a python generic function call: `<callable> [arg1 arg2 ... arg_n] [key1=val1 key2=val2 ... keyN=valN]`. This outputs results to the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

- **Pipe message**. Like a `call` in reverse, responds to a `pipe <arg> <f1> <f2> ... <fN>` message. In this sense, a value is *piped* through a chain of python functions in the objects namespace and returns the output to the left outlet, a bang from the right outlet upon success, or a bang from the middle outlet upon failure.

- **Code or Anything Messages**. Responds to a `code <expression || statement>` or (anything) `<expression || statement>` message. Arbitrary python code (expression or statement) can be used here, because the whole message body is converted to a string, the complexity of the code is only limited by max's parsing and excaping rules. (EXPERIMENTAL and evolving).


#### Interobject Communication

- **Scan Message**. Responds to a `scan` message with arguments. This scans the parent patcher of the object and stores scripting names in the global registry.

- **Send Message**. Responds to a `send <object-name> <msg> <msg-body>` message. Used to send *typed* messages to any named object. Evokes a `scan` for the patcher's objects if a `registry` of names is empty.


#### Code Editor

- **Line REPL**. The `py`has two bpatcher line `repls`, one of which embeds a `py` object and another which has an outlet to connect to one. The repls include a convenient menu with all the `py` object's methods and also feature coll-based history via arrow-up/arrow-down recall of all entries in a session. Of course, a coll can made to save all commands if required.

- **Code Editor**. Double-clicking the `py` object opens a code-editor. This is populated by a `read` message which reads a file into the editor and saves the filepath to an attribute. A `load` message also `reads` the file followed by `execfile`. Saving the text in the editor uses the attribute filepath and execs the saved text to the object's namespace.


#### Scripting

- **Exposing Max API to Python** A significant part of the max api in `c74support/max-includes` has been converted to a cython `.pxd` file called `api_max.pxd`. This makes it available for a cython implementation file, `api.pyx` which is converted to c-code during builds and embedded in the external. This enables a custom python builtin module called `api` which can be imported by python scripts in `py` objects or via `import` messages to the object. This allows the subset of the max-api which has been wrapped in cython code to be called by python scripts or via messages in a patcher.


## Caveats

- The `py` object is currently marked as experimental pre-alpha and still needs further unit/functional/integration testing and field testing of course. Do not use it expecting it is battle-tested!

- As of this writing, the `api` module which is a minimal cython-based wrapper of the max-api is does not unload fully between patches. It requires a restart of Max to work after you close the first patch which uses it. This is a [bug](https://bugs.python.org/issue34309)in python which is being worked on and my be [fixed](https://groups.google.com/forum/?utm_medium=email&utm_source=footer#!msg/cython-users/SnVpCE7Sq8M/hdT8S2iFBgAJ) in future versions.

- `core` features are supposed to be the most stable, and *should* not crash under most circumstances, `extra` features are less stable since they are more experimental, etc..

- The `api` module is the most experimental and evolving part of this project, and is completely optional. If you don't want to use it, don't import it.


## Building

Only tested on OS X at present. Should be relatively easy to port to windows. I'm personally not in a position to do so since I don't have another Max license left for Windows and I'm on Macs and Linux.

The following is required:

### Xcode

Not sure if full xcode is required, perhaps only the command line tools are sufficient

```
$ xcode-select --install
```

otherwise download xcode from the app store.


### py external source and maxsdk

The py external is developed as a max package with a `source` folder which contains the max-sdk as a subfolder, which is conveniently available as a git submodule.

First git clone the `py` repo:

```
$ git clone https://github.com/shakfu/py.git
```

Then cd into the newly cloned source directory and run the following to get the max-sdk

```
$ git submodule init
$ git submodule update 

```

### python3

Python is used to develop `py`, and should be installed on a mac. However, `py` is tested again [Homebrew](https://brew.sh) python on a MacOS Mojave 10.14.6.

The latest python3 can be easily installed can be installed as follows if you already have brew other click on the link above and install it before this step.

```
$ brew install python
```

see: https://installpython3.com/mac for further info if you are interested.


### cython (optional)

[Cython](https://cython.org) is used for wrapping the max api. One could de-couple the cython generated c code from the external and it would work fine since the latter is developed directly using the python c-api, but you would lose the nice feature of calling the max api from python scripts running inside `py` objects.

Install cython as follows:

```
pip install cython
```


### Build it

In the root of the package:

```
make -C source/py build
```
or

```
./build.sh
```
or in the `py/sources/py` directory

```
make build
```

This builds the default 'linked-to-system|homebrew python' version of `py`. Read further for alternative ways to build and install `py`.


### Alternative Builds


#### Embed Python in the Package

In the root,

```
make source/py -C homebrew-pkg
```

or in root/source/py

```
make homebrew-pkg
```

The same pattern applies to the rest of the options. We will assume that you have cd'ed into `root/source/py` so you can omit `-C source/py` which only applies if you call from the root directory.

If you want to copy and zip your system homebrew python and make it a dedicated python interpreter for the `py` object. Note, this does not install any modules. It's up to you to populate what you require.

If you want to build and install cpython from source (will be done automatically):

```
make python-org-pkg
```

#### Embed Python in the External itself

This places a whole minimized python distribution in the external `py.mxo` itself.

To use your system homebrew python to do this:

```
make homebrew-ext
```

If you want to download and build python from source (will be done automatically)

```
make python-org-ext
```


### Sidenote about building on a Mac

If you are developing the package in `$HOME/Documents/Max 8/Packages/py` and you have your icloud drive on for Documents, you will find that `make` or `xcodebuild` will reliably fail with 1 error during development, a codesigning error that is due to icloud sync creating detritus in the dev folder. This can mostly ignored (unless your only focus is codesigning the external).

The solution is to move the external project folder to a non iCloud drive folder (such as $HOME/Downloads for example) and then run "xattr -cr ." in the the project directory to remove the detritus (ironically which Apple's system is itself creating) and then it should succeed (provided you have your Info.plist and bundle id correctly specified). 

I've tried this several times and  and it works (for "sign to run locally" case and for the "Development" case).


### Style it

The coding style for this project can applied automatically during the build process with `clang-format`. On OS X, you can easily install using brew:

```
$ brew install clang-format
```

The style used in this project is specified in the `.clang-format` file.


## BUGS

- [ ] Using the code-editor is not intuitive. Test all scenarios

- [ ] CRITICAL: attempting to reload numpy after the patcher is closed crashes Max (except when you load it through `api` module!)

- [ ] `PyLong_Check` can't pick up `numpy` numbers 

- [ ] `api` object won't reload if a patch is closed (i.e. PyFinalize) and new one opened. Requires a restart of Max. (Python bug which is being worked on).

- [x] pyjs in-function calls to code did not work well as strings (conversion to array fixed it)

- [x] no-return ops in`pyjs` such as `exec` and `import` somehow make javascript assume an error has occured.


## TODO


### Core

- [ ] enhance `call` to allow kwargs [call fn x1 x2 y1=z1 y2=z2]


### Extension

- [ ] add more api wrappers.


### Attributes & Infrastructure

- [ ] add set/get for attributes as appropriate to trigger actions or methods calls
      after changes (NO REASON for using this found so far)

### Testing

- [50] convert `py_coll_tester` into bpatcher that can be fed by `py_repl`

- [ ] complete comprehensive test suite
    - [ ] complete c test suite
    - [ ] complete max test suite

### Future Experiments

- [ ] Try to integrate an ipython shell somehow 


## CHANGELOG


### v0.1


#### Features


##### Core

- [x] Convert `py` into a `jsextension` class

- [x] create new `py_anything` with heuristics to decide whether to delegate to `py_call` or `py_code`.

- [x] add `autoload` attribute to trigger autoload (`load` msg) of code editor code

- [x] for `pythonpath` add file location feature (try pkg/examples/scripts then absolute paths)

- [x] branch `embed-pkg`, embeds a local python install with a zipped stdlib in `support` already successfully tested embedding the python distro in the external itself.

- [x] made it possible to get the py object's name from any module in its namespace!

- [x] enhance `py_exec` method to create a single string from argv so it can import easily

- [x] enhance `py_anything` method to eval if identifier is not a callable yet exists in ns

- [x] Refactor 'py_eval' to make it more consistent with the others

- [x] Implementation of a few high level python api functions in max (eval, exec) to allow the evaluation of python code in a python `globals` namespace associated with the py object.

- [x] Each py object has its own python 'globals' namespace and responds to the following
      msgs
    - [x] `import <module>`: adds module to the namespace
    - [x] `eval <expression>`: evaluate expression within the context of the namespace (cannot modify ns)
    - [x] `exec <statement>`: executes statement into the namespace (can modify ns)
    - [x] `execfile <file.py>`: executes python file into the namespace (can modify ns)
    - [x] `run <file.py>`: executes python file into the namespace (can modify ns)

- [x] Add right inlet bang after eval op ends

- [x] add third (middle) outlet which bangs on an error

##### Extra


- [x] check whether setting a normal attr name, can also set scripting name

- [x] Implement 'send' msg, which sends typed messages to (script) named objects (see: https://cycling74.com/forums/error-handling-with-object_method_typed)

- [x] Add `call (anything)` method to call python callables in a namespace


##### Code Editor (Usability)

- [x] Edit default with text editor

- [x] Add text edit object
    - [x] enable code to be run from editor


##### Line REPL (Usability)

- [x] Add line repl
    - [x] Add up-arrow last line recall (great for 'random.random()')


##### Extensibility

- [x] Create type conversion method in `api.pyx` which could serve python code and also c-code calling python code

- [x] Implement section on two-way globals setting and reading (from python and c) in https://pythonextensionpatterns.readthedocs.io/en/latest/module_globals.html (deferred for now)

- [x] Add bpatcher line repl

- [x] add python scripts to 'examples/scripts'

- [x] Add cythonized access to max c-api..?

- [x] Extensible by embedded cython based python extensions which can call a library of wrapped max_api functions in python code. There is a proof of concept of the python code in the namsepace calling the max api `post` function successfully.

- [x] Exposing of good portion of the max api to cython scripting


##### Architectural

- [x] Global object/dict/ref mgmt (so two externals can exist without Py_Finalize() causing a crash


##### Documentation

- [x] Add .maxref.xml to docs


##### Code Quality

- [x] refactor error handling code (if possible)

- [x] Refactor eval code from py_eval into a function to allow for exec and execfile or PyRun_File scenarios

- [x] Refactor into functions


##### Testing

- [x] pytest testing harness

- [x] make test between test_translate and test_py2 which includes references to a the struct which is missing in the former


#### Bug Fixes

- [x] fixed 'send' which did not have enough error checking and was crashing frequently

- [x] fixed STRANGE bug, single quotes in `py_log` cased a crash in `py_scan_callback`, it's based on post but post alone with the same does not cause a crash. Should simplify logging!

- [x] globex remains after all objects are freed. solution: `PyXDECREF x->p_globals` on `py_free`

- [x] space in `eval` without quotes will cause a crash!

- [x] space in path causes "sprintf" type debugging in execfile to crash max!

- [x] codesigning errors are due to Package being developed in Documents/... which causes issues. If it's a non icloud exposed folder it works ok.

- [x] make exec work! (needs globals in both slots: `PyRun_String(py_argv, Py_single_input, x->p_globals, x->p_globals)`

- [x] `import` statement in eval causes a segmentation fault. see: https://docs.python.org/3/c-api/intro.html exception handling example -> needed to changed Py_DECREF to Py_XDECREF in error handling code

- [x] do not give attr has same name as method (the import saga) as this will crash. fix by making them different.


### leo (optional)

[leo-editor](https://leoeditor.com) is used to factilitate documentation and restructuring.

It is entirely optional, however, if one is interested it could be helpful to understand the project code in outline form:

To install it you can create a python `virtual environment` in the project root (which is ignored by `.gitignore` if named as `leoenv`:

```
$ virtualenv leoenv
$ source leoenv/bin/activate
$ pip install leo
```

After installation, you can open the `project.leo` file as follows:

```
$ leo project.leo &
```


## Prior Art and Thanks

Every now and then when I am developing a patch in Max, I yearn for some simple python function or the other, like the `any` and `all` builtins for example, and I then spend more time than I want researching a Max workaround. 

Thinking that there must be a max external out there, I looked around and found the following:

- Thomas Grill's [py/pyext – Python scripting objects for Pure Data and Max](https://grrrr.org/research/software/py/) which looked promising but then I read that the 'available Max port is not actively maintained.' I also noted that its written in C++ and needs an additional c++ flext layer (http://grrrr.org/ext/flext) to compile. But I was further dissuaded from diving in as it supported only python 2 which seemed difficult to swallow. Ironically, this project has become more active recently, so the above may no longer apply.

- [max-py](https://github.com/njazz/max-py) -- Embedding Python 2 / 3 in MaxMSP with pybind11. This looks like a reasonable effort, but only 9 commits and no further commits for 16 months as of this writing. Again c++ and using pybind11 which I'm not familiar with.

Around the time of the beginning of the covid-19 lockdown, I stumbled upon Iain Duncan's [Scheme for Max](https://github.com/iainctduncan/scheme-for-max) project, and I was quite inspired by his efforts to embed a scheme implementation into a Max external.

So I decided, during the lockdown period, with less distractions than usual, to try to make a minimal python3 external, learn the max sdk, the python c-api, and how to write more than a few lines of c code that won't crash.

It's been an education and I have come to understand precisely a quote I remember somewhere about the c language: that it's "like a scalpel". I painfully now understand this to mean that in skilled hands it can do wonders, otherwise you almost always end up killing the patient.

Thanks to Luigi Castelli for his help on my Max/Msp questions and Stefan Behnel for his help with Cython questions.

