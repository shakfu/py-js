# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

py-js is a collection of Python3 externals for MaxMSP. The project embeds Python interpreters in Max externals, enabling bidirectional integration between Max and Python. The two main externals are `py` and `pyjs`, with 15+ experimental variants exploring different embedding approaches.

## Build System Architecture

The project uses a multi-layered build system:

1. **Makefile**: High-level orchestration (recommended entry point)
2. **CMake**: Cross-platform build configuration
3. **Custom Python Builder**: Located in `source/projects/py/builder/`, handles complex Python embedding scenarios

### Build Variants

The externals can be built in multiple configurations:

- **local-sys**: Links to system Python (fastest, not relocatable)
- **shared-pkg**: Includes shared Python library in `support/` (relocatable)
- **static-pkg**: Statically linked Python (relocatable, larger binary)
- **framework-pkg**: Uses Python.framework (macOS only, relocatable)
- **windows-pkg**: Windows relocatable build with embedded Python

## Common Build Commands

### Development
```bash
# Build core externals (py + pyjs) linked to system Python
make

# Build using CMake (same as above but via CMake)
make core

# Build all projects except networking
make projects

# Build with Make backend (faster iteration, includes signing)
make dev

# Build with Ninja backend
make ninja
```

### Testing
```bash
# Run all tests
make test

# Tests are located in examples/tests/ and use pytest
pytest examples/tests/
```

### Python Variants
```bash
# Build specific Python linking variant
make shared-pkg      # Shared library + support folder
make static-pkg      # Static linking
make framework-pkg   # Framework bundle (macOS)

# Windows builds
make core-windows-pkg
```

### Individual Externals
```bash
# Build specific external
make py              # Core py external
make pyjs            # JavaScript-friendly variant
make cobra           # C++ single-header variant
make pktpy           # PocketPy-based variant
make net             # Networking externals (requires zmq)
```

### Cleaning
```bash
make clean           # Clean externals, support, and build artifacts
make clean-externals # Clean only built externals
make clean-build-dir # Clean CMake build directory
```

## Architecture

### Directory Structure

- `source/projects/*/` - Individual external implementations
  - `py/` - Main Python3 external (most active development)
  - `pyjs/` - JavaScript Max API variant
  - `cobra/`, `mamba/` - Single-header C/C++ embedding libraries
  - `pktpy/` - PocketPy interpreter external
  - `zpy/`, `ztp/`, `jmx/` - Networking variants (ZeroMQ-based)

- `source/projects/py/builder/` - Custom Python build system
  - Handles downloading, patching, and building Python from source
  - Creates relocatable Python distributions
  - Manages dependency building (OpenSSL, bzip2, xz)

- `examples/tests/` - Pytest-based test suite
- `patchers/` - Max patches for testing and demonstration
- `externals/` - Build output directory for `.mxo` (macOS) / `.mxe64` (Windows)
- `support/` - Runtime dependencies for relocatable builds

### Key Components

**py external** (most mature):
- Written in C using Max SDK and Python C API
- Includes `api.pyx`: Cython wrapper around Max C API (generates ~147K LOC of C)
- Includes `py_prelude.py`: Preloaded utilities compiled into external
- Core methods: import, eval, exec, execfile, call, pipe, assign

**Build System**:
- Python builder CLI: `python -m builder` (in source/projects/py/)
- Handles cross-platform Python compilation with custom patches
- Manages different linking strategies (static/shared/framework)

### Cython Integration

The `api.pyx` file wraps the Max C API. Regenerate `api.c` when modified:

```bash
make api
# Or manually:
cython -3 --timestamps source/projects/py/api.pyx
```

## Platform-Specific Notes

### macOS
- Builds native binaries only (no universal/fat binaries)
- Requires Xcode command line tools
- System Python or Homebrew Python works
- Uses codesigning (handled by `make sign`)

### Windows
- Requires Visual Studio and Python from python.org
- Use `make core-windows-pkg` for relocatable builds
- CMake can be from Visual Studio or standalone installation

## CMake Options

```bash
cmake .. \
  -DBUILD_PYTHON3_CORE_EXTERNALS=ON          # py + pyjs
  -DBUILD_PYTHON3_EXPERIMENTAL_EXTERNALS=ON  # cobra, mamba, etc.
  -DBUILD_POCKETPY_EXTERNALS=ON              # pktpy variants
  -DBUILD_NETWORKING_EXTERNALS=ON            # Requires libzmq
  -DBUILD_VARIANT=<variant>                  # Specify build variant
  -DBUILD_TARGETS=<target>                   # Specific external(s)
```

## Development Workflow

1. **Setup**: `make setup` (initializes submodules, symlinks to Max Packages)
2. **Modify source** in `source/projects/<external>/`
3. **Build**: `make <external>` or `cd build && cmake --build .`
4. **Test**: Open patches in `patchers/` or run `pytest examples/tests/`
5. **Iterate**: CMake detects changes, rebuilds only modified projects

## Python Version Management

Set via environment variable or Makefile:
```bash
make shared-ext PYTHON_VERSION=3.13.5
```

Or uncomment in Makefile:
```makefile
PYTHON_VERSION = 3.13.5
```

## Dependency Management

For macOS with Homebrew:
```bash
# Check dependencies
make check-deps

# Build Python dependencies from source
make deps      # OpenSSL, bzip2, xz
```

## Code Style

- C code: Uses `.clang-format` configuration
- Python code: Standard Python conventions
- Run formatter: `make clang-format`

## Max Package Structure

After `make setup`, the repo is symlinked to `~/Documents/Max 8/Packages/py-js/`:
- `externals/` - Compiled externals
- `help/` - Max help files
- `examples/` - Example patches and Python scripts
- `patchers/` - Test patches
