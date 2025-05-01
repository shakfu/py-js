# Min-API
![CI](https://github.com/Cycling74/min-api/actions/workflows/test.yml/badge.svg)

This folder contains the support files you will need to compile an external object written in C++ using a high-level declarative application programming interface. It is distributed as a part of the [Min-DevKit Package](https://github.com/Cycling74/min-devkit). Please refer to that package for additional documentation and best practices.

## Overview of Contents

* `include` : header files
* `doc` : documentation
* `script` : resources to be included and used by CMake
* `test` : supporting code and resources for unit testing
* [max-sdk-base](https://github.com/Cycling74/max-sdk-base) : a git submodule that provides the low-level bindings to the Max application
* [Conan.md](Conan.md) : information about how to create and use conan packages with `min-api`

## License

Use of this Min-API distribution is governed by the MIT License as stated in the accompanying `License.md` file.

## Breaking changes

April 9, 2021 - Min-API now requires CMake 3.19.0 or later. Use the pre-cmake-3_19 branch if you depend on an earlier CMake version.

## Max 8.2 Update

The Min-API was updated on Jun 7, 2021 to support Apple silion and unify base headers with the Max SDK. There may be some modifications required to existing projects in order to integrate this update. See the [SDK 8.2 update readme](https://github.com/Cycling74/max-sdk/blob/main/README-8.2-update.md) for further details. For support with this update please use the [Max developer forum](https://cycling74.com/forums/category/Dev/page/1).

