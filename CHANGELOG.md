# CHANGELOG

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) and [Commons Changelog](https://common-changelog.org). This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## Types of Changes

- Added: for new features.
- Changed: for changes in existing functionality.
- Deprecated: for soon-to-be removed features.
- Removed: for now removed features.
- Fixed: for any bug fixes.
- Security: in case of vulnerabilities.


## [Unreleased]

- Discovered builtin way for max to find the external using an included but undocumented function. This is by way of the `class_getpath` function. This is demonstrated (partially) in the `py.c` method `t_symbol* py_locate_path_to_external(t_py* x)`

- Added the `krait` project, which pushes the single-header implementation into cpp territory with a cpp class `PythonInterpreter` implementing and encapsulating all of the functionality. This is final extension of the idea of modular python3 interpreter which can be easily nested into any Max external object.

- Added the `mamba` project: a single header python3 library for max externals which provides the `t_py` max type providings max-friendly python3 interpreter methods.  The idea is that an object from this type be instanciated and nested inside any another external.

- Added FAQ

- Changed to `max-sdk-base` and was able to build on Apple Silicon macs successfully: externals now build as native `arm64` on Apple Silicon machines. Universal binaries are not currently supported.

- Added `runlog_all` bash function to `scripts/funcs.sh` to run and log all variations as a complete test cycle. (also added option to do it with homebrew variations via `runlog_all_no_brew`)


## [0.1.1]

### Added

- Added initial msp buffer object support

- Added better docs to api.pyx

- Added max table access functions to cython api

- Added doxygen docs for `py.c`


### Changed

- Changed mapping from `py` methods to `px.methods`

- Changed functions signatures between `py.c` so they return Max errors instead of being void functions

- Changed `py_code` and `py_anything` to used refactored `py_eval_text`


## [0.1.0]

### Added

- Apple Silicon Compatibility: externals now build as native `arm64` on Apple Silicon machines. Universal binaries are not currently supported.

- Added FAQ

- Added shrunk external + numpy to 30MB

- Added --enable-optimizations at configure

- Added codesigning / notarization solved.

- Added even more intuitive make frontend to builder with help (via `make help`)


### Changed

- Changed to python-based build system. Bash / Makefile is now deprecated.

- replacing 'cp -rf' with pure python self.copy

- copy libintl.a from /usr/local/opt/gettext/lib to ../targets/build/lib/.. for static linking as building it takes ages.

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

- Added `runlog_all` bash function to `scripts/funcs.sh` to run and log all variations as a complete test cycle. (also added option to do it with homebrew variations via `runlog_all_no_brew`)

- streamlined `common.xcconfig` so that it refers to `.build_pyjs` centrally

- Dropped 'full' version of `static-ext` as it was redundant after adding `_ssl` and `_hashlib` to `static-ext`


### Removed

- Removed `shared-pkg` till further notice or until I recreate it as a custom `.framework`, since its not working for Apple's notarization algorithm which only recognizes `.frameworks`.

- Removed dependency on libintl via patching `configure` for both static and shared

### Fixed

- Fixed static builds to work with `.build_pyjs`

- Fixed: `_hashlib` and `_ssl` are now built for `static-ext` build.

- Fixed `framework-ext` and `framework-pkg` options which were not building under new python build system


## [0.0.1]

### Added

- created a release (using python 3.96) of statically built `py.mxo`.

- retructuring folder structure to properly separate subprojects and projects.

- Added an example using Beeware's very static [python build method](https://github.com/beeware/Python-Apple-support)

- Added an example of framework-pkg bundle using Greg Neagle's [relocatable-python]( https://github.com/gregneagle/relocatable-python)

- Added builder: new python build system

- Added new `py_anything` with heuristics to decide whether to delegate to `py_call` or `py_code`.

- Added `autoload` attribute to trigger autoload (`load` msg) of code editor code

- Added features to get the `py` object's name from any module in its namespace!


- Added a few high level python api functions in max (`eval`, `exec`) to allow the evaluation of python code in a python `globals` namespace associated with the `py` object.

- Added right inlet bang after successful op, middle outlet bang on error


- Added 'send' msg method, which sends typed messages to (script) named objects (see: <https://cycling74.com/forums/error-handling-with-object_method_typed>)

- Added `call (anything)` method to call python callables in a namespace

- Added text edit object

- Added line repl, up-arrow last line recall (great for `random.random()`)

- Added type conversion methods in `api.pyx` which can serve python code and also c-code calling python code.

- Added python scripts to `examples/scripts`

- Added cythonized access to max c-api

- Added `.maxref.xml` to docs

- Added pytest testing harness

### Changed

- Changed `py_exec` method to create a single string from argv so it can import easily

- Changed `py_anything` method to eval if identifier is not a callable yet exists in ns

- Changed `py_eval` to make it more consistent with the others

- Changed `py_coll_tester` into bpatcher that can be fed by `py_repl`

### Fixed

- Fixed some shell expansion risks highlighted by jobor019.

- Fixed the release blocking issue of letting the external know its own path without hardcoding the name of the bundle. This means that in the specific case of static externals, `PYTHONHOME` can be set to the external's `Resources` folder. Thanks to Timothy Place for the tip on the cycling74 forums.

- Fixed global object/dict/ref mgmt (so two externals can exist without `Py_Finalize()` causing a crash)

- Python: now picks up default homebrew installed python details by querying `/usr/local/opt/python/libexec/bin/python`

- Fixed 'send' which did not have enough error checking and was crashing frequently

- Fixed STRANGE bug, single quotes in `py_log` caused a crash in `py_scan_callback`, it's based on post but post alone with the same does not cause a crash. Should simplify logging!

- Fixed `exec`: (needs globals in both slots: `PyRun_String(py_argv, Py_single_input, x->p_globals, x->p_globals)`
