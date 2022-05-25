# CHANGELOG

## v0.1.1

### Variants

- added the `mamba` project, a single header python3 library for max externals which provides the `t_py` python type which is supposed to be nested inside another external.

### Patchers

- added example `pyjs_overview.maxpat` patch

- added `pyjs_repl.maxpat` bpatcher


### Core

- normalized functions signatures between `py.c` and `pyjs.c` so they return Max errors instead of being void functions

- refactored `py_code` and `py_anything` to `py_eval_text`

### Documention

- added doxygen docs to `pyjs.c`

- added doxygen docs for `py.c`

- added doxygen for code documentation generation


## v0.1

### Build System (builder)

#### Enhancements

- Apple Silicon Comptability: externals now build as native `arm64` on Apple Silicon machines. Universal binaries are not currently supported.

- Github actions: improved build actions to provide a `.dmg` 'artifact' on-demand which packages and notarizes `py-js` and its externals and required frameworks.

- removed `shared-pkg` till further notice or until I recreate it as a custom `.framework`, since its not working for Apple's notarization algorithm which only recognizes `.frameworks`.

- created smallest possible python3 external: self-contained python3 external yet (6.2MB) for StaticTinyExt.

  This entailed:
  
  - removing openssl as a dependency
  - shrinking the python lib further
  - setting `-configuration` as `Deployment` 

- if python_version for relocatable is given as major.minor only, then it should still work. Currently requires full major.minor.patch version.

- Move download and post_processing of `relocatable-pkg` to python-related class in `builder/core`

- make a function to generate './configure --...' instructions.

- as a consequence of shifting to `max-sdk-base` was able to build on m1 mac `framework-ext` variations after a few tweaks which are detailed in the dev notes section.

- build system is now based on `max-sdk-base` with all tests passing.

- can build dependencies individually now from make

- added `runlog_all` bash function to `scripts/funcs.sh` to run and log all variations as a complete test cycle. (also added option to do it with homebrew variations via `runlog_all_no_brew`)

- streamlined `common.xcconfig` so that it refers to `.build_pyjs` centrally

- Dropped 'full' version of `static-ext` as it was redundant after adding `_ssl` and `_hashlib` to `static-ext`

- added even more intuitive make frontend to builder with help (via `make help`)

- make all xcodeprojects externally xcconfig-parametrizable to `builder`:

  ```bash
  xcodebuild -project py-js.xcodeproj -target py VERSION=3.7 SUFFIX=m
  ```
  
- inject commandline parameters in python build system to enable more granular downstream decisions.

- add step in bundle-creation to prepopulate site-packages with list of packages

#### Bugs

- fixed static builds to work with `.build_pyjs`

- `_hashlib` and `_ssl` are now built for `static-ext` build.

- fixed `framework-ext` and `framework-pkg` options which were not building under new python build system


### Core Features

- shift to python-based build system. Bash / Makefile is now deprecated.

- create a pkg distribution built from python src which retains pip for easily installing python packages

- create FAQ

- shrunk external + numpy to 30MB

- investigate static linking of numpy and python (see notes): not viable due to project size constraints.

- --enable-optimizations at configure

- replacing 'cp -rf' with pure python self.copy

- remove dependency on libintl via patching `configure` for both static and shared

- copy libintl.a from /usr/local/opt/gettext/lib to ../targets/build/lib/.. for static linking as building it takes ages.

- fix references (still pointing to compiled locations) ??

- fix problem with `--enable-shared` builds where pythonhome is not found, and sys.prefix defaults to what is hardcoded at compilation time.

- codesigning / notarization solved.

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

- pyjs in-function calls to code did not work well as strings (conversion to array fixed it)

- no-return ops in`pyjs` such as `exec` and `import` somehow make javascript assume an error has occured.

- Convert `py` into a `jsextension` class

- create new `py_anything` with heuristics to decide whether to delegate to `py_call` or `py_code`.

