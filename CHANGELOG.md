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

## [0.2.x]

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

## [0.2.0]

- Added the `webserv` external based on the [mongoose](https://github.com/cesanta/mongoose) embedded webserver platform for a proof-of-concept of a webserver in an external.

- Changed minimal macos version to '12.0'. This can be set in `py-js/source/scripts/common.cmake`

- Changed: build all projects of cmake type use `make projects` instead of `make cmake`

- Added the `pktpy` project which demonstrates use of [pocketpy](https://github.com/blueloveTH/pocketpy) as a max external.

- Discovered builtin way for max to find the external using an included but undocumented function. This is by way of the `class_getpath` function. This is demonstrated (partially) in the `py.c` method `t_symbol* py_locate_path_to_external(t_py* x)`

- Added the `krait` project, which pushes the single-header implementation into cpp territory with a cpp class `PythonInterpreter` implementing and encapsulating all of the functionality. This is final extension of the idea of modular python3 interpreter which can be easily nested into any Max external object.

- Added the `mamba` project: a single header python3 library for max externals which provides the `t_py` max type providings max-friendly python3 interpreter methods.  The idea is that an object from this type be instanciated and nested inside any another external.

- Added FAQ

- Changed to `max-sdk-base` and was able to build on Apple Silicon macs successfully: externals now build as native `arm64` on Apple Silicon machines. Universal binaries are not currently supported.

- Added `runlog_all` bash function to `scripts/funcs.sh` to run and log all variations as a complete test cycle. (also added option to do it with homebrew variations via `runlog_all_no_brew`)
