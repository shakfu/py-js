# TODO

## ENHANCEMENTS

### Build System (builder)

- [ ] make all xcodeprojects externally xcconfig-parametrizable to `builder`:

  ```bash
  xcodebuild -project py-js.xcodeproj -target py VERSION=3.7 SUFFIX=m
  ```
  
- [ ] inject commandline parameters in python build system to enable more granular downstream decisions.

- [ ] add step in bundle-creation to prepopulate site-packages with list of packages

## BUGS

- [ ] Workflow using the code-editor is not intuitive. The best implementation of this in the Thomas Grill's py/pyext project: double click on the external opens the preferred editor.

- [ ] `PyLong_Check` can't pick up `numpy` numbers: the type of numpy numbers has to be implmented in the type translator.

- [ ] `api` object won't reload if a patch is closed (i.e. PyFinalize) and new one opened. Requires a restart of Max. (Python bug which is being worked on).

- [ ] WARNING: attempting to reload numpy after the patcher is closed causes an error Max (except when you load it through `api` module!). This used to crash Max, but recent version of Python (3.9.x) just cause an non-crashingerror. In version 3.10, the import of does not raise an error but is still doesn't work (see bug above). At least there's progress!

## FEATURES

### Core

- [ ] enhance `call` to allow kwargs [call fn x1 x2 y1=z1 y2=z2]

### UI

- [ ] py_repl_plus -- message in from the inlet should be passed to the terminal line as if they were entered.

### Extensibility

- [ ] add an additional PYTHONHOME is the containing package `support` folder or `scripts` folder for self-contained externals

- [ ] add more api wrappers. (PAUSED PENDING JUPYTER WORK)

### Attributes & Infrastructure

- [ ] add script to install python packages and then remove the cache and `_vendor` libraries of the installer which are large.

- [ ] add set/get for attributes as appropriate to trigger actions or methods calls
      after changes (NO REASON for using this found so far)

### Testing

- [50] convert `py_coll_tester` into bpatcher that can be fed by `py_repl`

- [ ] complete comprehensive test suite
  - [ ] complete c test suite
  - [ ] complete max test suite

### Future Experiments

- [ ] Try to integrate an ipython shell somehow
