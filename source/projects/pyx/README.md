# pyx: example use of cobra with maxcpp

This is a proof-of-concept of using `cobra`, the single-header c++ python3 library for Max externals with Graham Wakefield's [maxcpp](https://github.com/grrrwaaa/maxcpp) (C++ templates for Max/MSP objects)

## Usage

This can be built individually with:

```sh
make pyx
```

For a relocatable dynamically-linked build

```sh
make pyx-shared
```

For a relocatable statically-linked build

```sh
make pyx-static
```
