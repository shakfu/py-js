# Packaging notes

## Static External Size Variations

method                | external | size
:-------------------- | :------: | -------:
base                  | py.mxo   | 9.3 MB
base                  | pyjs.mxo | 9.1 MB
full-nopatch          | py.mxo   | 14.4 MB
full-nopatch          | pyjs.mxo | 14.2 MB
full-patch (10.14)    | py.mxo   | 14.3 MB
full-patch (10.14)    | pyjs.mxo | 14.2 MB
full-patch (10.13)    | py.mxo   | 13.7 MB
full-patch (10.13)    | pyjs.mxo | 14.5 MB

## Setup.local

see examples in `py-js/source/py/patch`

## packaging results (3.8.4)

method            | size  | works  | codesign  | standalone
:---------------- | :---- | :----: | :-------: | :--------:
bin-framework-ext |       |  0c    |           |
bin-framework-pkg | 12+   |  1c    |           |
src-framework-ext |       |  0c    |           |
src-framework-pkg | 7.5   |  1c    | 1         |  0
bin-homebrew-ext  | 8.5   |  1     | 0c        |  1 (py)
bin-homebrew-pkg  | 9.0   |  1     | 1c        |  0
bin-homebrew-sys  | 0.0   |  1     | 1c        |  1 (py)
src-static-ext    | 7.4   |  1     | 1         |  1 (py)
src-static-pkg    |       |        |           |
src-shared-ext    |       |        |           |
src-shared-pkg    |       |        |           |

framework-ext has a build error (fix then retry) static-ext

## design

The Homebrew copy of gettext leaks into the macOS build
if it is present. Uninstall gettext to make sure that doesn't happen.

```bash
sudo brew uninstall --ignore-dependencies gettext
```

simplified as much as possible

```bash
$tree -L 6 -n py.mxo
py.mxo
└── Contents
    ├── Frameworks
    │   ├── Python.framework
    │   │   ├── Python -> Versions/Current/Python
    │   │   ├── Resources -> Versions/Current/Resources
    │   │   └── Versions
    │   │       ├── 3.8
    │   │       │   ├── Python
    │   │       │   ├── Resources
    │   │       │   ├── include
    │   │       │   └── lib
    │   │       └── Current -> 3.8
    │   └── <non-python dylibs>
    ├── Info.plist
    ├── MacOS
    │   ├── <external>
    │   └── python
    ├── PkgInfo
    └── Resources
        └── lib
            ├── python3.8
            │   ├── lib-dynload
            │   │   └── <python .so extensions>
            │   └── site.py
            └── python38.zip

```

based on py2app

```bash
$tree -L 6 -n py.mxo
py.mxo
└── Contents
    ├── Frameworks
    │   ├── Python.framework
    │   │   ├── Python -> Versions/Current/Python
    │   │   ├── Resources -> Versions/Current/Resources
    │   │   └── Versions
    │   │       ├── 3.8
    │   │       │   ├── Python
    │   │       │   ├── Resources
    │   │       │   ├── include
    │   │       │   └── lib
    │   │       └── Current -> 3.8
    │   └── <non-python dylibs>
    ├── Info.plist
    ├── MacOS
    │   ├── <external>
    │   └── python
    ├── PkgInfo
    └── Resources
        ├── include
        │   └── python3.8
        │       └── <includes>
        ├── lib
        │   ├── python3.8
        │   │   ├── config-3.8-darwin
        │   │   ├── lib-dynload
        │   │   │   └── <python .so extensions>
        │   │   └── site.pyc -> ../../site.pyc
        │   └── python38.zip
        ├── site.pyc
        └── zlib.cpython-38-darwin.so
```

## py2app

