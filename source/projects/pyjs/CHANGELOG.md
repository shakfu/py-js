# CHANGELOG

## [0.1.x]

- Successfully tested `pyjs` using Max 9's `v8` object. Added a test in `py-js/patchers/tests/test_pyjs/test_pyjs_v8.maxpat`.


## [0.1.1]

### Added

- Added example `pyjs_overview.maxpat` patch

- Added `pyjs_repl.maxpat` bpatcher

- Added doxygen docs to `pyjs.c`

### Changed

- Changed functions signatures between `pyjs.c` so they return Max errors instead of being void functions


## [0.1.0]

### Added

- Apple Silicon Compatibility: externals now build as native `arm64` on Apple Silicon machines. Universal binaries are not currently supported.

### Removed

- Removed `shared-pkg` till further notice or until I recreate it as a custom `.framework`, since its not working for Apple's notarization algorithm which only recognizes `.frameworks`.

- Removed dependency on libintl via patching `configure` for both static and shared


## [0.0.1]


### Added

- created a release (using python 3.96) of statically built `pyjs.mxo`.

- builder: new python build system

- Fixed issue with pyjs as in-function calls to `code` method did not work well as strings (conversion to array fixed it)

- Added `pyjs` as a `jsextension` class


### Fixed

- no-return ops in`pyjs` such as `exec` and `import` somehow make javascript assume an error has occured.
