# Relocatable Framework Pkg

A variation using Gleg Neagle's [Relocatable Python](https://github.com/gregneagle/relocatable-python)

Uses @rpath and requires that `LD_RUNPATH_SEARCH_PATHS` is set in xcode to the right back ref, currently to:

```text

LD_RUNPATH_SEARCH_PATHS = @loader_path/../../../../support/Python.framework

```

Tested as relocatable and works well for the Framework-pkg case.
