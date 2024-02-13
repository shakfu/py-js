import configparser
import logging
import os
import platform
import sysconfig
from pathlib import Path

try:
    import numpy
except ImportError:
    numpy = None


# ----------------------------------------------------------------------------
# Logging

DEBUG = False
LOG_LEVEL = logging.DEBUG if DEBUG else logging.INFO
LOG_FORMAT = "%(relativeCreated)-4d %(levelname)-5s: %(name)-10s %(message)s"

# ----------------------------------------------------------------------------
# CONSTANTS

HOME = os.environ["HOME"]

CURRENT_PYTHON_VERSION = platform.python_version()
DEFAULT_BZ2_VERSION = "1.0.8"
DEFAULT_SSL_VERSION = "1.1.1w"
DEFAULT_XZ_VERSION = "5.2.5"

# BASEDIR=f"{HOME}/.build_pyjs"
BASEDIR = Path(__file__).parent.parent.parent.parent.parent
BASELOGSDIR = f"{BASEDIR}/logs"
LOGDIR = f"{BASELOGSDIR}/{CURRENT_PYTHON_VERSION}"
ESCAPED_HOME = HOME.replace("/", "\\/")

# useful urls
URL_GETPIP = "https://bootstrap.pypa.io/get-pip.py"
URL_PYTHON_CMAKE_BUILDSYSTEM = "https://github.com/shakfu/python-cmake-buildsystem"


# colors
YELLOW = "\033[1;33m"
BLUE = "\033[1;34m"
GREEN = "\033[1;32m"
MAGENTA = "\033[1;35m"
CYAN = "\033[1;36m"
RESET = "\033[m"

DEFAULT_CONFIGURE_OPTIONS = [
    "disable_profiling",
    "disable_test_modules",
    "enable_ipv6",
    "enable_optimizations",
    "with_lto",
    "without_doc_strings",
    "without_readline",
    # "with_readline=editline",
    # "with_lto=thin",
    # "with_system_libmpdec",
    # "with_system_expat",
    # "with_system_ffi",
    # "with_openssl=DIR",
    # "with_openssl_rpath=auto",
    # "enable_universalsdk",
    # "enable_universalsdk=SDKDIR",
    # "enable_framework",
    # "enable_framework=INSTALLDIR",
]

PYJS_TARGETS = {
    "default": dict(
        desc="non-portable pyjs externals linked to your system", lines=210
    ),
    "homebrew-pkg": dict(
        desc="portable package w/ pyjs (requires homebrew python)", lines=275
    ),
    "homebrew-ext": dict(
        desc="portable pyjs externals (requires homebrew python)", lines=278
    ),
    "shared-pkg": dict(
        desc="portable package with pyjs externals (shared)", lines=17992
    ),
    "shared-ext": dict(desc="portable pyjs externals (shared)", lines=14195),
    "shared-tiny-ext": dict(desc="tiny portable pyjs externals (shared)", lines=14195),
    "static-ext": dict(desc="portable pyjs externals (static)", lines=14064),
    "static-tiny-ext": dict(desc="tiny portable pyjs externals (static)", lines=14064),
    "framework-pkg": dict(
        desc="portable package with pyjs externals (framework)", lines=14383
    ),
    "framework-ext": dict(desc="portable pyjs externals (framework)", lines=18203),
    "relocatable-pkg": dict(
        desc="portable package w/ more custom options (framework)", lines=414
    ),
    # "pymx"          : dict(desc="non-portable alternative python3 externals (min-lib)", lines=210),
}

PYTHON_TARGETS = {
    "python-shared": "minimal shared python build",
    "python-shared-ext": "minimal shared python build for externals",
    "python-shared-pkg": "minimal shared python build for packages",
    "python-static": "minimal statically-linked python build",
    "python-static-tiny": "tiny statically-linked python build",
    "python-framework": "minimal framework python build",
    "python-framework-ext": "minimal framework python build for externals",
    "python-framework-pkg": "minimal framework python build for packages",
    "python-relocatable": "custom relocatable python framework build",
}


