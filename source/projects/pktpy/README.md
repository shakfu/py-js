# PocketPy Max External

This is an experiment to use [pocketpy](https://github.com/blueloveTH/pocketpy), a C++17 header-only Python interpreter for game engines, in a Max external.

## Notes

This is a very cool early stage project to create an embedding-friendly self-contained python implementation for game engines.

A lot of language compatibility has been implemented but not a lot of infrastructure support (module coverage, user module support) yet. Still it's early days, so will be interesting to track the project and improve the external iteratively.

The project offers preliminary support for the following modules:

- bisect
- c (custom module for c-level access)
- collections
- heapq
- json
- math
- os
- random
- re
- sys
- time

The key feature of this library / external implementation is that it creates a smallish sized external (~ 1.2 Mb) without dependencies making it completely self-contained and portable and ideal for standalones and packages.


## Current Status

- `exec`, `eval`, `anything` methods to execute, eval and import pocketpy python code with support for basic types (int, float, string)

- examples of wrapped functions and builtins (local, max api, etc.).

- see `pktpy.maxhelp` for a demo


## TODO

- add support for container types (list, tuple, set, slice, range)

- add support for code editor object to edit code

- wrap more of max api

