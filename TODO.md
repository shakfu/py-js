# TODO

## Build System

- [ ] add `-configuration` to Builder.xcodebuild function in `builder.core`

- [ ] update and include testing framework [max-test](https://github.com/Cycling74/max-test)

- [ ] make a function to genrate './configure --...' instructions.

- [ ] external products which are built in full test runs should be stored in some organizaed way

  ```text

    build/
      lib/
      externals/
        x86_64/
          3.9.12/
            shared-pkg/
            shared-ext/
          3.10.3/
        arm64/
          3.10.3/
            static-ext
  ```


- [ ] check why `py-js.xcconfig` for `homebrew-ext` refers to `support`. May implicitly rely on `homebrew-pkg` preceding it.

- [ ] Reorganize patch system and clean it up. Perhaps even convert it such that it is generated from python builder

- [ ] Move download and post_processing of `relocatable-pkg` to python-related class in `builder/core`

- [ ] Add beeware downloader.

- [ ] Create `local-pkg`, `local-ext`, analog to `homebrew-pkg`, `homebrew-ext` but for python.org installations 

- [ ] If a patch is not found, default to a standard working path or no patch at all

- [ ] Add warning for `shared-ext` being opened up one after the other, Max will crash because it caches the former.


## ENHANCEMENTS


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
