# xcode configure via xcconfig

## key vars

### INSTALL_ROOT

Default Value	$(DSTROOT)

### INSTALL_PATH

The directory in which to install the build products. This path is prepended by the Installation Build Products Location (DSTROOT).

### INSTALL_DIR

Identifies the directory in the developer’s filesystem into which the installed product is placed

default value: $(DSTROOT)$(INSTALL_PATH)

### EXECUTABLE_PATH

Specifies the path to the binary the target produces within its bundle.

### DYLIB_INSTALL_NAME_BASE

Sets the base value for the internal install path (LC_ID_DYLIB) in a dynamic library. This will be combined with the EXECUTABLE_PATH to form the full install path. Setting Dynamic Library Install Name (LD_DYLIB_INSTALL_NAME) directly will override this setting. This setting defaults to the target’s Installation Directory (INSTALL_PATH). It is ignored when building any product other than a dynamic library.


### LD_DYLIB_INSTALL_NAME

Sets an internal install path (LC_ID_DYLIB) in a dynamic library. Any clients linked against the library will record that path as the way dyld should locate this library. If this option is not specified, then the -o path will be used. This setting is ignored when building any product other than a dynamic library. See Dynamic Library Programming Topics.


### LD_RUNPATH_SEARCH_PATHS

This is a list of paths to be added to the runpath search path list for the image being created. At runtime, dyld uses the runpath when searching for dylibs whose load path begins with @rpath/. See Dynamic Library Programming Topics.


### CODE_SIGN_IDENTITY

The name, also known as the common name, of a valid code-signing certificate in a keychain within your keychain path. A missing or invalid certificate will cause a build error.





## xcconfig

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