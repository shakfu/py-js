# pyx: example use of cobra with maxcpp

This is a proof-of-concept of using `cobra`, the single-header c++ python3 library for max externals with Graham Wakefield's [maxcpp](https://github.com/grrrwaaa/maxcpp) (C++ templates for Max/MSP objects)

## Usage

This can be built individually with:

```sh
make pyx
```

or as part of the `demos` subgroup:

```sh
make demos
```

## Possible Future Directions

- [ ] Use `nanobind` or `pybind11`

- [ ] Include option to enable `api` module

- [ ] Use `min.api` instead of `maxcpp`
