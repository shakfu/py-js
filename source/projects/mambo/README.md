# mambo: a single-header c-based python3 library for max externals

`mambo` is a variant of the `mamba` single-header library used in concert with the light-weight `source/scripts/buildpy.py` script and `cmake` to build relocatable python3 externals along the lines of what the `builder` module does with the `py` and `pyjs` externals.

The key benefit in this design is that it provides a lightweight way to embed Python in Max externals without requiring a full Python installation. The single-header approach makes it easy to integrate into Max projects while the build system ensures the Python environment is properly configured and relocatable. This allows Max developers to leverage Python's extensive ecosystem of libraries and tools while maintaining a small footprint and avoiding dependency issues.

## Usage

1. Build the external, you have a five build options

    a. For a fast non-relocatable build which references your existing python installation

    ```sh
    make mambo
    ```

    b. For a relocatable dynamically-linked build

    ```sh
    make mambo-shared
    ```

    c. For a relocatable statically-linked build

    ```sh
    make mambo-static
    ```

    d. For a relocatable dynamically-linked framework build

    ```sh
    make mambo-framework
    ```

    e. For a dynamically-linked framework build for relocatable Max packages

    ```sh
    make mambo-framework-pkg
    ```

2. Test it

    Open the `help/mambo.maxhelp` file to test the external.

    Feel free to looking inside it the external by right-click on it and select `Package contents`.

## Comparison of Build Variants

| Variant   | Build Command          | Size (MB) | Notes               |
| :-------- | :--------------------- | :-------- | :------------------ |
| Shared    | `make mambo-shared`    | 8.6       |                     |
| Static    | `make mambo-static`    | 11.2      |                     |
| Framework | `make mambo-framework` | 11.9      | includes executable |

## Next Steps

- [x] Add package build variants.

- [ ] Merge changes with `mamba`.
