# mambo: a single-header c-based python3 library for max externals

`mambo` is a variant of the `mamba` single-header library used in concert with the light-weight `buildpy.py` script and `cmake` to build a relocatable python3 external along the lines of what the `builder` module does with the `py` and `pyjs`.

## Usage

1. Build Relocatable Python

    Python source code (current version) is downloaded from python.org, configured, built, shrunk and prepared for embedding.

    ```sh
    ./source/scripts/buildpy.py
    ```

2. Build the external, you have a three build options

    a. For a dynamically-linked build

    ```sh
    make mambo-shared
    ```

    b. For a statically-linked build

    ```sh
    make mambo-static
    ```

    c. For a dynamically-linked framework build

    ```sh
    make mambo-framework
    ```

3. Test it

    Open the `help/mambo.maxhelp` file to test the external.

    Feel free to looking inside it the external by right-click on it and select `Package contents`.

## Comparison of Build Variants

| Variant   | Build Command          | Size (MB) | Notes               |
| :-------- | :--------------------- | :-------- | :------------------ |
| Shared    | `make mambo-shared`    | 8.6       |                     |
| Static    | `make mambo-static`    | 11.2      |                     |
| Framework | `make mambo-framework` | 11.9      | includes executable |

## Next Steps

- [ ] Add package build variants.

- [ ] Merge changes with `mamba`.
