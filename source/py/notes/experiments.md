# Experiments

## Trying to Compile the Max API wrapper as a python builtin

To overcome the code-loading problem, tried to compile minimal version of `api.pyx` + `py_max.pxd` as a python builtin.

The Setup.local file used is in `py-js/source/py/patch/3.9/setup-static-api.local'

To get it to compile with python, I run `python3 -m builder py_static --download` to download `Python-X.Y.Z` as src, copied a minimal version of api.c which did not import the header from py.c to `Python-X.Y.Z/Modules` and then `make`.

To compile with Xcode I had to add '-fprofile-instr-generate' to `Other Linker Flags` and used the `static_ext` recipe for static selc-contained compilation of the external.

**Outcome**: Managed to produce an external which worked as expected and was able to `import api`, but unfortunately embedding `api.c` in python as a statically compiled builtin did not fix the code loading issue.
