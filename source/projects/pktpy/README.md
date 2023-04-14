# PocketPy Max External

This is an experiment to use [pocketpy](https://github.com/blueloveTH/pocketpy), a nifty C++17 header-only Python interpreter for game engines, in a Max external.

## Notes

[pocketpy](https://github.com/blueloveTH/pocketpy) is a very cool early stage project to create an embeddable self-contained python implementation for game engines.

A lot of language compatibility has been implemented but not a lot of module coverage yet, although user modules have been recently implemented.

So far it offers preliminary support for the following modules:

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

This max external project embeds the pocketpy interpreter, provides for easy wrapping of max-api functions in c++, and produces a small sized external (~ 0.68 Mb when stripped) without dependencies making it completely self-contained, portable and ideal for standalones and packages.

## Structure of Implementation

```text
pktpy.cpp -includes-> pktpy.h -includes-> pocketpy.h
```

- `pocketpy.h`: the [pocketpy](https://github.com/blueloveTH/pocketpy) header.

- `pktp.h`: a general middle layer providing a cpp class, `PktpythonInterpreter`, with helpers and round-trip translation methods between pocketpy and the max c-api. The user should ideally not need to change anything in this file.

- `pktpy.cpp`: In this file, the max-api methods are implemented by using the functionality in the middle layer. This is were customization should ocurr (e.g custom bultin methods).

## Current Status

- `exec`, `eval`, `anything`, `execfile`, methods to enable the execution, evaluation and importation of pocketpy python code with support for basic types (int, float, strings) and lists.

- no separate method for `import`, this is provided as part of `anything` and it works as expected.

- `List` to atom conversion support

- examples of wrapped functions and builtins (local, max api, etc.).

- see `pktpy.maxhelp` for a demo

