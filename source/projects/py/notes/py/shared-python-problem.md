# The Python-Shared Problem

Struggling, even after extensive re-referencing using `install_name_tool` to make `shared-python` built from source relocatable. The $PREFIX used at build is basically embedded in a non-opaque way.

Only the Homebrew method which is rather complex manages to extricate itself from such issues. It's not clear whether this is by intention or otherwise.

## What macholib dump reveals

For python compiled via `src-shared-pkg` method:

```bash
$ python3 -m macholib dump python3.9/

python3.9/bin/python3.9
    [MachOHeader endian='<' size='64-bit' arch='x86_64']
    /System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation
    $HOME/Downloads/src/py-js/source/py/../../support/python3.9/lib/libpython3.9.dylib
    /usr/local/opt/gettext/lib/libintl.8.dylib
    /usr/lib/libSystem.B.dylib

python3.9/lib/python3.9/lib-dynload/_lzma.cpython-39-darwin.so
    [MachOHeader endian='<' size='64-bit' arch='x86_64']
    /usr/local/opt/xz/lib/liblzma.5.dylib
    /usr/lib/libSystem.B.dylib

python3.9/lib/python3.9/lib-dynload/_gdbm.cpython-39-darwin.so
    [MachOHeader endian='<' size='64-bit' arch='x86_64']
    /usr/local/opt/gdbm/lib/libgdbm.6.dylib
    /usr/lib/libSystem.B.dylib

python3.9/lib/python3.9/lib-dynload/_dbm.cpython-39-darwin.so
    [MachOHeader endian='<' size='64-bit' arch='x86_64']
    /usr/local/opt/gdbm/lib/libgdbm_compat.4.dylib
    /usr/lib/libSystem.B.dylib
```

## After setup-shared.local

Dropping the other non-system dependencies except for the pesky `libintl.8.dylib`

```bash

$ python3 -m macholib dump python-shared

python-shared/bin/python3.9
    [MachOHeader endian='<' size='64-bit' arch='x86_64' subarch='CPU_SUBTYPE_X86_64_ALL']
    /System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation
    $HOME/Downloads/src/py-js/source/py/targets/build/lib/python-shared/lib/libpython3.9.dylib
    /usr/local/opt/gettext/lib/libintl.8.dylib
    /usr/lib/libSystem.B.dylib

python-shared/lib/libpython3.9.dylib
    [MachOHeader endian='<' size='64-bit' arch='x86_64' subarch='CPU_SUBTYPE_X86_64_ALL']
    /usr/local/opt/gettext/lib/libintl.8.dylib
    /usr/lib/libSystem.B.dylib
    /System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation
```

## Do something like this

```bash

cd {BUILD_LIB}/python-shared/lib/

cp /usr/local/opt/gettext/lib/libintl.8.dylib .

chmod 777 libintl.8.dylib

install_name_tool -id @loader_path/libintl.8.dylib libintl.8.dylib 

install_name_tool -change /usr/local/opt/gettext/lib/libintl.8.dylib  @loader_path/libintl.8.dylib libpython3.9.dylib


cd {BUILD_LIB}/python-shared/bin

install_name_tool -change $HOME/Downloads/src/py-js/source/py/targets/build/lib/python-shared/lib/libpython3.9.dylib @executable_path/../lib/libpython3.9.dylib python3.9

install_name_tool -change /usr/local/opt/gettext/lib/libintl.8.dylib @executable_path/../lib/libintl.8.dylib python3.9
```

## py_test_standalone result

All externals built using python-shared from source always reference the original $PREFIX irrespective!!
