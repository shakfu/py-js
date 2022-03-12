# TODO

- [ ] create a pkg distribution built from python src which retains pip for easily installing python packages

- [ ] add step in bundle-creation to prepopulate site-packages with list of packages

- [ ] create FAQ.md

- [ ] py_repl_plus -- message in from the inlet should be passed to the terminal line as if they were entered.

- [x] investigate static linking of numpy and python (see notes): not viable due to project size constraints.

- [ ] reset should be deep, clean should be shallow

- [x] assert builder.product_exists

- [x] replacing 'cp -rf' with pure python self.copy

- [x] copy libintl.a from /usr/local/opt/gettext/lib to ../targets/build/lib/.. for static linking as building it takes ages.

- [x] fix references (still pointing to compiled locations) ??

## PENDING

- [ ] --enable-optimizations at configure
- [x] --enable-shared
- [ ] freeing memory for PY_STATIC case
- [ ] automate installation of packages
- [ ] testing fat dynamic in a package
- [x] codesigning

## BUGS

- [ ] Workflow using the code-editor is not intuitive. The best implementation of this in the Thomas Grill's py/pyext project: double click on the external opens the preferred editor.

- [ ] `PyLong_Check` can't pick up `numpy` numbers: the type of numpy numbers has to be implmented in the type translator.

- [ ] `api` object won't reload if a patch is closed (i.e. PyFinalize) and new one opened. Requires a restart of Max. (Python bug which is being worked on).

- [ ] WARNING: attempting to reload numpy after the patcher is closed causes an error Max (except when you load it through `api` module!). This used to crash Max, but recent version of Python (3.9.x) just cause an non-crashingerror.

## Features and Fixes

### Core

- [ ] enhance `call` to allow kwargs [call fn x1 x2 y1=z1 y2=z2]

### Extension

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