- add `autoload` attribute to trigger autoload (`load` msg) of code editor code

- for `pythonpath` add file location feature (try pkg/examples/scripts then absolute paths)

- branch `embed-pkg`, embeds a local python install with a zipped stdlib in `support` already successfully tested embedding the python distro in the external itself.

- made it possible to get the py object's name from any module in its namespace!

- enhance `py_exec` method to create a single string from argv so it can import easily

- enhance `py_anything` method to eval if identifier is not a callable yet exists in ns

- Refactor 'py_eval' to make it more consistent with the others

- Implementation of a few high level python api functions in max (eval, exec) to allow the evaluation of python code in a python `globals` namespace associated with the py object.

- Each py object has its own python 'globals' namespace and responds to the following
      msgs
  - `import <module>`: adds module to the namespace
    - `eval <expression>`: evaluate expression within the context of the namespace (cannot modify ns)
    - `exec <statement>`: executes statement into the namespace (can modify ns)
    - `execfile <file.py>`: executes python file into the namespace (can modify ns)
    - `run <file.py>`: executes python file into the namespace (can modify ns)

- Add right inlet bang after eval op ends

- add third (middle) outlet which bangs on an error

#### Extra

- check whether setting a normal attr name, can also set scripting name

- Implement 'send' msg, which sends typed messages to (script) named objects (see: <https://cycling74.com/forums/error-handling-with-object_method_typed>)

- Add `call (anything)` method to call python callables in a namespace

#### Code Editor (Usability)

- Edit default with text editor

- Add text edit object
  - enable code to be run from editor

#### Line REPL (Usability)

- Add line repl
  - Add up-arrow last line recall (great for 'random.random()')

#### Extensibility

- Create type conversion method in `api.pyx` which could serve python code and also c-code calling python code

- Implement section on two-way globals setting and reading (from python and c) in <https://pythonextensionpatterns.readthedocs.io/en/latest/module_globals.html> (deferred for now)

- Add bpatcher line repl

- add python scripts to 'examples/scripts'

- Add cythonized access to max c-api..?

- Extensible by embedded cython based python extensions which can call a library of wrapped max_api functions in python code. There is a proof of concept of the python code in the namsepace calling the max api `post` function successfully.

- Exposing of good portion of the max api to cython scripting

#### Architectural

- Global object/dict/ref mgmt (so two externals can exist without Py_Finalize() causing a crash

#### Documentation

- Add .maxref.xml to docs

#### Code Quality

- refactor error handling code (if possible)

- Refactor eval code from py_eval into a function to allow for exec and execfile or PyRun_File scenarios

- Refactor into functions

#### Testing

- pytest testing harness

- convert `py_coll_tester` into bpatcher that can be fed by `py_repl`

- make test between test_translate and test_py2 which includes references to a the struct which is missing in the former

### Bug Fixes

- pick up default homebrew installed python details by querying /usr/local/opt/python/libexec/bin/python

- fixed 'send' which did not have enough error checking and was crashing frequently

- fixed STRANGE bug, single quotes in `py_log` cased a crash in `py_scan_callback`, it's based on post but post alone with the same does not cause a crash. Should simplify logging!

- globex remains after all objects are freed. solution: `PyXDECREF x->p_globals` on `py_free`

- space in `eval` without quotes will cause a crash!

- space in path causes "sprintf" type debugging in execfile to crash max!

- codesigning errors are due to Package being developed in Documents/... which causes issues. If it's a non icloud exposed folder it works ok.

- make exec work! (needs globals in both slots: `PyRun_String(py_argv, Py_single_input, x->p_globals, x->p_globals)`

- `import` statement in eval causes a segmentation fault. see: <https://docs.python.org/3/c-api/intro.html> exception handling example -> needed to changed Py_DECREF to Py_XDECREF in error handling code

- do not give attr has same name as method (the import saga) as this will crash. fix by making them different.