I have a use-case for an open-source [project](https://github.com/shakfu/py-js) for a plugin which provides for arbitrary python code in [max](https://cycling74.com/products/max) and requires that I embed the python interpreter (via the c-api and cython) into a c-based plugin or 'external' in max/msp parlance.

The operational code works well and is tested but I am continuously struggling to properly structure a Python.Framework-based build (against which the external is linked via @loader_path) which is embedded in the external in a manner which will pass the opaque code-signing and notarization steps.

To give a concrete examples, the following 'bundle' actually works in the application and is based on compiling python.org's 3.7.7 as a framework:

```bash
$ tree -L 7 -n py.mxo
py.mxo
└── Contents
    ├── Frameworks
    │   └── Python.framework
    │       ├── Python -> Versions/Current/Python
    │       ├── Resources -> Versions/Current/Resources
    │       └── Versions
    │           ├── 3.7
    │           │   ├── Python
    │           │   ├── Resources
    │           │   │   ├── English.lproj
    │           │   │   ├── Info.plist
    │           │   │   └── Python.app
    │           │   ├── bin
    │           │   │   ├── get_pip.sh
    │           │   │   ├── python3.7
    │           │   │   └── python3.7m-config
    │           │   ├── include
    │           │   │   └── python3.7m
    │           │   └── lib
    │           │       ├── libpython3.7.dylib -> ../Python
    │           │       ├── libpython3.7m.dylib -> ../Python
    │           │       ├── python3.7
    │           │       └── python37.zip
    │           └── Current -> 3.7
    ├── Info.plist
    ├── MacOS
    │   └── py
    ├── PkgInfo
    └── _CodeSignature
        └── CodeResources
```

Of course, this fails the code signing step for reasons that will be well known to you.

In any case, I stumbled upon py2app and saw for the first time you had managed to place 'lib' in Resources which is a condition of a well-formed bundle I believe.

So my question is: would it be possible to use py2app and/or macholib to address the embedding scenario above where I would need a full python library (or a specified subset thereof) but would not need an initial 'script' per se or to turn it into a .app as per the typical py2app use-case. In this sense, py2app would be more of a 'py2bundle' (-:

```bash
$tree -L 6 -n hello.app
hello.app
└── Contents
    ├── Frameworks
    │   ├── Python.framework
    │   │   ├── Python -> Versions/Current/Python
    │   │   ├── Resources -> Versions/Current/Resources
    │   │   └── Versions
    │   │       ├── 3.8
    │   │       │   ├── Python
    │   │       │   ├── Resources
    │   │       │   ├── include
    │   │       │   └── lib
    │   │       └── Current -> 3.8
    │   ├── libcrypto.1.1.dylib
    │   ├── libgdbm.6.dylib
    │   ├── liblzma.5.dylib
    │   └── libssl.1.1.dylib
    ├── Info.plist
    ├── MacOS
    │   ├── hello
    │   └── python
    ├── PkgInfo
    └── Resources
        ├── PythonApplet.icns
        ├── __boot__.py
        ├── __error__.sh
        ├── __pycache__
        │   └── site.cpython-38.pyc
        ├── hello.py
        ├── include
        │   └── python3.8
        │       └── pyconfig.h
        ├── lib
        │   ├── python3.8
        │   │   ├── config-3.8-darwin
        │   │   ├── lib-dynload
        │   │   │   ├── _asyncio.so
        │   │   │   ├── _bisect.so
        │   │   │   ├── _blake2.so
        │   │   │   ├── _bz2.so
        │   │   │   ├── _codecs_cn.so
        │   │   │   ├── _codecs_hk.so
        │   │   │   ├── _codecs_iso2022.so
        │   │   │   ├── _codecs_jp.so
        │   │   │   ├── _codecs_kr.so
        │   │   │   ├── _codecs_tw.so
        │   │   │   ├── _contextvars.so
        │   │   │   ├── _csv.so
        │   │   │   ├── _ctypes.so
        │   │   │   ├── _datetime.so
        │   │   │   ├── _dbm.so
        │   │   │   ├── _decimal.so
        │   │   │   ├── _elementtree.so
        │   │   │   ├── _gdbm.so
        │   │   │   ├── _hashlib.so
        │   │   │   ├── _heapq.so
        │   │   │   ├── _lzma.so
        │   │   │   ├── _md5.so
        │   │   │   ├── _multibytecodec.so
        │   │   │   ├── _multiprocessing.so
        │   │   │   ├── _opcode.so
        │   │   │   ├── _pickle.so
        │   │   │   ├── _posixshmem.so
        │   │   │   ├── _posixsubprocess.so
        │   │   │   ├── _queue.so
        │   │   │   ├── _random.so
        │   │   │   ├── _scproxy.so
        │   │   │   ├── _sha1.so
        │   │   │   ├── _sha256.so
        │   │   │   ├── _sha3.so
        │   │   │   ├── _sha512.so
        │   │   │   ├── _socket.so
        │   │   │   ├── _ssl.so
        │   │   │   ├── _struct.so
        │   │   │   ├── _testcapi.so
        │   │   │   ├── _tkinter.so
        │   │   │   ├── _uuid.so
        │   │   │   ├── array.so
        │   │   │   ├── binascii.so
        │   │   │   ├── fcntl.so
        │   │   │   ├── grp.so
        │   │   │   ├── math.so
        │   │   │   ├── mmap.so
        │   │   │   ├── pyexpat.so
        │   │   │   ├── resource.so
        │   │   │   ├── select.so
        │   │   │   ├── termios.so
        │   │   │   ├── unicodedata.so
        │   │   │   └── zlib.so
        │   │   └── site.pyc -> ../../site.pyc
        │   └── python38.zip
        ├── site.pyc
        └── zlib.cpython-38-darwin.so
```

## codesigning and notarization

see:

- <http://www.zarkonnen.com/signing_notarizing_catalina>
- <https://stackoverflow.com/questions/56890749/macos-notarize-in-script>
- <https://github.com/rednoah/notarize-app/blob/master/notarize-app>
- <https://glyphsapp.com/tutorials/how-to-notarize-your-plug-ins>
- <https://developer.apple.com/forums/thread/129228>
- <https://developer.apple.com/forums/thread/128166>
- <https://developer.apple.com/forums/thread/129546>
- <https://developer.apple.com/forums/thread/129045>
- <https://developer.apple.com/documentation/macos-release-notes/macos-catalina-10_15-release-notes?preferredLanguage=occ>
- <https://github.com/pyinstaller/pyinstaller/wiki/Recipe-OSX-Code-Signing-Qt>

## packaging results (3.7.7)

method            | size  | works  | codesign  | standalone
:---------------- | :---- | :----: | :-------: | :--------:
bin-framework-ext |       |  0c    |           |
bin-framework-pkg | 12+   |  1c    |           |
src-framework-ext |       |  0c    |           |
src-framework-pkg | 7.5   |  1c    | 1         |  0
bin-homebrew-ext  | 8.5   |  1     | 0c        |  1 (py)
bin-homebrew-pkg  | 8.5   |  1     | 1c        |  0
bin-homebrew-sys  | 0.0   |  1     | 1c        |  1 (py)
src-static-ext    | 7.4   |  1     | 1         |  1 (py)
src-static-pkg    |       |        |           |
src-shared-ext    |       |        |           |
src-shared-pkg    |       |        |           |

framework-ext has a build error (fix then retry) static-ext

## relocatable python

see:

- <https://github.com/gregneagle/relocatable-python>

- <https://github.com/beeware/Python-Apple-support>

## max best practices

see: <https://cycling74.com/forums/frameworks-in-external-vs-framework-loading-in-standalone-app>

## dylib search and linking

see:

- <https://stackoverflow.com/questions/24598047/why-does-ld-need-rpath-link-when-linking-an-executable-against-a-so-that-needs>

- <https://stackoverflow.com/questions/9798623/how-to-properly-set-run-paths-search-paths-and-install-names>

- <https://matthew-brett.github.io/docosx/mac_runtime_link.html>

- <https://github.com/trojanfoe/xcodedevtools>

- <http://lessons.livecode.com/m/4071/l/15029-linking-an-osx-external-bundle-with-a-dylib-library>

- <https://wincent.com/wiki/@executable_path>,_@load_path_and_@rpath

- <https://medium.com/@donblas/fun-with-rpath-otool-and-install-name-tool-e3e41ae86172>

- <https://www.mulle-kybernetik.com/weblog/2015/how_to_embed_a_framework_in_a.html>

- <https://stackoverflow.com/questions/12521802/print-rpath-of-an-executable-on-macos>

- <https://stackoverflow.com/questions/10021428/macos-how-to-link-a-dynamic-library-with-a-relative-path-using-gcc-ld>

- <https://stackoverflow.com/questions/9798623/how-to-properly-set-run-paths-search-paths-and-install-names>

- <https://developer.apple.com/library/archive/documentation/DeveloperTools/Conceptual/DynamicLibraries/000-Introduction/Introduction.html>

## xcconfig -- Targets

variables:

BUNDLE_CONTENTS_FOLDER_PATH
BUNDLE_EXECUTABLE_FOLDER_PATH
BUNDLE_FORMAT
BUNDLE_FRAMEWORKS_FOLDER_PATH
BUNDLE_LOADER
BUNDLE_PLUGINS_FOLDER_PATH
BUNDLE_PRIVATE_HEADERS_FOLDER_PATH
BUNDLE_PUBLIC_HEADERS_FOLDER_PATH

COLOR_DIAGNOSTICS

CONFIGURATION

CURRENT_PROJECT_VERSION

CURRENT_VERSION

DEAD_CODE_STRIPPING

DEBUGGING_SYMBOLS

DEPLOYMENT_LOCATION
DEPLOYMENT_POSTPROCESSING

DEVELOPMENT_ASSET_PATHS

FRAMEWORK_SEARCH_PATHS
FRAMEWORK_VERSION

HEADER_SEARCH_PATHS

INFOPLIST_FILE

INSTALL_DIR also INSTALL_PATH

LD_DYLIB_INSTALL_NAME
DYLIB_INSTALL_NAME_BASE
LD_RUNPATH_SEARCH_PATHS

LIBRARY_SEARCH_PATHS

LINK_WITH_STANDARD_LIBRARIES

MAC_OS_X_VERSION_MIN_REQUIRED

OTHER_CFLAGS
OTHER_LDFLAGS
PRODUCT_NAME
PROJECT_NAME
PROJECT_DIR = $(SRCROOT)
SOURCE_ROOT = $(SRCROOT)

STRIP_INSTALLED_PRODUCT

CONTENTS_FOLDER_PATH

see:

- <https://xcodebuildsettings.com>

- <https://nshipster.com/xcconfig/>

- <https://developer.apple.com/library/archive/technotes/tn2339/_index.html>

see: <https://pewpewthespells.com/blog/xcconfig_guide.html>

1 xcconfig per target

## Packaging Options Test

### homebrew-pkg

```bash
make clean-pkg
make homebrew-pkg
```

Issues:

- copies `pyjs.mxo` that is part of another build (system)

Should be:

```bash
make clean clean-pkg homebrew-pkg
```

## Preparing a standalone for Apple App Store

<https://cycling74.com/forums/max-standalone-in-the-mac-app-store-2018>

## Python Distribution

for python 3.8

```bash
NAME=mxpython
PWD=$(pwd)
PREFIX=$PWD/$NAME

./configure MACOSX_DEPLOYMENT_TARGET=10.13 \
  --prefix=$PREFIX \
  --enable-shared \
  --with-universal-archs=64-bit \
  --with-lto \
  --enable-optimizations

```

## `sys.prefix`

A string giving the site-specific directory prefix where the platform independent Python files are installed; by default, this is the string '/usr/local'. This can be set at build time with the --prefix argument to the configure script. The main collection of Python library modules is installed in the directory prefix/lib/pythonX.Y while the platform independent header files (all except pyconfig.h) are stored in prefix/include/pythonX.Y, where X.Y is the version number of Python, for example 3.2.

Note If a virtual environment is in effect, this value will be changed in site.py to point to the virtual environment. The value for the Python installation will still be available, via base_prefix.

## `sys.exec_prefix`

A string giving the site-specific directory prefix where the platform-dependent Python files are installed; by default, this is also '/usr/local'. This can be set at build time with the --exec-prefix argument to the configure script. Specifically, all configuration files (e.g. the pyconfig.h header file) are installed in the directory exec_prefix/lib/pythonX.Y/config, and shared library modules are installed in exec_prefix/lib/pythonX.Y/lib-dynload, where X.Y is the version number of Python, for example 3.2.

Note If a virtual environment is in effect, this value will be changed in site.py to point to the virtual environment. The value for the Python installation will still be available, via base_exec_prefix.

## Max Package Elements

see: <https://docs.cycling74.com/max7/vignettes/packages>

You may create your own packages, either for your own use or for distribution to others. The folders (ending with a slash) and files comprising a package may optionally include the following (items in folders marked with a star will automatically be included in the searchpath):

```
* clippings/            Patchers to list in the "Paste From..." contextual
                        menu when patching

* code/                 Gen patchers

  collections/          Collections to list in the File Browser that are 
                        associated with the package

  default-definitions/  Definition info for Object Defaults support in UI externals

  default-settings/     Saved color schemes for Object Defaults

* docs/                 Reference pages and Vignettes to be accessible from
                        the Documentation Window

* examples/             Example patchers and supporting material

* extensions/           Special external objects loaded on Max launch

* externals/            External objects

* extras/               Patchers to be listed in the "Extras" menu

* help/                 Help patchers and supporting material

  icon.png              A PNG graphic file (500x500px) for display in 
                        the Package Manager

  init/                 Text files interpreted by Max at launch

  interfaces/           Supporting files for objects to display in the top
                        patcher toolbar and other Max integration.

* java-classes/         Compiled Java classes for use in mxj/mxj~. Place .jar 
                        folders in a 'lib' subfolder.

  java-doc/             Documentation for Java classes

* javascript/           Javascript files to be used by js

* jsextensions/         Extensions to JS implemented as special externals or js files

* jsui/                 Javascript files to be used by jsui, and listed in 
                        the contextual menu for jsui

  license.txt|md        Terms of use / redistribution of your package
                        (plain text or Markdown permitted)

* media/                Media files to be included in the searchpath

* misc/                 Anything

* patchers/             Patchers or abstractions to be included in the searchpath

* object-icons/         An SVG-format object icon for a particular Max object
                        (named <objectname>.svg), used in the Object Browser

  object-prototypes/    Object Prototypes will be listed in the context menu for 
                        a selected UI object

  readme.txt|md         Information about your package (text or Markdown permitted)

  snippets/             Snippets associated with this package

  source/               Source code for external objects, ignored by Max

  support/              Special location for DLL or dylib dependencies of 
                        external objects. Added to the DLL search path on Windows.

  templates/            Patchers to be listed in the "File > New From Template" menu

```

## Directory Structure

- py
  - clippings
  - docs
    - refpages (.maxref.xml)
  - externals (.mxo)
  - extras
  - fonts (.otf)
  - help (.maxhelp)
  - interfaces (svg)
  - jsextensions (.mxo / .js)
  - jsui (.js)
  - media (.png/...)
  - object-icons (.svg)
  - object-prototypes (.maxproto)
  - patchers
  - snippets (.maxsnip)
  - styles (.maxstyle)
  - templates
