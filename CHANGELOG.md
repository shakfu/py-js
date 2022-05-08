
# CHANGELOG

## v0.1.1

### Core

- [x] added preliminary doxygen docs for `py.c`

- [x] added doxygen for code documentation generation

- [x] normalized functions signatures between `py.c` and `pyjs.c` so they are return errors instead of being void functions

- [x] refactored `py_code` and `py_anything` to `py_eval_text`


## v0.1

### Build System (builder)

#### Enhancements

- [x] Apple Silicon Comptability: externals now build as native `arm64` on Apple Silicon machines. Universal binaries are not currently supported.

- [x] Github actions: improved build actions to provide a `.dmg` 'artifact' on-demand which packages and notarizes `py-js` and its externals and required frameworks.

- [x] removed `shared-pkg` till further notice or until I recreate it as a custom `.framework`, since its not working for Apple's notarization algorithm which only recognizes `.frameworks`.

- [x] created smallest possible python3 external: self-contained python3 external yet (6.2MB) for StaticTinyExt.

  This entailed:
  
  - removing openssl as a dependency
  - shrinking the python lib further
  - setting `-configuration` as `Deployment` 

- [x] if python_version for relocatable is given as major.minor only, then it should still work. Currently requires full major.minor.patch version.

- [x] Move download and post_processing of `relocatable-pkg` to python-related class in `builder/core`

- [x] make a function to generate './configure --...' instructions.

- [x] as a consequence of shifting to `max-sdk-base` was able to build on m1 mac `framework-ext` variations after a few tweaks which are detailed in the dev notes section.

- [x] build system is now based on `max-sdk-base` with all tests passing.

- [x] can build dependencies individually now from make

- [x] added `runlog_all` bash function to `scripts/funcs.sh` to run and log all variations as a complete test cycle. (also added option to do it with homebrew variations via `runlog_all_no_brew`)

- [x] streamlined `common.xcconfig` so that it refers to `.build_pyjs` centrally

- [x] Dropped 'full' version of `static-ext` as it was redundant after adding `_ssl` and `_hashlib` to `static-ext`

- [x] added even more intuitive make frontend to builder with help (via `make help`)

- [x] make all xcodeprojects externally xcconfig-parametrizable to `builder`:

  ```bash
  xcodebuild -project py-js.xcodeproj -target py VERSION=3.7 SUFFIX=m
  ```
  
- [x] inject commandline parameters in python build system to enable more granular downstream decisions.

- [x] add step in bundle-creation to prepopulate site-packages with list of packages

#### Bugs

- [x] fixed static builds to work with `.build_pyjs`

- [x] `_hashlib` and `_ssl` are now built for `static-ext` build.

- [x] fixed `framework-ext` and `framework-pkg` options which were not building under new python build system


### Core Features

- [x] shift to python-based build system. Bash / Makefile is now deprecated.

- [x] create a pkg distribution built from python src which retains pip for easily installing python packages

- [x] create FAQ

- [x] shrunk external + numpy to 30MB

- [x] investigate static linking of numpy and python (see notes): not viable due to project size constraints.

- [x] --enable-optimizations at configure

- [x] replacing 'cp -rf' with pure python self.copy

- [x] remove dependency on libintl via patching `configure` for both static and shared

- [x] copy libintl.a from /usr/local/opt/gettext/lib to ../targets/build/lib/.. for static linking as building it takes ages.

- [x] fix references (still pointing to compiled locations) ??

- [x] fix problem with `--enable-shared` builds where pythonhome is not found, and sys.prefix defaults to what is hardcoded at compilation time.

- [x] codesigning / notarization solved.

## Pre-release0

- created a release (using python 3.96) of statically built `py.mxo` and `pyjs.mxo`.

- retructuring folder structure to properly separate subprojects and projects.

- Added an example using Beeware's very static [python build method](https://github.com/beeware/Python-Apple-support)

- Added an example of framework-pkg bundle using Greg Neagle's [relocatable-python]( https://github.com/gregneagle/relocatable-python)

- Fixed some shell expansion risks highlighted by jobor019.

- Finally managed to resolve the release blocking issue of letting the external know its own path without hardcoding the name of the bundle. This means that in the specific case of static externals, `PYTHONHOME` can be set to the external's `Resources` folder. Thanks to Timothy Place for the tip on the cycling74 forums.

