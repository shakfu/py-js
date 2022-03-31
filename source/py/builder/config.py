import os
import logging
import platform
import sysconfig
from pathlib import Path


# ----------------------------------------------------------------------------
# Logging Configuration

DEBUG = False
LOG_LEVEL = logging.DEBUG if DEBUG else logging.INFO
LOG_FORMAT = "%(relativeCreated)-4d %(levelname)-5s: %(name)-10s %(message)s"

# ----------------------------------------------------------------------------
# Utility Functions


def get_var(x): return sysconfig.get_config_var(x)         # type: ignore
def get_path(x): return Path(sysconfig.get_config_var(x))  # type: ignore


# ----------------------------------------------------------------------------
# Configuration Classes

class Python:
    """configuration object to to get info about the python implementation used
    """

    version = get_var('py_version')
    version_short = get_var('py_version_short')
    version_nodot = get_var('py_version_nodot')
    name = f"python{version_short}"
    abiflags = get_var('abiflags')
    arch = platform.machine()

    prefix = get_path('prefix')
    bindir = get_path('BINDIR')
    include = get_path('INCLUDEPY')
    libdir = get_path('LIBDIR')
    libs = get_path('LIBS')
    pkgs = get_path('BINLIBDEST')

    mac_dep_target = get_var('MACOSX_DEPLOYMENT_TARGET')
    staticlib = library = get_var('LIBRARY')

    # can be either: 
    # in case of framework: Python.framework/Versions/X.Y/Python
    # in case of shared lib: libpythonX.Ym.dylib
    ldlibrary = get_path('LDLIBRARY')
    dylib = f"libpython{version_short}{abiflags}.dylib"

    config_ver_platform = f"config-{version_short}{abiflags}-darwin"

    def to_dict(self) -> dict:
        return {
            'version': self.version,
            'version_short': self.version_short,
            'version_nodot': self.version_nodot,
            'name': self.name,
            'abiflags': self.abiflags,
            'arch': self.arch,
            'prefix': str(self.prefix),
            'bindir': str(self.bindir),
            'include': str(self.include),
            'libdir': str(self.libdir),
            'libs': str(self.libs),
            'mac_dep_target': self.mac_dep_target,
            'staticlib': self.staticlib,
            'ldlibrary': str(self.ldlibrary),
            'dylib': self.dylib,
            'config_ver_platform': self.config_ver_platform,
        }


class Project:
    """A place for all the files, resources, and information required to
    build one or more software products.
    """

    name = "py-js"
    python = Python()
    
    arch = platform.machine()

    HOME = Path(os.getenv("HOME"))

    # environmental vars
    package_name = "py-js"
    package = Path(f"{HOME}/Documents/Max 8/Packages/{package_name}")
    package_dirs = [
        "docs",
        "examples",
        "externals",
        "help",
        "init",
        "javascript",
        "jsextensions",
        "media",
        "patchers",
    ]

    is_symlinked = True

    # root in this case is root assumed for build / make / scripts
    # actual project is root.parent.parent (see below)
    # current working directory
    root = Path.cwd()

    # project root here
    pyjs = root.parent.parent
    support = pyjs / "support"
    externals = pyjs / "externals"

    py_external = externals / "py.mxo"
    pyjs_external = externals / "pyjs.mxo"

    # project-build section
    scripts = root / "scripts"
    patch = root / "patch"
    targets = root / "targets"

    if is_symlinked:
        build = targets / "build"
        build_externals = externals

    else: # is copied to {package}
        build = HOME / ".build_pyjs"
        build_externals = build / 'externals'

    build_downloads = build / "downloads"
    build_src = build / "src"
    build_lib = build / "lib"

    # settings
    mac_dep_target = "10.13"

    def to_dict(self) -> dict:
        return {
            'name': self.name,
            'arch': self.arch,
            'root': str(self.root),
            'scripts': str(self.scripts),
            'patch': str(self.patch),
            'targets': str(self.targets),
            'build': str(self.build),
            'downloads': str(self.downloads),
            'src': str(self.src),
            'lib': str(self.lib),
            'pyjs': str(self.pyjs),
            'support': str(self.support),
            'externals': str(self.externals),
            'py_external': str(self.py_external),
            'pyjs_external': str(self.pyjs_external),
            'HOME': self.HOME,
            'package_name': self.package_name,
            'package': str(self.package),
            'package_dirs': self.package_dirs,
            'mac_dep_target': self.mac_dep_target,
            'python': self.python.to_dict(),
        }

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    def __repr__(self):
        return self.__str__()

    def __hash__(self):
        return hash((self.name, self.python.name, self.mac_dep_target))