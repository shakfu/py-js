# PocketPy Max External

This is an experiment to use [pocketpy](https://github.com/blueloveTH/pocketpy), a C++17 header-only Python interpreter for game engines, in a Max external.

## Notes

This is a very cool early stage project to create an embedding-friendly self-contained python implementation for game engines.

A lot of language compatibility has been implemented but not a lot of infrastructure support (module coverage, user module support) yet.

So far `pocketpy` project offers preliminary support for the following modules:

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

Still it's early days, so it will be interesting to track the project and improve the external iteratively.

The key feature of this library / external implementation is that it creates a smallish sized external (~ 0.75 Mb when stripped) without dependencies making it completely self-contained and portable and ideal for standalones and packages.


## Current Status

- `exec`, `eval`, `anything` methods to provide enable the execution, evaluation and importation pocketpy python code with support for basic types (int, float, string)

- no separate method for `import`, this is provided as part of `anything` and it works as expected.

- `List` to atom conversion support

- examples of wrapped functions and builtins (local, max api, etc.).

- see `pktpy.maxhelp` for a demo


## TODO

- add additional support for container types (list, tuple, set, slice, range)

- add support for code editor object to edit code

- wrap more of max api

