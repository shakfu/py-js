# CHANGELOG for `mamba` object

## [0.1.x]

## [0.1.2]

- Merged `mambo` build system. 

- Changed build process to use re-usable cmake function.

- Added `mamba-framework-pkg` build for Max packages.

- Shared, static, and framework builds successful on macOS.

- Added shared, static, and framework builds via: `make mamba-shared`, `make mamba-static` and `make mamba-framework` respectively.

- Added a slightly modified version of the `source/scripts/buildpy.py` script which was [developed externally](https://github.com/shakfu/buildpy).

- `mambo` project, based on `mamba`, created.

## [0.1.1]

- Added `py_exec_file_input` and `py_exec_single_input` to support interactive console.