# TODO

## Important


## General Improvements



## Refactor


## Bugs

- [ ] `PyLong_Check` can't pick up `numpy` numbers: the type of numpy numbers has to be implmented in the type translator.

- [ ] `api` object won't reload if a patch is closed (i.e. PyFinalize) and new one opened. Requires a restart of Max. (Python bug which is being worked on).

- [ ] WARNING: attempting to reload numpy after the patcher is closed causes an error Max (except when you load it through `api` module!). This used to crash Max, but recent version of Python (3.9.x) just cause an non-crashing error. In version 3.10+,  import `api` of does not raise an error but is still doesn't work (see bug above). At least there's progress!

## Features

### Core


### Usability


### Extensibility

- [ ] add an additional the containing package `support` folder or `scripts` folder to `PYTHONPATH` for self-contained externals

- [ ] add script which to create throwaway virtualenv environments: `virtualenv venv` to install python packages and then copy targeted to appropriate place in external or package and then delete remove the `venv` after done.

### Documentation

- [ ] complete quarto based documentation subproject

### Testing

- [ ] convert `py_coll_tester` into bpatcher that can be fed by `py_repl`

- [ ] list remaining tests to implement

- [ ] complete comprehensive test suite
  - [ ] complete c test suite
  - [ ] complete max test suite

### Attributes

- [ ] add set/get for attributes as appropriate to trigger actions or methods calls
      after changes (NO REASON for using this found so far)

- [ ] differentiate between class and object attributes!! (now everything is a class attribute)

### Editor

- [x] redo editor logic, set current default to run-on-close

- [ ] Fix defaults of `run_on_save`, `close_onsave` options:
  - if they are mutually exclusive, then enum is better otherwise make them binary options

### Max API Wrapper

- [ ] add more tests and examples

### Build System

- [ ] Normalize tiny-variant names: so `shared-tiny-ext` becomes `shared-ext-tiny` and `static-tiny-ext` becomes `static-ext-tiny`.

- [ ] Each python or pyjs build variant such as `shared-pkg` or `shared-ext`, should produce a unique output, and there should be a dependency mgmt solution which includes a clear dep graph and hashing and caching to minimize unecessary builds and rebuilds

- [ ] add NUMPY_INCLUDE var to all xcode projects

- [ ] add `-framework MaxAudioAPI -framework JitterAPI` to OTHER_LDFLAGS in all xcode projects.

- [ ] `min-setup.local` patch system needs to be organized and automated and linked to modules so that options lead to proper removal of extensions and modules with clear dependencies.

- [ ] If a patch is not found, default to a standard working path or no patch at all

- [ ] Add warning for `shared-ext` being opened up one after the other, Max will crash because it caches the former.

### Future Experiments

- [ ] move `builder` from `py-js/source/projects/py/builder`to `py-js/source/scripts/builder` and make it more general such that it can build other python3 externals. This requires `xcodegen` to become more mature.

### Collected

- Implement section on two-way globals setting and reading (from python and c) in <https://pythonextensionpatterns.readthedocs.io/en/latest/module_globals.html> (deferred for now)

- [x] investigate static linking of numpy and python (see notes): not viable due to project size constraints.


- [ ] add step in bundle-creation to prepopulate site-packages with list of packages (this is alread done with `relocatable python` variations)
