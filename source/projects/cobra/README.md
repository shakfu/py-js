# cobra: a single-header c++ python3 library for max externals

Provides a single-header cpp-centric `py_interpreter.h` library with a python3 interpreter class.

Note that `cobra.cpp` is just a demonstration of an external using `py_interpreter.h`.

I called it `cobra`, in honour of the 'Krait Lightspeeder' in the original [Elite](https://en.wikipedia.org/wiki/Elite_(video_game)).

## Building

From the root of the `py-js` project, there are several options to build the external:

- For a fast non-relocatable build which references your existing python installation

```sh
make cobra
```

- For a relocatable dynamically-linked build

```sh
make cobra-shared
```

- For a relocatable statically-linked build

```sh
make cobra-static
```

- For a relocatable dynamically-linked framework build

```sh
make cobra-framework
```

- For a dynamically-linked framework build for relocatable Max packages

```sh
make cobra-framework-pkg
```

Finally, open the `help/cobra.maxhelp` help file to test the external.
This will build all subprojects, including `cobra`, using the standard cmake build process.

## Tests

See `cobra.maxhelp` in `py-js/help`
