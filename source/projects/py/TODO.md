# TODO

There are three major subsystems in this subproject

- The `py` external in c

- The `api` module in c/cython

- The `builder` module in python

-------------------------------------------------------------------------------

## PY External

### Bugs

- [ ] Fix `p_name` attribute not being set properly.

### Enhancements

- [x] Implement section on two-way globals setting and reading (from python and c) in <https://pythonextensionpatterns.readthedocs.io/en/latest/module_globals.html> (deferred for now)

- [ ] Add set/get for attributes as appropriate to trigger actions or methods calls
      after changes (NO REASON for using this found so far)

## Editor

- [x] Redo editor logic, set current default to run-on-close

- [ ] Fix defaults of `run_on_save`, `close_onsave` options, if they are mutually exclusive, then enum is better otherwise make them binary options

## Testing

- [x] complete c test suite

-------------------------------------------------------------------------------

## Max API Module

### Bugs

- [ ] `api` object won't reload if a patch is closed (i.e. `PyFinalize`) and new one opened. Requires a restart of Max. Refer to the [relevant Python bug](https://discuss.python.org/t/safely-using-the-c-api-when-python-might-shut-down) which is being worked.

- [ ] `PyLong_Check` can't pick up `numpy` numbers: the type of numpy numbers has to be implmented in the type translator. This assume tight integration with numpy headers, which creates a dependency on numpy. Should be an option if it is implemented.

- [ ] Attempting to reload `numpy` after the patcher is closed causes an error in Max (except when you load it through `api` module!). This used to crash Max, but recent versions of Python > (3.9.x) just cause an non-crashing error. In version 3.10+,  import `api` of does not raise an error but is still doesn't work (see bug 'reload bug' above). At least there's progress!

- [ ] The `zlib-not-available` bug (see `py-js/patchers/tests/_breaking/zlib-not-available` folder for a detailed write up), which causes the failure of `zlib` to be loaded in complex max patches which include the `js` and `v8` objects and which only afflicts the `shared-ext`, `framework-ext` and `homebre-ext` build variants. Removing the `js` and `v8` objects is so far the only solution for such build variants.

## Enhancements

- [ ] Make `MaxObject` more general so it can be used as a superclass for objects such as `coll`.

- [ ] Add python dict to api.Dict conversion

- [ ] Add [buffer protocol](https://cython.readthedocs.io/en/latest/src/userguide/buffer.html) support to `api.Matrix` to facilitate reading and writing to matrices along the lines of what was done with the `api.Buffer` wrapper.

- [x] Add `api.Path` extension class which wraps the `ext_path.h` api (also `ext_sysfile.h`)

- [ ] Simplify `api` module's 'api' so to speak. It currently is includes quite a bit of redundancy and some low-level classes which may not be worthwhile exposing to users (such as the `LinkList` class).

### Testing

- [ ] Complete api test suite

-------------------------------------------------------------------------------

## Builder Module

- [ ] Change python variant product names to include python version and architecture (and platform), for example: `shared-ext-3711-x86`

- [ ] Each python or pyjs build variant such as `shared-pkg` or `shared-ext`, should produce a unique output, and there should be a dependency mgmt solution which includes a clear dep graph and hashing and caching to minimize unecessary builds and rebuilds

- [ ] Develop a configuration based api for `builder` which can consume yaml, json or similar simple configuration.

- [x] Add NUMPY_INCLUDE var to all xcode projects

- [ ] `min-setup.local` patch system needs to be organized and automated and linked to modules so that options lead to proper removal of extensions and modules with clear dependencies.

- [ ] Add warning for `shared-ext` being opened up one after the other, Max will crash because it caches the former.

- [x] Investigate static linking of numpy and python (see notes): not viable due to project size constraints.

- [ ] Add step in bundle-creation to prepopulate site-packages with list of packages (this is alread done with `relocatable python` variations)

### Automation

- [x] Create script to shrink numpy from source for inclusion in a build.

- [x] Add script which to create throwaway virtualenv environments: `virtualenv venv` to install python packages and then copy targeted to appropriate place in external or package and then delete remove the `venv` after done.

### Future Experiments

- [ ] Move `builder` from `py-js/source/projects/py/builder`to `py-js/source/scripts/builder` and make it more general such that it can build other python3 externals. This requires `xcodegen` to become more mature.