CMAKE_DEFAULT_OPTIONS = {
    "PYTHON_VERSION": "3.9.17",
    "PYTHON_APPLY_PATCHES": "ON",
    "CMAKE_BUILD_TYPE": "Debug",
    "DOWNLOAD_SOURCES": "ON",
    "BUILD_LIBPYTHON_SHARED": "OFF",
    "BUILD_EXTENSIONS_AS_BUILTIN": "OFF",
    "WITH_STATIC_DEPENDENCIES": "OFF",
    "INSTALL_DEVELOPMENT": "ON",
    "INSTALL_MANUAL": "OFF",
    "INSTALL_TEST": "OFF",
    "USE_LIB64": "OFF",
    "WITH_C_LOCALE_COERCION": "ON",
    "WITH_DECIMAL_CONTEXTVAR": "ON",
    "WITH_TRACE_REFS": "OFF",
    "WITH_SSL_DEFAULT_SUITES": "python",
    "EXTRA_PYTHONPATH": "",  # colon (:) separated
    "USE_SYSTEM_LIBRARIES": "ON",
    "USE_SYSTEM_Curses": "ON",
    "USE_SYSTEM_EXPAT": "ON",
    "USE_SYSTEM_LibFFI": "OFF",
    "USE_SYSTEM_OpenSSL": "ON",
    "USE_SYSTEM_TCL": "ON",
    "USE_SYSTEM_ZLIB": "ON",
    "USE_SYSTEM_DB": "ON",
    "USE_SYSTEM_GDBM": "ON",
    "USE_SYSTEM_LZMA": "ON",
    "USE_SYSTEM_READLINE": "ON",
    "USE_SYSTEM_SQLite3": "ON",
    # extension options
    "ENABLE_ARRAY": "ON",
    "ENABLE_AUDIOOP": "ON",
    "ENABLE_BINASCII": "ON",
    "ENABLE_BISECT": "ON",
    "ENABLE_BSDDB": "ON",
    "ENABLE_BZ2": "ON",
    "ENABLE_CMATH": "ON",
    "ENABLE_CODECS_CN": "ON",
    "ENABLE_CODECS_HK": "ON",
    "ENABLE_CODECS_ISO2022": "ON",
    "ENABLE_CODECS_JP": "ON",
    "ENABLE_CODECS_KR": "ON",
    "ENABLE_CODECS_TW": "ON",
    "ENABLE_COLLECTIONS": "ON",
    "ENABLE_CPICKLE": "ON",
    "ENABLE_CRYPT": "ON",
    "ENABLE_CSTRINGIO": "ON",
    "ENABLE_CSV": "ON",
    "ENABLE_CTYPES": "ON",
    "ENABLE_CTYPES_TEST": "ON",
    "ENABLE_CURSES": "ON",
    "ENABLE_CURSES_PANEL": "ON",
    "ENABLE_DATETIME": "ON",
    "ENABLE_DBM": "ON",
    "ENABLE_ELEMENTTREE": "ON",
    "ENABLE_FCNTL": "ON",
    "ENABLE_FUNCTOOLS": "ON",
    "ENABLE_FUTURE_BUILTINS": "ON",
    "ENABLE_GDBM": "ON",
    "ENABLE_GRP": "ON",
    "ENABLE_HASHLIB": "ON",
    "ENABLE_HEAPQ": "ON",
    "ENABLE_HOTSHOT": "ON",
    "ENABLE_IO": "ON",
    "ENABLE_ITERTOOLS": "ON",
    "ENABLE_JSON": "ON",
    "ENABLE_LINUXAUDIODEV": "ON",
    "ENABLE_LOCALE": "ON",
    "ENABLE_LSPROF": "ON",
    "ENABLE_LZMA": "ON",
    "ENABLE_MATH": "ON",
    "ENABLE_MMAP": "ON",
    "ENABLE_MULTIBYTECODEC": "ON",
    "ENABLE_MULTIPROCESSING": "ON",
    "ENABLE_NIS": "ON",
    "ENABLE_NT": "ON",
    "ENABLE_OPERATOR": "ON",
    "ENABLE_OSSAUDIODEV": "ON",
    "ENABLE_PARSER": "ON",
    "ENABLE_POSIX": "ON",
    "ENABLE_PWD": "ON",
    "ENABLE_PYEXPAT": "ON",
    "ENABLE_RANDOM": "ON",
    "ENABLE_READLINE": "ON",
    "ENABLE_RESOURCE": "ON",
    "ENABLE_SELECT": "ON",
    "ENABLE_SOCKET": "ON",
    "ENABLE_SPWD": "ON",
    "ENABLE_SQLITE3": "ON",
    "ENABLE_SSL": "ON",
    "ENABLE_STROP": "ON",
    "ENABLE_STRUCT": "ON",
    "ENABLE_SYSLOG": "ON",
    "ENABLE_TERMIOS": "ON",
    "ENABLE_TESTCAPI": "ON",
    "ENABLE_TIME": "ON",
    "ENABLE_TKINTER": "ON",
    "ENABLE_UNICODEDATA": "ON",
    "ENABLE_ZLIB": "ON",
    "BUILTIN_ARRAY": "OFF",
    "BUILTIN_AUDIOOP": "OFF",
    "BUILTIN_BINASCII": "OFF",
    "BUILTIN_BISECT": "OFF",
    "BUILTIN_BSDDB": "OFF",
    "BUILTIN_BZ2": "OFF",
    "BUILTIN_CMATH": "OFF",
    "BUILTIN_CODECS_CN": "OFF",
    "BUILTIN_CODECS_HK": "OFF",
    "BUILTIN_CODECS_ISO2022": "OFF",
    "BUILTIN_CODECS_JP": "OFF",
    "BUILTIN_CODECS_KR": "OFF",
    "BUILTIN_CODECS_TW": "OFF",
    "BUILTIN_COLLECTIONS": "OFF",
    "BUILTIN_CPICKLE": "OFF",
    "BUILTIN_CRYPT": "OFF",
    "BUILTIN_CSTRINGIO": "OFF",
    "BUILTIN_CSV": "OFF",
    "BUILTIN_CTYPES": "OFF",
    "BUILTIN_CTYPES_TEST": "OFF",
    "BUILTIN_CURSES": "OFF",
    "BUILTIN_CURSES_PANEL": "OFF",
    "BUILTIN_DATETIME": "OFF",
    "BUILTIN_DBM": "OFF",
    "BUILTIN_ELEMENTTREE": "OFF",
    "BUILTIN_FCNTL": "OFF",
    "BUILTIN_FUNCTOOLS": "OFF",
    "BUILTIN_FUTURE_BUILTINS": "OFF",
    "BUILTIN_GDBM": "OFF",
    "BUILTIN_GRP": "OFF",
    "BUILTIN_HASHLIB": "OFF",
    "BUILTIN_HEAPQ": "OFF",
    "BUILTIN_HOTSHOT": "OFF",
    "BUILTIN_IO": "OFF",
    "BUILTIN_ITERTOOLS": "OFF",
    "BUILTIN_JSON": "OFF",
    "BUILTIN_LINUXAUDIODEV": "OFF",
    "BUILTIN_LOCALE": "OFF",
    "BUILTIN_LSPROF": "OFF",
    "BUILTIN_LZMA": "OFF",
    "BUILTIN_MATH": "OFF",
    "BUILTIN_MMAP": "OFF",
    "BUILTIN_MULTIBYTECODEC": "OFF",
    "BUILTIN_MULTIPROCESSING": "OFF",
    "BUILTIN_NIS": "OFF",
    "BUILTIN_NT": "ON",
    "BUILTIN_OPERATOR": "OFF",
    "BUILTIN_OSSAUDIODEV": "OFF",
    "BUILTIN_PARSER": "OFF",
    "BUILTIN_POSIX": "ON",
    "BUILTIN_PWD": "ON",
    "BUILTIN_PYEXPAT": "OFF",
    "BUILTIN_RANDOM": "OFF",
    "BUILTIN_READLINE": "OFF",
    "BUILTIN_RESOURCE": "OFF",
    "BUILTIN_SELECT": "OFF",
    "BUILTIN_SOCKET": "OFF",
    "BUILTIN_SPWD": "OFF",
    "BUILTIN_SQLITE3": "OFF",
    "BUILTIN_SSL": "OFF",
    "BUILTIN_STROP": "OFF",
    "BUILTIN_STRUCT": "OFF",
    "BUILTIN_SYSLOG": "OFF",
    "BUILTIN_TERMIOS": "OFF",
    "BUILTIN_TESTCAPI": "OFF",
    "BUILTIN_TIME": "OFF",
    "BUILTIN_TKINTER": "OFF",
    "BUILTIN_UNICODEDATA": "OFF",
    "BUILTIN_ZLIB": "OFF",
}

