# Relocatable Framework Pkg

A variation using Gleg Neagle's [Relocatable Python](https://github.com/gregneagle/relocatable-python)

Uses @rpath and requires that `LD_RUNPATH_SEARCH_PATHS` is set in xcode to the right back ref.

Tested as relocatable and works well for the Framework-pkg case.
