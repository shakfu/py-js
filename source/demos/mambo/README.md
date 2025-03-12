# mambo: a single-header c-based python3 library for max externals

`mambo` uses the `mamba` single-header in an successful experiment to use the new `source/scripts/buildpy.py` script to build a reloctable python version in the the build directory and then use `cmake` to build this external and embed the python interpreter in the external bundle's `Resource` directory along the lines of what `builder` does with the `py` and `pyjs`

## Usage

1. Build Relocatable Python

Python source code (current version) is downloaded from python.org, configured, built, shrunk and prepared for embedding.

```sh
./source/scripts/buildpy.py
```

2. Build the external

```sh
make mambo-shared
```

for a dynamically-linked build, and 


```
make mambo-static
```

for a statically-linked build.

3. Test it

Open the `help/mambo.maxhelp` file to test the external.

Feel free to looking inside it the external by right-click on it and select `Package contents`.

