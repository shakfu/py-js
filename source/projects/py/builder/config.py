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
DEFAULT_SSL_VERSION = "1.1.1n"
DEFAULT_XZ_VERSION = "5.2.5"

# BASEDIR=f"{HOME}/.build_pyjs"
BASEDIR = Path(__file__).parent.parent.parent.parent.parent
BASELOGSDIR = f"{BASEDIR}/logs"
LOGDIR = f"{BASELOGSDIR}/{CURRENT_PYTHON_VERSION}"
ESCAPED_HOME = HOME.replace("/", "\\/")

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
    "static-ext": dict(desc="portable pyjs externals (static)", lines=14064),
    "tiny-static-ext": dict(desc="tiny portable pyjs externals (static)", lines=14064),
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
    "python-framework": "minimal framework python build",
    "python-framework-ext": "minimal framework python build for externals",
    "python-framework-pkg": "minimal framework python build for packages",
    "python-relocatable": "custom relocatable python framework build",
}


PACKAGES_TO_DEL = [
    # project.python.config_ver_platform,
    "config-{ver}-{platform}",
    "idlelib",
    "lib2to3",
    "tkinter",
    "turtledemo",
    "turtle.py",
    "ctypes",
    "curses",
    "ensurepip",
    "venv",
]

PACKAGES_TO_KEEP = [
    "asyncio",
    "collections",
    "concurrent",
    # 'ctypes',
    # 'curses',
    "dbm",
    "distutils",
    "email",
    "encodings",
    "html",
    "http",
    "importlib",
    "json",
    "lib-dynload",
    "logging",
    "multiprocessing",
    "pydoc_data",
    "sqlite3",
    "unittest",
    "urllib",
    "wsgiref",
    "xml",
    "xmlrpc",
]

EXTENSIONS_TO_DEL = [
    "_tkinter",
    "_ctypes",
    "_multibytecodec",
    "_codecs_jp",
    "_codecs_hk",
    "_codecs_cn",
    "_codecs_kr",
    "_codecs_tw",
    "_codecs_iso2022",
    "_curses",
    "_curses_panel",
]

BINS_TO_DEL = [
    "2to3-{ver}",
    "idle{ver}",
    "easy_install-{ver}",
    "pip{ver}",
    "pyvenv-{ver}",
    "pydoc{ver}",
]


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
    root = BASEDIR # i.e py-js
    pydir = root / 'source' / 'projects' / 'py'
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
    release_dir = HOME / 'Downloads' / 'PYJS_RELEASE'

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
        with open(self.build_cache, 'w') as configfile:
            config.write(configfile)

    def cache_get(self, key, as_path=False):
        config = configparser.ConfigParser()
        config.read(self.build_cache)
        value = config['cache'][key]
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
        """get final dmg package name and path
        """
        package_name = self.get_package_name(variant)
        dmg = self.root / f'{package_name}.dmg'
        return dmg.resolve()

    def record_variant(self, name):
        if name.startswith('pyjs'):
            variant = name[len('pyjs_'):].replace('_','-')
            self.cache_set(
                VARIANT=variant,
                PRODUCT_DMG=self.get_dmg(variant),
            )

