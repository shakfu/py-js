# TODO

## Important

- [ ] pass object pointer to cython api via [PyCapsule](https://docs.python.org/3/c-api/capsule.html) method.


## General Improvements



## Refactor


## Bugs

- [ ] `PyLong_Check` can't pick up `numpy` numbers: the type of numpy numbers has to be implmented in the type translator.

- [ ] `api` object won't reload if a patch is closed (i.e. PyFinalize) and new one opened. Requires a restart of Max. (Python bug which is being worked on).

- [ ] WARNING: attempting to reload numpy after the patcher is closed causes an error Max (except when you load it through `api` module!). This used to crash Max, but recent version of Python (3.9.x) just cause an non-crashingerror. In version 3.10+,  import `api` of does not raise an error but is still doesn't work (see bug above). At least there's progress!

## Features

### Core

- [ ] enhance `call` to allow kwargs [call fn x1 x2 y1=z1 y2=z2]

### Usability

- [ ] py_repl_plus -- message in from the inlet should be passed to the terminal line as if they were entered.

### Extensibility

- [ ] add an additional `PYTHONHOME` is the containing package `support` folder or `scripts` folder for self-contained externals

- [ ] add script to install python packages and then remove the cache and `_vendor` libraries of the installer which are large.

### Documentation

- [ ] remove redundant patcher files

- [ ] split `.maxhelp` files into tabs for more information.

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

- [ ] Add bpatcher for editing with external editor with filewatching and reloading on save (prototype done)

- [ ] Workflow using the code-editor is not intuitive. The best implementation of this in the Thomas Grill's py/pyext project: double click on the external opens the preferred editor.

- [ ] redo editor logic, set current default to run-on-close

- [ ] Fix defaults of `run_on_save`, `close_onsave` options:
  - if they are mutually exclusive, then enum is better otherwise make them binary options

### Max API Wrapper

- [ ] add more tests and examples

### Build System

- [ ] add NUMPY_INCLUDE var to all xcode projects

- [ ] add `-framework MaxAudioAPI -framework JitterAPI` to OTHER_LDFLAGS in all xcode projects.

- [x] add shrink standalone step to scripts

- [ ] refactor builder.sign - builder.package to class

- [ ] `min-setup.local` patch system needs to be organized and automated and linked to modules so that options lead to proper removal of extensions and modules with clear dependencies.

- [x] add `-configuration` and `-arch` to Builder.xcodebuild function in `builder.core`

- [ ] update and include testing framework [max-test](https://github.com/Cycling74/max-test)

- [ ] external products which are built in full test runs should be stored in some organized way

- [ ] check why `py-js.xcconfig` for `homebrew-ext` refers to `support`. May implicitly rely on `homebrew-pkg` preceding it.

- [ ] Reorganize patch system and clean it up. Perhaps even convert it such that it is generated from python builder

- [ ] Add beeware downloader.

- [ ] Create `local-pkg`, `local-ext`, analog to `homebrew-pkg`, `homebrew-ext` but for python.org installations.

- [ ] If a patch is not found, default to a standard working path or no patch at all

- [ ] Add warning for `shared-ext` being opened up one after the other, Max will crash because it caches the former.

### Future Experiments

- [ ] move `builder` from `py-js/source/projects/py/builder`to `py-js/source/scripts/builder` and make it more general such that it can build other python3 externals. This requires `xcodegen` to become more mature.

- [ ] Try to integrate an ipython shell somehow (via node.js seems to be most promising)

### Collected

- Implement section on two-way globals setting and reading (from python and c) in <https://pythonextensionpatterns.readthedocs.io/en/latest/module_globals.html> (deferred for now)

- for `pythonpath` add file location feature (try pkg/examples/scripts then absolute paths)

- [ ] enable code to be run from editor (now it optionally runs when an editor is closed)

- [ ] create a pkg distribution built from python src which retains pip for easily installing python packages

- [x] investigate static linking of numpy and python (see notes): not viable due to project size constraints.

- [ ] inject commandline parameters in python build system to enable more granular downstream decisions.

- [ ] add step in bundle-creation to prepopulate site-packages with list of packages (this is alread done with `relocatable python` variations)
