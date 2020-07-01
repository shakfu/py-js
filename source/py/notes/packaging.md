# Packaging notes


## Design

build -> homebrew-sys

build-homebrew -> support/python -> homebrew(ext/pkg)

build-python -> support/python -> python(ext/pkg)

build-framework -> support/frameworks -> framework(ext/pkg)



## max best practices

see: https://cycling74.com/forums/frameworks-in-external-vs-framework-loading-in-standalone-app




## dylib search and linking

see:

- https://stackoverflow.com/questions/24598047/why-does-ld-need-rpath-link-when-linking-an-executable-against-a-so-that-needs

- https://stackoverflow.com/questions/9798623/how-to-properly-set-run-paths-search-paths-and-install-names

- https://matthew-brett.github.io/docosx/mac_runtime_link.html

- https://github.com/trojanfoe/xcodedevtools

- http://lessons.livecode.com/m/4071/l/15029-linking-an-osx-external-bundle-with-a-dylib-library

- https://wincent.com/wiki/@executable_path,_@load_path_and_@rpath

- https://medium.com/@donblas/fun-with-rpath-otool-and-install-name-tool-e3e41ae86172

- https://www.mulle-kybernetik.com/weblog/2015/how_to_embed_a_framework_in_a.html

- https://stackoverflow.com/questions/12521802/print-rpath-of-an-executable-on-macos

- https://stackoverflow.com/questions/10021428/macos-how-to-link-a-dynamic-library-with-a-relative-path-using-gcc-ld


- https://stackoverflow.com/questions/9798623/how-to-properly-set-run-paths-search-paths-and-install-names


- https://developer.apple.com/library/archive/documentation/DeveloperTools/Conceptual/DynamicLibraries/000-Introduction/Introduction.html

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

- https://xcodebuildsettings.com

- https://nshipster.com/xcconfig/

- https://developer.apple.com/library/archive/technotes/tn2339/_index.html


see: https://pewpewthespells.com/blog/xcconfig_guide.html

1 xcconfig per target


## Packaging Options Test

### homebrew-pkg

```
make clean-pkg
make homebrew-pkg
```
Issues:

- copies `pyjs.mxo` that is part of another build (system)

Should be:


```
make clean clean-pkg homebrew-pkg
```




## Preparing a standalone for Apple App Store

https://cycling74.com/forums/max-standalone-in-the-mac-app-store-2018


## Python Distribution

for python 3.8

```
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
see: https://docs.cycling74.com/max7/vignettes/packages

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

