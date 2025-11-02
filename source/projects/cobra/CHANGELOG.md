# CHANGELOG for `cobra` object

## [0.2.2] - 2025-11-02

### Critical Bug Fixes
1. **Fixed mutex deadlock bug**: Changed `std::mutex` to `std::recursive_mutex` to prevent deadlocks when methods call other methods that also acquire the mutex
   - This partially addressed crash when sending `eval str(2)` message
   - With recursive_mutex, the same thread can lock the mutex multiple times without deadlocking

2. **Fixed missing GIL protection in output handlers**: Added `GILGuard` to `handle_output()` method
   - The crash occurred because Python C API functions were being called without holding the GIL
   - `eval_pcode()` would acquire and release the GIL, but then `handle_output()` would call Python API functions (like `Py_XDECREF`, `PyUnicode_AsUTF8`) without re-acquiring it
   - **This was the actual cause of the `eval str(2)` crash**

### Technical Details
1. **Mutex changes:**
   - Changed member variable from `mutable std::mutex m_mutex` to `mutable std::recursive_mutex m_mutex`
   - Updated all `std::lock_guard<std::mutex>` to `std::lock_guard<std::recursive_mutex>`
   - Affected methods: `syspath_append()`, `import_module()`, `eval_pcode()`, `exec_pcode()`, `execfile_path()`, `eval_text()`, `call()`, `assign()`

2. **GIL protection:**
   - Added `GILGuard gil;` at the start of `handle_output()` method (line 1070)
   - Ensures all Python C API calls in output handlers have GIL protection
   - Fixed memory leak by adding `Py_DECREF(pval)` in error paths for `Py_None` and unhandled types

### Root Cause Analysis
The crash sequence was:
1. `eval_pcode()` acquires GIL, executes Python code, returns result, **releases GIL**
2. `eval()` calls `handle_output()` **without GIL**
3. `handle_output()` calls `handle_string_output()` **without GIL**
4. `handle_string_output()` calls `Py_XDECREF()` **without GIL** → CRASH in `_Py_Dealloc`
## [0.2.0] - 2025-11-02

### Critical Fixes
- **Fixed Python interpreter lifecycle management**: Implemented static reference counting to prevent crashes with multiple instances. First instance initializes Python, last instance finalizes.
- **Fixed GIL (Global Interpreter Lock) management**: Created RAII `GILGuard` class for automatic, correct GIL handling. All methods now properly acquire/release GIL.
- **Fixed reference counting bugs**: `p_globals` now properly owned with `Py_INCREF`. Fixed borrowed reference errors that caused memory corruption.
- **Fixed memory leaks**: Audited all error paths and ensured proper cleanup with `Py_XDECREF` calls.

### High Priority Fixes
- **Added thread safety**: Implemented `std::mutex` for all public methods. Safe for concurrent access from Max scheduler and main threads.
- **Fixed resource leaks**: Created RAII `FileGuard` class for automatic file handle cleanup in `execfile_path()`.
- **Fixed compilation errors**: Resolved C++ goto-past-initialization issues by moving variable declarations.

### Improvements
- **Modern C++ adoption**:
  - Replaced `NULL` with `nullptr` throughout
  - Added RAII classes (`GILGuard`, `FileGuard`)
  - Used `std::mutex` and `std::lock_guard`
  - Added `delete` for non-copyable classes
- **Better documentation**:
  - Added comprehensive thread safety documentation
  - Documented memory management semantics
  - Clarified ownership rules for PyObject pointers
- **Enhanced error handling**:
  - Consistent cleanup patterns with goto labels
  - Early parameter validation
  - Better error messages with context

### Methods Refactored
- `PythonInterpreter()` constructor - Reference counting and proper initialization
- `~PythonInterpreter()` destructor - Proper GIL acquisition and cleanup
- `syspath_append()` - Thread safety and fixed goto issues
- `import_module()` - Fixed reference counting
- `eval_pcode()` - Fixed GIL management
- `exec_pcode()` - Fixed GIL and cleanup
- `execfile_path()` - RAII file handling
- `eval_text()` - Fixed GIL release issues
- `call()` - Complete refactor with proper cleanup
- `assign()` - Fixed reference counting

### Notes
- Binary size: 99KB (arm64)
- Build tested and verified on macOS with Python 3.14
- Production-ready pending runtime integration tests
- Backup of original code: py_interpreter.h.backup

## [0.1.x]

- Changed name of the project to `cobra`
- Added new general build system

## [0.1.1]

- Added changelog
- Added py_interpreter.h