PYJS_CMAKE_DEFAULT_OPTIONS = CMAKE_DEFAULT_OPTIONS.copy()
PYJS_CMAKE_DEFAULT_OPTIONS.update(
    {
        "CMAKE_INSTALL_PREFIX:PATH": None,
        "BUILD_EXTENSIONS_AS_BUILTIN": "ON",
        "BUILD_LIBPYTHON_SHARED": "ON",
        "USE_SYSTEM_LIBRARIES": "OFF",
        "USE_SYSTEM_ZLIB": "OFF",
        "PYTHON_APPLY_PATCHES": "OFF",
        "WITH_C_LOCALE_COERCION": "OFF",
        # extensions
        "ENABLE_AUDIOOP": "OFF",
        "ENABLE_BSDDB": "OFF",
        "ENABLE_CODECS_CN": "OFF",
        "ENABLE_CODECS_HK": "OFF",
        "ENABLE_CODECS_ISO2022": "OFF",
        "ENABLE_CODECS_JP": "OFF",
        "ENABLE_CODECS_KR": "OFF",
        "ENABLE_CODECS_TW": "OFF",
        "ENABLE_CPICKLE": "OFF",
        "ENABLE_CTYPES": "OFF",
        "ENABLE_CTYPES_TEST": "OFF",
        "ENABLE_CURSES": "OFF",
        "ENABLE_CURSES_PANEL": "OFF",
        "ENABLE_DBM": "OFF",
        "ENABLE_GDBM": "OFF",
        "ENABLE_HASHLIB": "OFF",
        "ENABLE_HOTSHOT": "OFF",
        "ENABLE_LINUXAUDIODEV": "OFF",
        "ENABLE_LOCALE": "OFF",
        "ENABLE_LSPROF": "OFF",
        "ENABLE_MULTIBYTECODEC": "OFF",
        "ENABLE_NIS": "OFF",
        "ENABLE_OSSAUDIODEV": "OFF",
        "ENABLE_READLINE": "OFF",
        "ENABLE_SPWD": "OFF",
        "ENABLE_SQLITE3": "OFF",
        "ENABLE_SSL": "OFF",
        "ENABLE_SYSLOG": "OFF",
        "ENABLE_TERMIOS": "OFF",
        "ENABLE_TESTCAPI": "OFF",
        "ENABLE_TKINTER": "OFF",
        "ENABLE_UNICODEDATA": "OFF",
    }
)

