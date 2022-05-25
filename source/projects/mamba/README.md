# The Mamba Experiment

This project is an experimental attempt to modularize the python object and make it re-usable so that it can be easily nested inside another external.

The objective to make this simple and easy to do by including a single header file so that the max external can provide general or specialized python 'services'.

The project is implemented in the header file `py.h`.

The name is of this header is likely to change to differentiate it from the `py` object and its header.

Other names could be `mpy.h` or `mamba.h`


## Building

From the root of the `py-js` project

```bash
make cmake
```


## Tests

See `test_mamba.maxpat` in `py-js/patchers`