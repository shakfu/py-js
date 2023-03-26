# CHANGELOG

All notable project-wide changes will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) and [Commons Changelog](https://common-changelog.org). This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## Types of Changes

- Added: for new features.
- Changed: for changes in existing functionality.
- Deprecated: for soon-to-be removed features.
- Removed: for now removed features.
- Fixed: for any bug fixes.
- Security: in case of vulnerabilities.

---

## [0.1.x]

- Changed minimal macos version to '12.0'. This can be set in `py-js/source/scripts/common.cmake`

- Changed: build all projects of cmake type use `make projects` instead of `make cmake`

- Added the `pktpy` project which demonstrates use of [pocketpy](https://github.com/blueloveTH/pocketpy) as a max external.

- Discovered builtin way for max to find the external using an included but undocumented function. This is by way of the `class_getpath` function. This is demonstrated (partially) in the `py.c` method `t_symbol* py_locate_path_to_external(t_py* x)`

- Added the `krait` project, which pushes the single-header implementation into cpp territory with a cpp class `PythonInterpreter` implementing and encapsulating all of the functionality. This is final extension of the idea of modular python3 interpreter which can be easily nested into any Max external object.

- Added the `mamba` project: a single header python3 library for max externals which provides the `t_py` max type providings max-friendly python3 interpreter methods.  The idea is that an object from this type be instanciated and nested inside any another external.

- Added FAQ

- Changed to `max-sdk-base` and was able to build on Apple Silicon macs successfully: externals now build as native `arm64` on Apple Silicon machines. Universal binaries are not currently supported.

- Added `runlog_all` bash function to `scripts/funcs.sh` to run and log all variations as a complete test cycle. (also added option to do it with homebrew variations via `runlog_all_no_brew`)