# ----------------------------------------------------------------------------
# Utility Functions


def get_var(x):
    return sysconfig.get_config_var(x)  # type: ignore


def get_path(x):
    return Path(sysconfig.get_config_var(x))  # type: ignore


# ----------------------------------------------------------------------------
# Configuration Classes


class Python:
    """configuration object to to get info about the python implementation used"""

    version = get_var("py_version")
    version_short = get_var("py_version_short")
    version_nodot = get_var("py_version_nodot")
    name = f"python{version_short}"
    abiflags = get_var("abiflags")
    arch = platform.machine()

    tag = f"macOS-{arch}-py{version}"

    prefix = get_path("prefix")
    bindir = get_path("BINDIR")
    include = get_path("INCLUDEPY")
    libdir = get_path("LIBDIR")
    libs = get_path("LIBS")
    pkgs = get_path("BINLIBDEST")

    mac_dep_target = get_var("MACOSX_DEPLOYMENT_TARGET")
    staticlib = library = get_var("LIBRARY")

    # can be either:
    # in case of framework: Python.framework/Versions/X.Y/Python
    # in case of shared lib: libpythonX.Ym.dylib
    ldlibrary = get_path("LDLIBRARY")
    dylib = f"libpython{version_short}{abiflags}.dylib"

    config_ver_platform = f"config-{version_short}{abiflags}-darwin"

    numpy_includes = numpy.get_include() if numpy else None

    def to_dict(self) -> dict:
        return {
            "version": self.version,
            "version_short": self.version_short,
            "version_nodot": self.version_nodot,
            "name": self.name,
            "abiflags": self.abiflags,
            "arch": self.arch,
            "prefix": str(self.prefix),
            "bindir": str(self.bindir),
            "include": str(self.include),
            "libdir": str(self.libdir),
            "libs": str(self.libs),
            "mac_dep_target": self.mac_dep_target,
            "staticlib": self.staticlib,
            "ldlibrary": str(self.ldlibrary),
            "dylib": self.dylib,
            "config_ver_platform": self.config_ver_platform,
        }