- Side project: translate [pdpython](https://github.com/garthz/pdpython) to max -> mxpy.c
  - compiling without errors but non-functional right now.

- New demo section: uses [xcodegen](https://github.com/yonaskolb/XcodeGen) to generate xcode projects from `yaml` spec files.

- builder: new python build system

  - While the makefile/bash based build system works quite well for the homebrew cases, it was found to be a little limited in more complex cases. This led me to go off on a protracted tangent to develop a pure python build system which is now included as a parallel build system.
  
  - This does not mean that bash and makefile will be abandoned. This parallel track just means that I can use the python build system when bash scripts and makefiles become unwieldy. Ultimately the python builder should be able to address all compilation cases comprehensively.

  To test it, from path `py-js/source/py` run the following

  ```bash
  $ python3 -m builder --help

  usage: builder [-h] [-v]

  builder: builds the py-js max external and python from source.

  optional arguments:
    -h, --help            show this help message and exit
    -v, --version         show program's version number and exit

  subcommands:
    valid subcommands

    {py_all,py_shared,py_static,pyjs_ext,pyjs_pkg,pyjs_sys,static_ext,test}
                          additional help
      py_all              build all python variations
      py_shared           build shared python
      py_static           build static python
      pyjs_ext            build portable pyjs externals (homebrew)
      pyjs_pkg            build portable pyjs package (homebrew)
      pyjs_sys            build non-portable pyjs package (homebrew)
      static_ext          build portable pyjs externals (static py)
      test                interactive testing shell

  ```

- `bin-homebrew-pkg` is now working without issues and can even be used in standalones

  - The package has to be manually moved into the standalone C74/packages directory and the py.mxo external which was automatically copied during standalone creation has to to be removed since it already exists in the package which copied in manually.

  - The static python build was used in a new target `static-ext` to statically build `py.mxo` and `pyjs.mxo` externals successfully.

### Features

#### Core

- [x] pyjs in-function calls to code did not work well as strings (conversion to array fixed it)

- [x] no-return ops in`pyjs` such as `exec` and `import` somehow make javascript assume an error has occured.

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

#### Extra

- [x] check whether setting a normal attr name, can also set scripting name

- [x] Implement 'send' msg, which sends typed messages to (script) named objects (see: <https://cycling74.com/forums/error-handling-with-object_method_typed>)

- [x] Add `call (anything)` method to call python callables in a namespace

#### Code Editor (Usability)

- [x] Edit default with text editor

- [x] Add text edit object
  - [x] enable code to be run from editor

#### Line REPL (Usability)

- [x] Add line repl
  - [x] Add up-arrow last line recall (great for 'random.random()')

#### Extensibility

- [x] Create type conversion method in `api.pyx` which could serve python code and also c-code calling python code

- [x] Implement section on two-way globals setting and reading (from python and c) in <https://pythonextensionpatterns.readthedocs.io/en/latest/module_globals.html> (deferred for now)

- [x] Add bpatcher line repl

- [x] add python scripts to 'examples/scripts'

- [x] Add cythonized access to max c-api..?

- [x] Extensible by embedded cython based python extensions which can call a library of wrapped max_api functions in python code. There is a proof of concept of the python code in the namsepace calling the max api `post` function successfully.

- [x] Exposing of good portion of the max api to cython scripting

#### Architectural

- [x] Global object/dict/ref mgmt (so two externals can exist without Py_Finalize() causing a crash

#### Documentation

- [x] Add .maxref.xml to docs

#### Code Quality

- [x] refactor error handling code (if possible)

- [x] Refactor eval code from py_eval into a function to allow for exec and execfile or PyRun_File scenarios

- [x] Refactor into functions

#### Testing

- [x] pytest testing harness

- [x] convert `py_coll_tester` into bpatcher that can be fed by `py_repl`

- [x] make test between test_translate and test_py2 which includes references to a the struct which is missing in the former

### Bug Fixes

- [x] pick up default homebrew installed python details by querying /usr/local/opt/python/libexec/bin/python

- [x] fixed 'send' which did not have enough error checking and was crashing frequently

- [x] fixed STRANGE bug, single quotes in `py_log` cased a crash in `py_scan_callback`, it's based on post but post alone with the same does not cause a crash. Should simplify logging!

- [x] globex remains after all objects are freed. solution: `PyXDECREF x->p_globals` on `py_free`

- [x] space in `eval` without quotes will cause a crash!

- [x] space in path causes "sprintf" type debugging in execfile to crash max!

- [x] codesigning errors are due to Package being developed in Documents/... which causes issues. If it's a non icloud exposed folder it works ok.

- [x] make exec work! (needs globals in both slots: `PyRun_String(py_argv, Py_single_input, x->p_globals, x->p_globals)`

- [x] `import` statement in eval causes a segmentation fault. see: <https://docs.python.org/3/c-api/intro.html> exception handling example -> needed to changed Py_DECREF to Py_XDECREF in error handling code

- [x] do not give attr has same name as method (the import saga) as this will crash. fix by making them different.
