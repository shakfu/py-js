# mamba: a single-header c-based python3 library for max externals

This project is a single-header python3 library for Max externals. It is the result of an attempt to modularize the python interpreter for Max and make it re-usable so that it can be easily nested inside another external.

The idea is that by including a single header file any max external can provide general or specialized python 'services'.

The project is implemented in the header file `py.h`.

The name of this header is likely to change to differentiate it from the `py` object and its header.

Other names could be `mpy.h` or `mamba.h`

## Building

From the root of the `py-js` project

```bash
make projects
```

This will build all subprojects, including `mamba`, using the standard cmake build process.

## Tests

See `mamba.maxhelp` in `py-js/help`
