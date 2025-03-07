# CHANGELOG for `py` object

## [0.3.x]

- Added `apply` method to `py`, so you can call python functions with the classic python function calling syntax: `func(*args, **kwds)` by sending a max-friendly msg syntax to a `py` instance as follows:
  
  ```python
  
  # now this msg:
  
  apply func 100 200 300 low_range : 0 100 high_range : 100 200
  
  # would be translated to python:
  
  func(100, 200, 300, low_range=[0,100], high_range=[100, 200])  
  ```

- Added `sig` and `product` function to `py_prelude.py` which demonstrate the use of the aforementioned wrapper methods. The first displays the signature of a pure python function object (pure functions were not captured earlier by the `py_handle_output_*` methods), and the second can get obtain the product of a numeric list of atoms.

- Added two wrapper methods in `py.c`, `py_apply_pyfunc_to_pyobj`, for wrapping python functions which take a single python argument, and `py_apply_pyfunc_to_atoms`, for wrapping python functions which take typed atoms.

- Fixed critical `zlib-not-available` bug that appeared for dynamically linked build variants. See `py-js/patchers/tests/_breaking/zlib-no-available/README.md` for full write-up.

- Added `api.Hashtab` methods and tests

- Added `api.support_dir()`, to obtain package's `support` directory.

- Added `api.resources_dir()`, to obtain external bundle's `Resources` directory.

- Added initial `api.Path` class and methods, which requires further iteration.

- Added the remaining `api.AtomArray` methods (pending further testing)

- Fixed a few memory leaks and replaced calls of `PyDECREF` and `PyXDECREF` with `PyCLEAR`.

- Removed redundant `py_table_exists`, `py_list_to_table` and `py_table_to_list` methods from `py.h/py.c`. The `api.Table` object provides all of these functions and more.

- Added `__repr__` methods to all extension classes, and any object not convertible to max atoms not defaults to its `repr` string representation.

- Added `api.MaxObject` attr-related methods: `add_attribute`, `remove_attribute`, `set_attr_value`, `get_attr_value`

- Added optional optimization/stripping configuration which can reduce external sizes by 14%

- Added many unimplemented methods to `api.Table`

- Added `api.Patcher.add_box_from_dict` method.

- Changed `api.Patcher` methods which previously created `api.Box` instances but which returned `bool` results. These methods now correctly return `api.Box` instances.

- Added `register`, `unregister`, `attach`, `detach`, `subscribe`, and `unsubscribe` methods to `api.MaxObject`.

- Added `help`, `open_refpage` and `open_query` methods to `api.MaxObject`.

- Added `api.MaxObject.set_value` and `api.MaxObject.get_value`

- Added initial `api.Matrix` to represent a Jitter Matrix.

- Added `api_jit.pxd` cython header to enabled `api` access to `jit.*` functions/data.

- Added `**kwargs` to `api.Dictionary` constructor, and also `api.Dictionary.__contains__`

- Added `api.Patcher.registered_names`

- Changed `api.Box.from_ptr` to `api.Box.from_object_ptr` and added actual `api.Box.from_ptr`

- Added `api.MaxObject.set_value` and `api.MaxObject.get_value`

- Added `test_api_box.py` and `test_api_box.maxpat`

- Added magic methods `api.Atom.__float__`, `api.Atom.__long__`, and `api.Atom.__str__`

- Added classmethods `api.Dictionary.from_atom` and `api.Dictionary.from_string`

- Added `test_api_pyxternal.py` and `test_api_pyexternal.maxpat`

- Added `api.MaxObject.patcher` and `api.MaxObject.box` properties

- Added `api.PyExternal.out_atoms`

- Added `api.Dictionary.validate` and `api.Dictionary.ensure_atom_safety`

- Added `api.Dictionary.json_from_string`, `api.Dictionary.to_atoms` and `api.Dictionary.from_string`

- Added `api.Dictionary.register` and `api.Dictionary.unregister`

- Added `api.Dictionary.findregistered_clone` and `api.Dictionary.findregistered_retain`

- Added `api.Dictionary.release`

- Added `api.Dictionary.get_atomarray` and `api.Dictionary.append_atomarray`

- Added `api.Dictionary.get_object` and `api.Dictionary.append_object`

- Added `api.Dictionary.namefromptr`