class Project:
    """A place for all the files, resources, and information required to
    build one or more software products.
    """

    name = "py-js"

    python = Python()

    arch = platform.machine()

    system = platform.system()

    HOME = Path(HOME)

    # environmental vars
    # TODO: package_name should be
    #  py-js-<variation>-<platform>-<arch>-<py_ver> for example
    #  py-js-shared-ext-darwin-x86-3.11
    package_name = name
    package = Path(f"{HOME}/Documents/Max 8/Packages/{package_name}")
    package_dirs = [
        "docs",
        "examples",
        "externals",
        "extras",
        "help",
        "init",
        "javascript",
        "jsextensions",
        "media",
        "patchers",
        "support",
    ]

    is_symlinked = True

    # project root here
    root = BASEDIR  # i.e py-js
    pydir = root / "source" / "projects" / "py"
    support = root / "support"
    externals = root / "externals"

    py_external = externals / "py.mxo"
    pyjs_external = externals / "pyjs.mxo"

    # resources
    resources = pydir / "resources"
    entitlements = resources / "entitlements"
    addons = resources / "addons"
    patch = resources / "patch"

    # project-build section
    scripts = pydir / "scripts"
    targets = pydir / "targets"

    # dmg = root / f"{name}.dmg"

    if is_symlinked:
        build = targets / "build"
        build_externals = externals

    else:  # is copied to {package}
        build = HOME / ".build_pyjs"
        build_externals = build / "externals"

    build_cache = build / "build.ini"
    build_downloads = build / "downloads"
    build_src = build / "src"
    build_lib = build / "lib"

    # collect stapled and zipped .dmgs in release directory
    release_dir = HOME / "Downloads" / "PYJS_RELEASE"

    # settings
    mac_dep_target = "10.13"

    def to_dict(self) -> dict:
        return {
            "name": self.name,
            "arch": self.arch,
            "root": str(self.root),
            "pydir": str(self.pydir),
            "scripts": str(self.scripts),
            "patch": str(self.patch),
            "targets": str(self.targets),
            "build": str(self.build),
            "downloads": str(self.build_downloads),
            "build_src": str(self.build_src),
            "lib": str(self.build_lib),
            "support": str(self.support),
            "externals": str(self.externals),
            "py_external": str(self.py_external),
            "pyjs_external": str(self.pyjs_external),
            "HOME": self.HOME,
            "package_name": self.package_name,
            "package": str(self.package),
            "package_dirs": self.package_dirs,
            "mac_dep_target": self.mac_dep_target,
            "python": self.python.to_dict(),
        }

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    def __repr__(self):
        return self.__str__()

    def __hash__(self):
        return hash((self.name, self.python.name, self.mac_dep_target))

    def cache_set(self, **kwds):
        config = configparser.ConfigParser()
        config["cache"] = kwds
        if not self.build.exists():
            self.build.mkdir(exist_ok=True)
        with open(self.build_cache, "w") as configfile:
            config.write(configfile)

    def cache_get(self, key, as_path=False):
        config = configparser.ConfigParser()
        config.read(self.build_cache)
        value = config["cache"][key]
        if as_path:
            value = Path(value)
        return value

    def get_package_name(self, variant):
        """ensure package name has standard format.

        `py-js-<variant>-<system>-<arch>-<py_ver>` for example
        `py-js-shared-ext-darwin-x86-3.11`
        """
        name = self.name
        system = self.system.lower()
        arch = self.arch
        ver = self.python.version
        return f"{name}-{variant}-{system}-{arch}-{ver}"

    def get_dmg(self, variant):
        """get final dmg package name and path"""
        package_name = self.get_package_name(variant)
        dmg = self.root / f"{package_name}.dmg"
        return dmg.resolve()

    def record_variant(self, name):
        if name.startswith("pyjs"):
            variant = name[len("pyjs_") :].replace("_", "-")
            self.cache_set(
                VARIANT=variant,
                PRODUCT_DMG=self.get_dmg(variant),
            )
