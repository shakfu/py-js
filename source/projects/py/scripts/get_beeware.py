"""
get tweaked / statically built components from beeware Python-Apple-support project

https://github.com/beeware/Python-Apple-support

Note that the structure of beeware python releases changed after python 3.7

3.7
├── BZip2
│   └── Headers
├── OpenSSL
│   └── Headers
├── Python
│   ├── Headers
│   └── Resources
└── XZ
    └── Headers

3.8-3.11
├── Python.xcframework
│   └── macos-arm64_x86_64
│        └── Headers
└── python-stdlib
    ├── asyncio
    ├── collections
    ├── concurrent
    ├── ctypes
    ├── curses
    ├── dbm
    ├── distutils
    ├── email
    ├── encodings
    ├── html
    ├── http
    ├── importlib
    ├── json
    ├── lib-dynload
    ├── lib2to3
    ├── logging
    ├── multiprocessing
    ├── pydoc_data
    ├── sqlite3
    ├── unittest
    ├── urllib
    ├── venv
    ├── wsgiref
    ├── xml
    ├── xmlrpc
    └── zoneinfo

"""
import os
from pathlib import Path

URL = ('https://github.com/beeware/Python-Apple-support/releases/'
      'download/{version}-{patch}/Python-{version}-macOS-support.{patch}.tar.gz')


VERSIONS = {
    '3.7' : 'b9',
    '3.8' : 'b12',
    '3.9' : 'b10',
    '3.10': 'b6',
    '3.11': 'b1',

}

def cmd(shellcmd):
    print(shellcmd)
    os.system(shellcmd)

def get_python_libs(destination_dir: str, version: str = '3.8'):
    """get tweaked / statically built components from beeware Python-Apple-support project

    see: https://github.com/beeware/Python-Apple-support
    """
    # checks
    assert version in VERSIONS, f"version {version} not available"
    patch = VERSIONS[version]
    dst_dir = Path(destination_dir)
    url = Path(URL.format(version=version, patch=patch))
    dst = dst_dir / url.name
    print(f"retrieving {url} to {dst}")
    if dst.exists():
        print("skipping download ...")
    else:
        cmd(f"curl -L --fail '{url}' -o '{dst}'")
    if not dst.exists():
        print(f"could not retrieve {url}")
        return
    print("unpacking...")
    tarfile = dst_dir / url.name
    cmd(f"tar -xvf '{tarfile}' --directory '{destination_dir}'")
    cmd(f"rm -f '{tarfile}'")
    print('DONE.')
