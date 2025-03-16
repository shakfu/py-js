# mambo: a single-header c-based python3 library for max externals

`mambo` is a variant of the `mamba` single-header library used in concert with the light-weight `source/scripts/buildpy.py` script and `cmake` to build relocatable python3 externals along the lines of what the `builder` module does with the `py` and `pyjs` externals.

The key benefit in this design is that it provides a lightweight way to embed Python in Max externals without requiring a full Python installation. The single-header approach makes it easy to integrate into Max projects while the build system ensures the Python environment is properly configured and relocatable. This allows Max developers to leverage Python's extensive ecosystem of libraries and tools while maintaining a small footprint and avoiding dependency issues.

## Usage

There are several options to build the external:

- For a fast non-relocatable build which references your existing python installation

```sh
make mambo
```

- For a relocatable dynamically-linked build

```sh
make mambo-shared
```

- For a relocatable statically-linked build

```sh
make mambo-static
```

- For a relocatable dynamically-linked framework build

```sh
make mambo-framework
```

- For a dynamically-linked framework build for relocatable Max packages

```sh
make mambo-framework-pkg
```

Finally, open the `help/mambo.maxhelp` help file to test the external.

## Comparison of Build Variants

| Variant   | Build Command              | Size (MB) | Notes               |
| :-------- | :------------------------- | :-------- | :------------------ |
| Local     | `make mambo`               | 0.19      | non-relocatable     |
| Shared    | `make mambo-shared`        | 8.6       |                     |
| Static    | `make mambo-static`        | 11.2      |                     |
| Framework | `make mambo-framework`     | 11.9      | includes executable |
| Package   | `make mambo-framework-pkg` | 11.9      | includes executable |

Note that in the the package variant above, the external is relocatable as long as it is part of the containing package, as it refers to a `Python.framework` in the `support` folder. The other variants, with the exception of the `local` build, are self-contained externals.

## Next Steps

- [x] Add package build variants.

- [ ] Merge changes with `mamba`.
