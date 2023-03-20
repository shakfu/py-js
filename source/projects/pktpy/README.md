# PocketPy Max External

This is an experiment to use [pocketpy](https://github.com/blueloveTH/pocketpy), a C++17 header-only Python interpreter for game engines, in a Max external.

## Notes

This is a very cool early stage project to create an embedding-friendly self-contained python implementation for game engines.

A lot of language compatibility has been implemented but not a lot of infrastructure support (modules, use module support) yet. Still it's early days, so will be interesting to track the project and improve the external iteratively.

## Status

- basic support for `exec`, `eval` of basic types (int, float, string)
