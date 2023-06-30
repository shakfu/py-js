# CHANGELOG for `py` object


## [0.2.x]

- Added `patch/3.8/setup-static-min6.local` to enable tiny variants in python 3.8

- Added `shared-tiny-ext` Makefile target which is the shared lib counterpart to `static-tiny-ext`. The resulting external size for `py.mxo` and `pyjs.mxo` are 8.5 MB and 8.1 MB respectively.

- Added `log-<variant>` Makefile targets for timed and file logged builds

- Slight Reduction of `builder`-based externals' size by removing deprecated modules including `distutils`.

- Changed `configure.patch` for py 11.4 due to changes during patch increments. For versions of python 3.11 below 3.11.4 there is now a special case `configure_pre_11_4.patch`

- Added `fold`, which wraps `functools.reduce`

- Added mechanism to remove `\\` escape sequences to enable commas, double quotes and multi-argument functions to work correctly.

- `pipe` is now much more versatile and can handle multiple arguments and multiple functions.

- Changed `py_call` to a more versatile pure python function which can handle keyword arguments. Now `call` message can be [call fn x1 x2 y1=z1 y2=z2]

- Added `py_mod.py` which is converted to `py_mod.h`, essentially an embedded python module which is loaded into all `py` instances.

- Added another simpler method of retrieving the `t_py *x` object pointer in `api` module via `uintptr_t` round-trip conversion.

- Add bpatcher for editing with external editor with filewatching and reloading on save

- Added `patchers/py_external_editor.maxpat` prototype of using external editor with `filewatcher` and `py_load <path>` message.

- Changed notarized .dmg to be named according to the format `py-js-<variation>-<platform>-<arch>-<py_ver>` for example `py-js-shared-ext-darwin-x86-3.11`

- Changed Github actions: improved build actions to provide a `.dmg` 'artifact' on-demand which packages and notarizes `py-js` and its externals and required frameworks.

- Added cmake support for building from custom python-X variants

- Fix a bug where python-static was not building its dependencies


## [0.2.0]

- Removed most platform specific code related to finding external path and use the builtin `class_getpath`

- Added general support for python 3.11.x for all build variations (see prelease for signed but not notarized demos)

- Changed `import api` to be more robust especially relative to import numpy logic which is now manually enabled.

- Fixed `py` cython build step such that it run only if `api.pyx` is changed.

- Changed: enabling `numpy` support now requires setting manual switches at Makefile and Cmake level.

- Changed supported python for `relocatable-pkg` to python 3.10.8

- Fixed `tiny-static-ext` or `make tiny` option for python 3.10.x

- Discovered builtin way for max to find the external using an included but undocumented function. This is by way of the `class_getpath` function. This is demonstrated (partially) in the `py.c` method `t_symbol* py_locate_path_to_external(t_py* x)`

- Added more api wrappers to cython-based `api` builtin module:
  - Atom
  - Table
  - Buffer
  - Dictionary
  - Database
  - Linklist
  - Binbuf
  - Hashtab
  - AtomArray



## [0.1.1]


- Added initial msp buffer object support

- Added better docs to api.pyx

- Added max table access functions to cython api

- Added doxygen docs for `py.c`

- Changed mapping from `py` methods to `px.methods`

- Changed functions signatures between `py.c` so they return Max errors instead of being void functions

- Changed `py_code` and `py_anything` to used refactored `py_eval_text`


## [0.1.0]

- Added even more intuitive make frontend to builder with help (via `make help`)

- Changed to python-based build system. Bash / Makefile is now deprecated.

- replacing 'cp -rf' with pure python self.copy

- copy libintl.a from /usr/local/opt/gettext/lib to ../targets/build/lib/.. for static linking as building it takes ages.

- make a function to generate './configure --...' instructions.

- streamlined `common.xcconfig` so that it refers to `.build_pyjs` centrally

- Dropped 'full' version of `static-ext` as it was redundant after adding `_ssl` and `_hashlib` to `static-ext`

- Removed `shared-pkg` till further notice or until I recreate it as a custom `.framework`, since its not working for Apple's notarization algorithm which only recognizes `.frameworks`.

- Removed dependency on libintl via patching `configure` for both static and shared

- Fixed static builds to work with `.build_pyjs`

- Fixed: `_hashlib` and `_ssl` are now built for `static-ext` build.

- Fixed `framework-ext` and `framework-pkg` options which were not building under new python build system


## [0.0.1]

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

- Changed `py_exec` method to create a single string from argv so it can import easily

- Changed `py_anything` method to eval if identifier is not a callable yet exists in ns

- Changed `py_eval` to make it more consistent with the others

- Changed `py_coll_tester` into bpatcher that can be fed by `py_repl`

- Fixed some shell expansion risks highlighted by jobor019.

- Fixed the release blocking issue of letting the external know its own path without hardcoding the name of the bundle. This means that in the specific case of static externals, `PYTHONHOME` can be set to the external's `Resources` folder. Thanks to Timothy Place for the tip on the cycling74 forums.

- Fixed global object/dict/ref mgmt (so two externals can exist without `Py_Finalize()` causing a crash)

- Python: now picks up default homebrew installed python details by querying `/usr/local/opt/python/libexec/bin/python`

- Fixed 'send' which did not have enough error checking and was crashing frequently

- Fixed STRANGE bug, single quotes in `py_log` caused a crash in `py_scan_callback`, it's based on post but post alone with the same does not cause a crash. Should simplify logging!

- Fixed `exec`: (needs globals in both slots: `PyRun_String(py_argv, Py_single_input, x->p_globals, x->p_globals`)
