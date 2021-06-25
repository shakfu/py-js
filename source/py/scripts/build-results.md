# Build Results

note: `make clean` is run before each build cycle

## Working

- [x] bin-homebrew-sys
- [x] bin-homebrew-pkg
- [x] bin-homebrew-ext

- [x] src-framework-pkg

- [x] src-shared-pkg

## Working with qualifications

- [x] src-shared-ext (dylib ref still local)

## Not working at all

- [f] src-framework-ext -- code object not signed in subcomponent (Python.Framework)
- [f] src-static-pkg -- gettext lib not found in linker
- [f] src-static-ext -- Python.h not found
