# CHANGELOG

All notable project-wide changes will be documented in this file. Note that each subproject has its own CHANGELOG.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) and [Commons Changelog](https://common-changelog.org). This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## Types of Changes

- Added: for new features.
- Changed: for changes in existing functionality.
- Deprecated: for soon-to-be removed features.
- Removed: for now removed features.
- Fixed: for any bug fixes.
- Security: in case of vulnerabilities.

---

## [0.3.x]

- Changed minimum macOS version from `10.11` to `10.15` by redirecting to `source/scripts/cmake/max-pretarget.cmake`.

- Re-Added pybind11/min-api-based `pymx` from its own repo as `min-api` is now supported in `py-js`.

- Added `min-api` and `min-lib` to `source` directory.

- Added `mpyx`, a proof-of-concept single-header c++ python3 library variant of `cobra`, called `mpy_interpreter.h`, for use with [min-api](https://github.com/cycling74/min-api)-based projects which want to use python3.

- Added `pyx`, a proof-of-concept of using `cobra`, the single-header c++ python3 library for Max externals with Graham Wakefield's [maxcpp](https://github.com/grrrwaaa/maxcpp) (C++ templates for Max/MSP objects),

- Changed `make setup` to now symlink the project directory to both Max 8 and Max 9 Packages directories if they exist.

- Added a single-script version of [py2max](https://github.com/shakfu/py2max), a python library for offline generation of Max/MSP patcher (.maxpat) files, to facilitate `.maxpat` test generation using `maxref.py`.

- Swapped names of `cobra` and `krait` projects.

- Renamed `zthread` to `ztp` to emphasize the use of zeromq + threads + python

- Building `py` and `pyjs` via `make` doesn't require `cython` to be installed.

- Restructured `CMakeLists.txt` to improve organization and readability:
  - Added list-based grouped build options for different external types (core, experimental, pocketpy, etc.)
  - Improved handling of build variants and targets
  - Added cache cleanup at end of build

- Changed the main `CMakeLists.txt` file to enable single-project build options.

- Merged `mambo` features into `mamba` and `cobra`. `mambo` is removed as a separate subproject.

- Added `mambo` variant of `mamba` which uses a new way of building relocatable python3 externals. It includes a tweaked version of the `mamba` single-header c-based python3 library, and makes use of a slightly modified version of the `source/scripts/buildpy.py` script which was [developed externally](https://github.com/shakfu/buildpy). So far, shared, static, and framework builds are possible via: `make mambo-shared`, `make mambo-static` and `make mambo-framework` respectively.

- Added additional 'category' folders in `py-js/source` to improve classification of externals

- Added `ninja` make target to use the `Ninja` builder in cmake builds

- Added additional cmake build options with corresponding make targets

- Added the `pktpy2` external, based on `v2.0.5` of the [pocketpy](https://pocketpy.dev) python3.x interpreter, which aims to be an alternative to [Lua](https://lua.org) for game scripting. Development is ongoing.

- Added [Python3-Externals-for-Max-MSP.pdf](https://github.com/shakfu/py-js/tree/main/source/docs/_book/Python3-Externals-for-Max-MSP.pdf) pdf book to the repository.

- Changed `README` to make it less complex and pushed more project-specific details to each project's README. Quick start is included for ease of setup.

## [0.3.0]

- Added support for Python 3.13.x

## [0.2.6]

- Changed default configuration in order to speed up builds. Now optimizations options ["enable_optimizations", "with_lto"] are not enabled by default but can now be enabled by setting OPTIMIZE=1 environment variable.

- Added `make strip` option (with script) to recursively strip externals in the externals folder (currently macOS only)

- Switched to building `make projects`, on macOS, using `cmake -GXcode ..` to skip signing requirement on Apple Silicon macs. Use `make dev` for development with better error reporting (and an additional codesigning step).

- All python3 externals + `pktpy` now work on Windows via the following sequence:

    ```bash
    mkdir build
    cd build
    cmake ..
    cmake --build . --config Release
    ```

- Removed all posix headers (`libgen.h`, etc.) usage to pave the way for windows compatibility

- Added a project documentation section in `scripts/docs` to collect documentation, organize notes, and automatically build a project guide book as pdf. This is done via [quarto](https://quarto.org)

- Added library of common utility functions

- Added CHANGELOG and TODO files for each subproject

- Fixed .github workflows which have become deprecated due to api changes at github

- Added the `webserv` external based on the [mongoose](https://github.com/cesanta/mongoose) embedded webserver platform for a proof-of-concept of a webserver in an external.

- Changed minimal macos version to '12.0'. This can be set in `py-js/source/scripts/common.cmake`

- Changed: build all projects of cmake type use `make projects` instead of `make cmake`

- Added the `pktpy` project which demonstrates use of [pocketpy](https://github.com/blueloveTH/pocketpy) as a max external.

- Discovered builtin way for max to find the external using an included but undocumented function. This is by way of the `class_getpath` function. This is demonstrated (partially) in the `py.c` method `t_symbol* py_locate_path_to_external(t_py* x)`

- Added the `cobra` project, which pushes the single-header implementation into cpp territory with a cpp class `PythonInterpreter` implementing and encapsulating all of the functionality. This is final extension of the idea of modular python3 interpreter which can be easily nested into any Max external object.

- Added the `mamba` project: a single header python3 library for max externals which provides the `t_py` max type providings max-friendly python3 interpreter methods.  The idea is that an object from this type be instanciated and nested inside any another external.

- Added FAQ

- Changed to `max-sdk-base` and was able to build on Apple Silicon macs successfully: externals now build as native `arm64` on Apple Silicon machines. Universal binaries are not currently supported.

- Added `runlog_all` bash function to `scripts/funcs.sh` to run and log all variations as a complete test cycle. (also added option to do it with homebrew variations via `runlog_all_no_brew`)