- Added `api.DatabaseView` and `api.DatabaseResult` objects

- Changed `api.Linklist` to use `ptr` instead of `lst`, and other objects similarly.

- Added `api.Dictionary.transaction_lock` and `api.Dictionary.transaction_unlock`

- Added `api.Dictionary.dump`

- Added `api.Dictionary.get_default_long`, `api.Dictionary.get_default_float`,
`api.Dictionary.get_default_sym`, `api.Dictionary.get_default_string`

- Added `api.Dictionary.clone` and `api.Dictionary.clear`

- Added `api.Dictionary.clone_to_self` and `api.Dictionary.clone_to_existing`

- Added `api.Dictionary.merge_to_self` and `api.Dictionary.merge_to_existing`

- Added `api.Dictionary.delete_entry` and `api.Dictionary.chuck_entry`

- Added `api.Atom.setdouble_array` and `api.Atom.getdouble_array`

- Added `api.Atom.setfloat_array` and `api.Atom.getfloat_array`

- Added `api.Atom.setlong_array` and `api.Atom.getlong_array`

- Added `api.Dictionary.getkeys` and `api.Dictionary.getkeys_ordered`

- Added extensive documentation to `api.pyx`

## [0.3.0]

- Added support for Python 3.13.x

- Added `make install-numpy` for automatically building and adding a shrunk version of numpy to the previously built external or package

## [0.2.6]

- Added `make release` for automatic packaging, codesigning, notarizing of pyjs externals.

- Improved `shared-ext`, `static-ext`, `shared-tiny-ext`, `static-tiny-ext` variants for python 3.12

- Fixed a bug which caused Max to crash if more than one `py` instance was created.

- Added `py_textedit` bpatcher to handle python code in the patcher.

- Fixed codesigning bug in `homebrew-ext` and `homebrew-pkg`, both are codesigned by default

- Fixed error caused by removal of `distutils` in 3.12

- Fix for broken `xz-utils` dependency due to its main repo being taken offline due to a well-known security issue. `xz-5.2.5`, which is known to be safe, is now only downloaded from the [cpython dependencies repo](https://github.com/python/cpython-source-deps) instead.

- Fixed broken `static-ext` build with improved setup config

- Added patcher scripting methods to `api.Patcher`

- Added generic `api.MaxObject` object.

- Added tests and tested methods for `api.Binbuf` object.

- Added [buffer protocol support](https://cython.readthedocs.io/en/latest/src/userguide/buffer.html#buffer) to `api.Buffer` with numpy test

- Combined buffer tests (except numpy and array tests) into `test_buffer.py` and `test_buffer.maxpat`

- Added additional message method support for `api.Buffer`

- Added auto-resizing to `api.Buffer.set_samples` and methods to set duration, samplerate as well as methods for generic message sending to a buffer for examples "fill" operations.

- Added `api.Patcher` object to facilitate patcher-scripting from python.

- Added additional capabilities and tests to Buffer extension in the `api` module. This includes retrieval by buffer name, buffer creation,  setting / getting samples from python code using either python's builtin array module or numpy, and a set of demonstrative set of tests:

  pure python tests:

  - `examples/tests/test_buffer.py`
  - `examples/tests/test_buffer_np.py`
  - `examples/tests/test_buffer_array.py`

  patcher tests:

  - `patchers/tests/test_buffer_np.maxpat`
  - `patchers/tests/test_buffer_array.maxpat`
  - `examples/tests/test_buffer_resize.maxpat`

- Added unstable proof-of-concept for building python via cmake. Next step will be to integrate this into the `builder` system. This capability is provided courtesy of the [python-cmake-buildsystem](https://github.com/python-cmake-buildsystem/python-cmake-buildsystem) project, but the python tests fails so this is a work-in-progress

- Added `builder.factory.FactoryManager` to encapsulate dispatch functions.

- Made it easier to build specific python versions (3.7 - 3.11) by providing an optional environment parameter. If provided, the specific version will be downloaded from python.org and used as follows:

  ```bash
  # build both python variant and external

  make shared-ext PYTHON_VERSION=3.7.17

  # or to build python variant only

  make python-shared-ext PYTHON_VERSION=3.7.17

  # and then use that to build the external

  make build-shared-ext PYTHON_VERSION=3.7.17
  ```

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
