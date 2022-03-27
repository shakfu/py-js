"""builder: a builder of py-js max externals

A pure python builder without any dependencies except the standard library.

Python
Project
ShellCmd
Settings
Product
Recipe
Builder
    Bzip2Builder
    OpensslBuilder
    XzBuilder
    PythonBuilder
        PythonSrcBuilder
            FrameworkPythonBuilder
            SharedPythonBuilder
                SharedPythonForExtBuilder
                SharedPythonForPkgBuilder
            StaticPythonBuilder
        PyJsBuilder
            HomebrewBuilder
            StaticExtBuilder
                StaticExtFullBuilder
            SharedExtBuilder
            SharedPkgBuilder
            FrameworkExtBuilder
            FrameworkPkgBuilder
            RelocatablePkgBuilder

install:
    configure -> reset -> download -> pre_process -> build -> post_process


configure:
    if settings.py_version:
        product.version = settings.py_version

reset:
    cmd.remove(src_path)
    cmd.remove(project.lib / "Python.framework")

    or
    cmd.remove(prefix)


pre_process:
    write_setup_local()
    apply_patch(patch="configure.patch", to_file="configure")

build:
    for builder in depends_on:
        builder.build

post_process:
    clean
    ziplib
    fix
    sign

clean:
    clean_python_pyc(prefix)
    clean_python_tests(python_lib)
    clean_python_site_packages()

    for i in (python_lib / "distutils" / "command").glob("*.exe"):
        cmd.remove(i)

    cmd.remove(prefix_lib / "pkgconfig")
    cmd.remove(prefix / "share")

    remove_packages()
    remove_extensions()
    remove_binaries()
"""

import logging
import os
import platform
import re
import shutil
import subprocess
import sysconfig
from textwrap import dedent
from pathlib import Path
from types import SimpleNamespace
from typing import List, Dict, Optional

from .depend import DependencyManager, PATTERNS_TO_FIX
from . import constants

DEBUG = False
LOG_LEVEL = logging.DEBUG if DEBUG else logging.INFO
LOG_FORMAT = "%(relativeCreated)-4d %(levelname)-5s: %(name)-10s %(message)s"
URL_GETPIP = "https://bootstrap.pypa.io/get-pip.py"

logging.basicConfig(format=LOG_FORMAT, level=LOG_LEVEL)


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

    # root in this case is root assumed for build / make / scripts
    # actual project is root.parent.parent (see below)
    # current working directory
    root = Path.cwd()

    # project-build section
    scripts = root / "scripts"
    patch = root / "patch"
    targets = root / "targets"
    # build = Path('/tmp/_build_pyjs')
    # build = targets / "build"
    build = HOME / ".build_pyjs"
    build_externals = build / 'externals'
    build_downloads = build / "downloads"
    build_src = build / "src"
    build_lib = build / "lib"

    # project root here
    pyjs = root.parent.parent
    support = pyjs / "support"
    externals = pyjs / "externals"

    py_external = externals / "py.mxo"
    pyjs_external = externals / "pyjs.mxo"

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


# ----------------------------------------------------------------------------
# Utility Classes


class ShellCmd:
    """Provides platform agnostic file/folder handling."""

    def __init__(self, log):
        self.log = log

    def cmd(self, shellcmd, *args, **kwargs):
        """Run shell command with args and keywords"""
        _cmd = shellcmd.format(*args, **kwargs)
        self.log.info(_cmd)
        os.system(_cmd)

    __call__ = cmd

    def chdir(self, path):
        """Change current workding directory to path"""
        self.log.info("changing working dir to: %s", path)
        os.chdir(path)

    def chmod(self, path, perm=0o777):
        """Change permission of file"""
        self.log.info("change permission of %s to %s", path, perm)
        os.chmod(path, perm)

    def move(self, src, dst):
        """Move from src path to dst path."""
        self.log.info("move path %s to %s", src, dst)
        shutil.move(src, dst)

    def copy(self, src: Path, dst: Path):
        """copy file or folders -- tries to be behave like `cp -rf`"""
        self.log.info("copy %s to %s", src, dst)
        src, dst = Path(src), Path(dst)
        if dst.exists():
            dst = dst / src.name
        if src.is_dir():
            shutil.copytree(src, dst)
        else:
            shutil.copy2(src, dst)

    def remove(self, path):
        """Remove file or folder."""
        if path.is_dir():
            self.log.info("remove folder: %s", path)
            shutil.rmtree(path, ignore_errors=(not DEBUG))
        else:
            self.log.info("remove file: %s", path)
            try:
                # path.unlink(missing_ok=True)
                path.unlink()
            except FileNotFoundError:
                self.log.warning("file not found: %s", path)


class Settings(SimpleNamespace):
    """A dictionary object with dotted access to its members.

    >>> settings = Settings(**dict)
    """

    def __str__(self):
        return str(self.__dict__)

    def copy(self) -> "Settings":
        """provide a copy of the internal dictionary"""
        return Settings(**self.__dict__.copy())

    def update(self, other):
        """Like a dict.update but using the internal dict instead"""
        if isinstance(other, dict):
            self.__dict__.update(other)
        elif isinstance(other, Settings):
            self.__dict__.update(other.__dict__)
        else:
            raise TypeError


# ----------------------------------------------------------------------------
# Implementation Classes


class Product:
    """A product of a builder."""

    def __init__(
        self,
        name: str,
        version: str = None,  # type: ignore
        build_dir: str = None,  # type: ignore
        libs_static: List[str] = None,  # type: ignore
        url_template: str = None,  # type: ignore
        **settings,
    ):
        self.name = name
        self.version = version
        self.build_dir = build_dir or self.name
        self.libs_static = libs_static or []
        self.url_template = url_template
        self.settings = Settings(**settings)

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    @property
    def ver(self) -> str:
        """provides major.minor version: 3.9.1 -> 3.9"""
        return ".".join(self.version.split(".")[:2])

    @property
    def ver_nodot(self) -> str:
        """provides 'majorminor' version: 3.9.1 -> 39"""
        return self.ver.replace(".", "")

    @property
    def name_version(self) -> str:
        """Product-version: Python-3.9.1"""
        return f"{self.name}-{self.version}"

    @property
    def name_ver(self) -> str:
        """Product(major.minor): python3.9"""
        return f"{self.name.lower()}{self.ver}"

    @property
    def name_archive(self):
        """Archival name of Product-version"""
        # return f"{self.name_version}.tgz"
        if self.url:
            return self.url.name

    @property
    def dylib(self) -> str:
        """name of dynamic library in macos case."""
        ver = '3.7m' if self.ver == '3.7' else self.ver
        return f"lib{self.name.lower()}{ver}.dylib"

    @property
    def url(self):
        """Returns url to download product src as a pathlib.Path instance."""
        if self.url_template:
            return Path(self.url_template.format(name=self.name, version=self.version))
        # raise KeyError("url_template not providing in settings")

    def to_dict(self) -> Dict:
        return {
            'name': self.name,
            'version': self.version,
            'build_dir': str(self.build_dir),
            'libs_static': self.libs_static,
            'url_template': self.url_template,
            'settings': str(self.settings),
            'ver': self.ver,
            'ver_nodot': self.ver_nodot,
            'name_version': self.name_version,
            'name_ver': self.name_ver,
            'name_archive': self.name_archive,
            'dylib': self.dylib,
            'url': str(self.url),
        }


class Builder:
    """A Builder know how to build a single product type in a project."""

    def __init__(
        self,
        product: Product,
        project: Project = None,  # type: ignore
        depends_on: List["Builder"] = None,  # type: ignore
        **settings,
    ):
        self.product = product
        self.project = project or Project()
        self.depends_on = depends_on or []
        self.settings = Settings(**settings)
        self.log = logging.getLogger(self.__class__.__name__)
        self.cmd = ShellCmd(self.log)

    def __str__(self):
        # return f"<'{self.__class__.__name__}' project='{self.project.name}' product='{self.product.name}'>"
        return f'<{self.__class__.__name__}>'

    __repr__ = __str__

    @property
    def prefix(self) -> Path:
        """compiled product destination root directory."""
        return self.project.build_lib / self.product.name.lower()

    @property
    def prefix_lib(self) -> Path:
        """compiled product destination lib directory."""
        return self.prefix / "lib"

    @property
    def prefix_include(self) -> Path:
        """compiled product destination include directory."""
        return self.prefix / "include"

    @property
    def prefix_bin(self) -> Path:
        """compiled product destination bin directory."""
        return self.prefix / "bin"

    @property
    def prefix_resources(self) -> Path:
        """compiled product Resources directory."""
        return self.prefix / "Resources"

    @property
    def download_path(self) -> Optional[Path]:
        """Returns path to downloaded product-version archive."""
        if self.product.name_archive:
            return self.project.build_downloads / self.product.name_archive

    @property
    def src_path(self) -> Path:
        """Return product source directory."""
        return self.project.build_src / self.product.name_version

    @property
    def url(self) -> Optional[Path]:
        """Returns url to download product as a pathlib.Path instance."""
        return self.product.url

    # -------------------------------------------------------------------------
    # Core functions

    @property
    def product_exists(self) -> bool:
        """checks if product is built"""
        return self.has_static_libs

    @property
    def has_static_libs(self) -> bool:  # sourcery skip: use-named-expression
        """check for presence of static libs"""
        libs = self.product.libs_static
        if libs:
            return all((self.prefix_lib / lib).exists() for lib in libs)
        return False

    def to_dict(self) -> dict:
        """dump configured vars to dict"""

        return {
            'prefix': str(self.prefix),
            'prefix_lib': str(self.prefix_lib),
            'prefix_include': str(self.prefix_include),
            'prefix_bin': str(self.prefix_bin),
            'download_path': str(self.download_path),
            'src_path': str(self.src_path),
            'url': str(self.url),
            'product_exists': self.product_exists,
            'has_static_libs': self.has_static_libs,
            'project': self.project.to_dict(),
            'product': self.product.to_dict(),
            'depends_on': [str(i) for i in self.depends_on],
        }

    def to_yaml(self):
        import yaml
        with open('dump.yml', 'w') as f:
            f.write(yaml.safe_dump(self.to_dict(), indent=4, default_flow_style=False))

    def to_json(self):
        import json
        with open('dump.json', 'w') as f:
            json.dump(self.to_dict(), f, 
                sort_keys=True, indent=4)

    def recursive_clean(self, path, pattern):
        """generic recursive clean/remove method."""
        self.cmd(f'find "{path}" | grep -E "({pattern})" | xargs rm -rf')

    def install_name_tool_id(self, new_id, target):
        """change dynamic shared library install names"""
        _cmd = f"install_name_tool -id '{new_id}' '{target}'"
        self.cmd(_cmd)

    def install_name_tool_change(self, src, dst, target):
        """change dependency reference"""
        _cmd = f"install_name_tool -change '{src}' '{dst}' '{target}'"
        self.cmd(_cmd)

    def install_name_tool_add_rpath(self, rpath, target):
        """change dependency reference"""
        _cmd = f"install_name_tool -add_rpath '{rpath}' '{target}'"
        self.cmd(_cmd)

    def xcodebuild(self, project: str, targets: List[str], *preprocessor_flags, **xcconfig_flags):
        """python wrapper around command-line xcodebuild"""
        x_flags = " ".join([f"{k}={repr(v)}" for k,v in xcconfig_flags.items()]) if xcconfig_flags else ''
        p_flags = "GCC_PREPROCESSOR_DEFINITIONS='$GCC_PREPROCESSOR_DEFINITIONS {flags}'".format(
            flags=" ".join([f"{k}=1" for k in preprocessor_flags])) if preprocessor_flags else ''
        for target in targets:
            self.cmd(
                f"xcodebuild -project 'targets/{project}/py-js.xcodeproj'"
                f" -target {repr(target)} {x_flags} {p_flags}"
            )

    # -------------------------------------------------------------------------
    # Core Methods

    def clean(self):
        """shallow cleanse build"""
        for builder in self.depends_on:
            builder.clean()

    def reset_prefix(self):
        """remove prefix or compilation destinations"""
        self.cmd.remove(self.prefix)

    def reset(self):
        """remove product src directory and compiled product directory."""
        #     for builder in self.depends_on:
        #         builder.reset()
        self.cmd.remove(self.src_path)
        self.cmd.remove(self.prefix)
        assert not (self.src_path.exists() or self.prefix.exists()), "reset not completed"

    def download(self):
        """download src using curl and tar.

        curl and tar are automatically available on mac platforms.
        """
        self.project.build_downloads.mkdir(parents=True, exist_ok=True)
        for dep in self.depends_on:
            dep.download()

        # download
        if self.download_path and not self.download_path.exists():
            self.project.build_downloads.mkdir(parents=True, exist_ok=True)
            self.log.info("downloading %s to %s", self.url, self.download_path)
            self.cmd(f"curl -L --fail '{self.url}' -o '{self.download_path}'")
            assert self.download_path.exists(), f"could not download: {self.download_path}"

        # unpack
        if not self.src_path.exists():
            self.project.build_src.mkdir(parents=True, exist_ok=True)
            self.log.info("unpacking %s", self.src_path)
            self.cmd(f"tar -xvf '{self.download_path}' --directory '{self.project.build_src}'")
            assert self.src_path.exists(), f"{self.src_path} not created"

    def build(self):
        """build product"""
        for builder in self.depends_on:
            builder.build()

    def pre_process(self):
        """pre-build operations"""

    def post_process(self):
        """post-build operations"""

    def install(self):
        """deploy to package"""


class Recipe:
    """A platform-specific container for multiple builder-centric projects."""

    # type: ignore
    def __init__(self, name: str, py_version: str = None, builders: List[Builder] = None,  # type: ignore
                 **settings):
        self.name = name
        self.py_version = py_version or constants.DEFAULT_PYTHON_VERSION
        self.builders = builders or []
        self.settings = Settings(**settings)

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    __repr__ = __str__

    def build(self):
        """build builders"""
        for builder in self.builders:
            builder.build()



# ------------------------------------------------------------------------------------
# DEPENDENCY BUILDERS

class Bzip2Builder(Builder):
    """Bzip2 static library builder"""

    def build(self):
        if not self.product_exists:
            self.cmd.chdir(self.src_path)
            self.cmd(
                f"""MACOSX_DEPLOYMENT_TARGET={self.project.mac_dep_target} \
                    make install PREFIX='{self.prefix}'""")
            self.cmd.chdir(self.project.root)


class OpensslBuilder(Builder):
    """OpenSSL static library builder"""

    def build(self):
        if not self.product_exists:
            self.cmd.chdir(self.src_path)
            os.environ['MACOSX_DEPLOYMENT_TARGET'] = self.project.mac_dep_target
            self.cmd(
                f"""MACOSX_DEPLOYMENT_TARGET={self.project.mac_dep_target} \
                    ./config no-shared no-tests \
                    --prefix='{self.prefix}'""")
            self.cmd(
                f"""MACOSX_DEPLOYMENT_TARGET='{self.project.mac_dep_target}' \
                    make install_sw""")
            self.cmd.chdir(self.project.root)


class XzBuilder(Builder):
    """Xz static library builder"""

    def build(self):
        if not self.product_exists:
            self.cmd.chdir(self.src_path)
            self.cmd(
                f"""MACOSX_DEPLOYMENT_TARGET={self.project.mac_dep_target} \
                ./configure --disable-shared --enable-static --prefix='{self.prefix}'"""
            )
            self.cmd(
                f"""MACOSX_DEPLOYMENT_TARGET='{self.project.mac_dep_target}' \
                    make && make install""")
            self.cmd.chdir(self.project.root)

# ------------------------------------------------------------------------------------
# PYTHON BUILDERS (ABSTRACT)

class PythonBuilder(Builder):
    """Generic Python from src builder."""

    @property
    def static_lib(self):
        """Name of static library: libpython.3.9.a"""
        return f"lib{self.product.name.lower()}{self.product.ver}.a"  # pylint: disable=E1101

    @property
    def python_lib(self):
        """python/lib/product.major.minor: python/lib/python3.9"""
        return self.prefix_lib / self.product.name_ver

    @property
    def site_packages(self):
        """path to 'site-packages'"""
        return self.python_lib / "site-packages"

    @property
    def lib_dynload(self):
        """path to 'lib-dynload'"""
        return self.python_lib / "lib-dynload"

    # ------------------------------------------------------------------------
    # src-level operations

    def pre_process(self):
        """pre-build operations"""

    def post_process(self):
        """post-build operations"""
        self.clean()
        self.ziplib()
        # self.fix()
        # self.sign()

    def install(self):
        """install and build compilation product"""
        self.reset()
        self.pre_process()
        self.build()
        self.post_process()

    # ------------------------------------------------------------------------
    # post-processing operations

    def clean_python_pyc(self, path):
        """remove python .pyc files."""
        self.recursive_clean(path, r"__pycache__|\.pyc|\.pyo$")

    def clean_python_tests(self, path):
        """remove python tests files."""
        self.recursive_clean(path, "tests|test")

    def rm_libs(self, names):
        """remove all named python dylib libraries"""
        for name in names:
            self.cmd.remove(self.python_lib / name)

    def rm_exts(self, names):
        """remove all named extensions"""
        for name in names:
            self.cmd.remove(
                self.python_lib
                / "lib-dynload"
                / f"{name}.cpython-{self.product.ver_nodot}-darwin.so"
            )

    def rm_bins(self, names):
        """remove all named binary executables"""
        for name in names:
            self.cmd.remove(self.prefix_bin / name)

    def clean_python_site_packages(self, basedir=None):
        """remove python site-packages"""
        if not basedir:
            basedir = self.python_lib
        self.cmd.remove(basedir / "site-packages")

    def remove_packages(self):
        """remove list of non-critical packages"""

        self.rm_libs(
            [
                self.project.python.config_ver_platform,
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
        )

    def remove_extensions(self):
        """remove extensions"""
        self.rm_exts(
            [
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
        )

    def remove_binaries(self):
        """remove list of non-critical executables"""
        ver = self.product.ver
        self.rm_bins(
            [
                f"2to3-{ver}",
                f"idle{ver}",
                f"easy_install-{ver}",
                f"pip{ver}",
                f"pyvenv-{ver}",
                f"pydoc{ver}",
                # f'python{ver}{self.suffix}',
                # f'python{ver}-config',
            ]
        )

    def write_python_getpip(self):
        """optionally provide latets pip to binary"""
        with open(f"{self.prefix}/bin/get_pip.sh") as txtfile:
            txtfile.write(
                dedent(
                    f"""
                curl {URL_GETPIP} -s -o get-pip.py
                ./bin/{self.product.name_ver} get-pip.py
                rm get-pip.py
                """
                )
            )

    def clean(self):
        """clean everything."""
        self.clean_python_pyc(self.prefix)
        self.clean_python_tests(self.python_lib)
        self.clean_python_site_packages()

        for i in (self.python_lib / "distutils" / "command").glob("*.exe"):
            self.cmd.remove(i)

        self.cmd.remove(self.prefix_lib / "pkgconfig")
        self.cmd.remove(self.prefix / "share")

        self.remove_packages()
        self.remove_extensions()
        self.remove_binaries()

    def ziplib(self):
        """zip python package in site-packages in .zip archive"""
        temp_lib_dynload = self.prefix_lib / "lib-dynload"
        temp_os_py = self.prefix_lib / "os.py"

        self.cmd.remove(self.site_packages)
        self.lib_dynload.rename(temp_lib_dynload)
        self.cmd.copy(self.python_lib / "os.py", temp_os_py)

        zip_path = self.prefix_lib / f"python{self.product.ver_nodot}"
        shutil.make_archive(str(zip_path), "zip", str(self.python_lib))

        self.cmd.remove(self.python_lib)
        self.python_lib.mkdir()
        temp_lib_dynload.rename(self.lib_dynload)
        temp_os_py.rename(self.python_lib / "os.py")
        self.site_packages.mkdir()

    def fix_python_dylib_for_pkg(self):
        """redirect ref of dylib to loader in a package deployment."""
        self.cmd.chdir(self.prefix_lib)
        self.cmd.chmod(self.product.dylib)
        self.install_name_tool_id(
            f"@loader_path/../../../../support/{self.product.name}/lib/{self.product.dylib}",
            self.product.dylib,
        )
        self.cmd.chdir(self.project.root)

    def fix_python_dylib_for_ext(self):
        """redirect ref of dylib to loader in a self-contained external deployment."""
        self.cmd.chdir(self.prefix_lib)
        self.cmd.chmod(self.product.dylib)
        self.install_name_tool_id(f"@loader_path/{self.product.dylib}", self.product.dylib)
        self.cmd.chdir(self.project.root)


class PythonSrcBuilder(PythonBuilder):
    """Generic Python from src builder."""

    setup_local: str = ""
    patch: str = ""
    # ------------------------------------------------------------------------
    # python properties

    # ------------------------------------------------------------------------
    # src-level operations
    def configure(self):
        """configures overrides to defaults from commandline"""
        if self.settings.py_version:
            self.product.version = self.settings.py_version

    def install(self):
        """install and build compilation product"""
        self.configure()
        self.reset()
        self.download()
        self.pre_process()
        self.build()
        self.post_process()

    def pre_process(self):
        """pre-build operations"""
        self.cmd.chdir(self.src_path)
        self.write_setup_local()
        self.apply_patch(patch="configure.patch", to_file="configure")
        self.cmd.chdir(self.project.root)

    def post_process(self):
        """post-build operations"""
        self.clean()
        self.ziplib()
        # self.fix()
        # self.sign()

    def write_setup_local(self, setup_local=None):
        """Write to Setup.local file for cusom compilations of python builtins."""
        if not any([setup_local, self.setup_local]):
            return
        if not setup_local:
            setup_local = self.setup_local
        self.cmd.copy(
            self.project.patch / self.product.ver / setup_local,
            self.src_path / "Modules" / "Setup.local",
        )

    def apply_patch(self, patch=None, to_file=None):
        """Apply a standard patch from the patch directory (prefixed by major.minor ver)

        if param `to_file` is given then the patch is applied directly the file (diff method)
        otherwise: the patch is applied to the directory itself (git method)
        """
        if not any([patch, self.patch]):
            return
        if not patch:
            patch = self.patch
        if to_file:
            self.cmd(f"patch {to_file} < {self.project.patch}/{self.product.ver}/{patch}")
        else:
            self.cmd(f"patch -p1 < {self.project.patch}/{self.product.ver}/{patch}")


# ------------------------------------------------------------------------------------
# PYTHON BUILDERS (BASE)

class FrameworkPythonBuilder(PythonSrcBuilder):
    """builds python in a macos framework format."""

    setup_local = "setup-shared.local"

    @property
    def prefix(self) -> Path:
        return self.project.build_lib / "Python.framework" / "Versions" / self.product.ver

    def reset(self):
        self.cmd.remove(self.src_path)
        self.cmd.remove(self.project.build_lib / "Python.framework")

    def build(self):
        for dep in self.depends_on:
            dep.build()

        self.cmd.chdir(self.src_path)
        self.cmd(
            f"""\
        ./configure MACOSX_DEPLOYMENT_TARGET='{self.project.mac_dep_target}' \
            --enable-framework='{self.project.build_lib}' \
            --with-openssl={self.project.build_lib / 'openssl'} \
            --without-doc-strings \
            --enable-ipv6 \
            --without-ensurepip \
            --with-lto \
            --enable-optimizations
        """
        )
        self.cmd("make altinstall")
        self.cmd.chdir(self.project.root)

    # PYTHONBUG: Python.framework/Versions/3.9/Resources/Python.app
    #            is linked to executable in the frameowork
    # def clean(self):
    #     """clean everything."""
    #     super().clean() # call superclass clean method
    #     self.cmd.remove(self.prefix_resources / "Python.app")


class SharedPythonBuilder(PythonSrcBuilder):
    """builds python in a shared format."""

    setup_local = "setup-shared.local"

    @property
    def prefix(self) -> Path:
        return self.project.build_lib / self.product.build_dir

    def build(self):
        for dep in self.depends_on:
            dep.build()

        self.cmd.chdir(self.src_path)
        self.cmd(
            f"""\
        ./configure MACOSX_DEPLOYMENT_TARGET={self.project.mac_dep_target} \
            --prefix='{self.prefix}' \
            --enable-shared \
            --with-openssl={self.project.build_lib / 'openssl'} \
            --without-doc-strings \
            --enable-ipv6 \
            --without-ensurepip \
            --with-lto \
            --enable-optimizations \
            ac_cv_lib_intl_textdomain=no
        """
        )
        self.cmd("make altinstall")
        self.cmd.chdir(self.project.root)


class StaticPythonBuilder(PythonSrcBuilder):
    """builds python in a static format."""

    setup_local = "setup-static-min3.local"

    @property
    def prefix(self) -> Path:
        return self.project.build_lib / self.product.build_dir

    def build(self):
        # for dep in self.depends_on:
        #     dep.build()

        self.cmd.chdir(self.src_path)
        self.cmd(
            f"""\
        ./configure MACOSX_DEPLOYMENT_TARGET={self.project.mac_dep_target} \
            --prefix='{self.prefix}' \
            --with-openssl={self.project.build_lib / 'openssl'} \
            --without-doc-strings \
            --enable-ipv6 \
            --without-ensurepip \
            --with-lto \
            --enable-optimizations
        """
        )
        self.cmd("make altinstall")
        self.cmd.chdir(self.project.root)

    def remove_extensions(self):
        """remove extensions: not implemented"""


# ------------------------------------------------------------------------------------
# PYTHON BUILDERS (SPECIALIZED)

class SharedPythonForExtBuilder(SharedPythonBuilder):
    """builds python in a shared format for self-contained externals."""

    setup_local = "setup-shared.local"

    def fix_python_dylib_for_ext_resources(self):
        """change dylib ref to point to loader in external build format"""
        self.cmd.chdir(self.prefix / 'lib')
        dylib_path = self.prefix / 'lib' / self.product.dylib
        assert dylib_path.exists()
        self.cmd.chmod(self.product.dylib)
        self.install_name_tool_id(
            f"@loader_path/../Resources/lib/{self.product.dylib}",
            self.product.dylib,
        )
        self.cmd.chdir(self.project.root)

    def post_process(self):
        """post-build operations"""
        self.clean()
        self.ziplib()
        self.fix_python_dylib_for_ext_resources()


class SharedPythonForPkgBuilder(SharedPythonBuilder):
    """builds python in a shared format for self-contained externals."""

    setup_local = "setup-shared.local"

    def pre_process(self):
        """pre-build operations"""
        self.cmd.chdir(self.src_path)
        self.write_setup_local()
        self.apply_patch(patch="configure.patch", to_file="configure")
        self.cmd.chdir(self.project.root)

    def remove_packages(self):
        """remove list of non-critical packages"""
        self.rm_libs(
            [
                self.project.python.config_ver_platform,
                "idlelib",
                "lib2to3",
                "tkinter",
                "turtledemo",
                "turtle.py",
                "ctypes",
                "curses",
                # "ensurepip",
                "venv",
            ]
        )

    def fix_python_dylib_for_pkg(self):
        """change dylib ref to point to loader in package build format"""
        self.cmd.chdir(self.prefix / 'lib')
        dylib_path = self.prefix / 'lib' / self.product.dylib
        assert dylib_path.exists(), f"{dylib_path} does not exist"
        self.cmd.chmod(self.product.dylib)
        # both of these are equivalent (and both don't work!)
        self.install_name_tool_id(
            f"@loader_path/../../../../support/{self.product.name_ver}/lib/{self.product.dylib}",
            self.product.dylib,
        )
        self.cmd.chdir(self.project.root)

    def fix_python_exe_for_pkg(self):  # sourcery skip: use-named-expression
        """redirect ref of pythonX to libpythonX.Y.dylib"""
        self.cmd.chdir(self.prefix_bin)
        exe = self.product.name_ver
        d = DependencyManager(exe)
        dirs_to_change = d.analyze_executable()
        if dirs_to_change:
            dir_to_change = dirs_to_change[0]
            self.install_name_tool_change(
                dir_to_change,
                f"@executable_path/../lib/{self.product.dylib}", 
                exe
            )
        self.cmd.chdir(self.project.root)

    def post_process(self):
        """post-build operations"""
        self.clean()
        self.ziplib()
        self.fix_python_exe_for_pkg()
        self.fix_python_dylib_for_pkg()


class FrameworkPythonForExtBuilder(FrameworkPythonBuilder):
    """builds python in a framework format for self-contained externals."""

    setup_local = "setup-shared.local"

    def fix_python_dylib_for_ext_resources(self):
        """change dylib ref to point to loader in external build format"""
        self.cmd.chdir(self.prefix)
        dylib_path = self.prefix / 'Python'
        assert dylib_path.exists()
        #self.cmd.chmod(dylib_path)
        self.install_name_tool_id(
            f"@loader_path/../Resources/Python.framework/Versions/{self.product.ver}/Python",
            dylib_path
            # self.project.build_lib / self.project.python.ldlibrary
        )        
        self.cmd.chdir(self.project.root)

    def fix_python_exec_for_framework(self):    # sourcery skip: use-named-expression
        """change ref on executable to point to relative dylib"""
        self.cmd.chdir(self.prefix_bin)
        executable = self.product.name_ver
        result = subprocess.check_output(["otool", "-L", executable])
        entries = [line.decode("utf-8").strip() for line in result.splitlines()]
        for entry in entries:
            match = re.match(r"\s*(\S+)\s*\(compatibility version .+\)$", entry)
            if match:
                path = match.group(1)
                # homebrew files are installed in /usr/local/Cellar
                if any(path.startswith(p) for p in PATTERNS_TO_FIX):
                    self.install_name_tool_change(path, "@executable_path/../Python", executable)
        self.cmd.chdir(self.project.root)

    def fix_python_exec_for_framework2(self):    # sourcery skip: use-named-expression
        """change ref on executable to point to relative dylib"""
        parent_dir = self.prefix_resources / "Python.app" / "Contents" / "MacOS"
        self.cmd.chdir(parent_dir)
        executable = parent_dir / "Python"
        result = subprocess.check_output(["otool", "-L", executable])
        entries = [line.decode("utf-8").strip() for line in result.splitlines()]
        for entry in entries:
            match = re.match(r"\s*(\S+)\s*\(compatibility version .+\)$", entry)
            if match:
                path = match.group(1)
                # homebrew files are installed in /usr/local/Cellar
                if any(path.startswith(p) for p in PATTERNS_TO_FIX):
                    self.install_name_tool_change(
                        path, "@executable_path/../../../../Python", executable
                    )

        self.cmd.chdir(self.project.root)

    def post_process(self):
        """post-build operations"""
        self.clean()
        self.ziplib()
        self.fix_python_dylib_for_ext_resources()
        self.fix_python_exec_for_framework()
        self.fix_python_exec_for_framework2()


class FrameworkPythonForPkgBuilder(FrameworkPythonBuilder):
    """builds python in a framework format for relocatable max packages."""

    setup_local = "setup-shared.local"

    def pre_process(self):
        """pre-build operations"""
        self.cmd.chdir(self.src_path)
        self.write_setup_local()
        self.apply_patch(patch="configure.patch", to_file="configure")
        self.cmd.chdir(self.project.root)

    def remove_packages(self):
        """remove list of non-critical packages"""
        self.rm_libs(
            [
                self.project.python.config_ver_platform,
                "idlelib",
                "lib2to3",
                "tkinter",
                "turtledemo",
                "turtle.py",
                "ctypes",
                "curses",
                # "ensurepip",
                "venv",
            ]
        )

    def fix_python_dylib_for_pkg(self):
        """change dylib ref to point to loader in package build format"""
        self.cmd.chdir(self.prefix)
        dylib_path = self.prefix / 'Python'
        assert dylib_path.exists()
        self.cmd.chmod(dylib_path)
        # both of these are equivalent (and both don't work!)
        self.install_name_tool_id(
            "@loader_path/../../../../support" / self.project.python.ldlibrary,
            dylib_path,
        )

        self.cmd.chdir(self.project.root)

    def fix_python_exe_for_pkg(self):    # sourcery skip: use-named-expression
        """redirect ref of pythonX to libpythonX.Y.dylib"""
        self.cmd.chdir(self.prefix_bin)
        exe = self.product.name_ver
        d = DependencyManager(exe)
        dirs_to_change = d.analyze_executable()
        if dirs_to_change:
            dir_to_change = dirs_to_change[0]
            self.install_name_tool_change(dir_to_change, "@executable_path/../Python", exe)
        self.cmd.chdir(self.project.root)

    def fix_python_exec_for_pkg2(self):    # sourcery skip: use-named-expression
        """change ref on executable to point to relative dylib"""
        parent_dir = self.prefix_resources / "Python.app" / "Contents" / "MacOS"
        self.cmd.chdir(parent_dir)
        executable = parent_dir / "Python"
        result = subprocess.check_output(["otool", "-L", executable])
        entries = [line.decode("utf-8").strip() for line in result.splitlines()]
        for entry in entries:
            match = re.match(r"\s*(\S+)\s*\(compatibility version .+\)$", entry)
            if match:
                path = match.group(1)
                # homebrew files are installed in /usr/local/Cellar
                if any(path.startswith(p) for p in PATTERNS_TO_FIX):
                    self.install_name_tool_change(
                        path, "@executable_path/../../../../Python", executable
                    )

        self.cmd.chdir(self.project.root)

    def post_process(self):
        """post-build operations"""
        self.clean()
        self.ziplib()
        self.fix_python_dylib_for_pkg()
        self.fix_python_exe_for_pkg()
        self.fix_python_exec_for_pkg2()

# ------------------------------------------------------------------------------------
# PYJS EXTERNAL BUILDERS (ABSTRACT)

class PyJsBuilder(PythonBuilder):
    """pyjs concrete base class"""

    @property
    def prefix(self):
        return self.project.support / self.project.python.name

    def remove_externals(self):
        """remove py and pyjs externals from the py-js/externals directory"""
        self.cmd.remove(self.project.py_external)
        self.cmd.remove(self.project.pyjs_external)

    def install(self):
        for builder in self.depends_on:
            builder.settings.update(self.settings)
            builder.install()


# ------------------------------------------------------------------------------------
# PYJS EXTERNAL BUILDERS (SPECIALIZED)

class HomebrewBuilder(PyJsBuilder):
    """homebrew python builder"""

    suffix = ""
    setup_local: str = ""
    patch: str = ""

    def cp_pkgs(self, pkgs):
        """copy package dirs from homebrew python lib to target python lib"""
        for pkg in pkgs:
            self.cmd.copy(self.project.python.pkgs / pkg, self.python_lib / pkg)

    def rm_libs(self, names):
        """remove all named python dylib libraries"""
        for name in names:
            self.cmd.remove(self.python_lib / name)

    # def remove_extensions(self):
    #     """remove extensions: not implemented"""

    def clean_python(self):
        """clean everything."""
        self.clean_python_pyc(self.prefix)
        self.clean_python_tests(self.python_lib)
        for i in (self.python_lib / "distutils" / "command").glob("*.exe"):
            self.cmd.remove(i)

        self.remove_packages()
        self.remove_extensions()

    def fix_python_exec(self):  # sourcery skip: use-named-expression
        """change ref on executable to point to relative dylib"""
        self.cmd.chdir(self.prefix_bin)
        executable = self.product.name_ver
        result = subprocess.check_output(["otool", "-L", executable])
        entries = [line.decode("utf-8").strip() for line in result.splitlines()]
        for entry in entries:
            match = re.match(r"\s*(\S+)\s*\(compatibility version .+\)$", entry)
            if match:
                path = match.group(1)
                # homebrew files are installed in /usr/local/Cellar
                if path.startswith("/usr/local/Cellar/python"):
                    self.install_name_tool_change(
                        path,
                        f"@executable_path/../{self.product.dylib}",
                        executable,
                    )
        self.cmd.chdir(self.project.root)

    def fix_python_dylib_for_pkg(self):
        """change dylib ref to point to loader in package build format"""
        self.cmd.chdir(self.prefix)
        self.cmd.chmod(self.product.dylib)
        self.install_name_tool_id(
            f"@loader_path/../../../../support/{self.product.name_ver}/{self.product.dylib}",
            self.product.dylib,
        )
        self.cmd.chdir(self.project.root)

    def fix_python_dylib_for_ext_resources(self):
        """change dylib ref to point to loader in external build format"""
        self.cmd.chdir(self.prefix)
        self.cmd.chmod(self.product.dylib)
        self.install_name_tool_id(
            f"@loader_path/../Resources/{self.product.name_ver}/{self.product.dylib}",
            self.product.dylib,
        )
        self.cmd.chdir(self.project.root)

    def cp_python_to_ext_resources(self, arg):
        """copy processed python libs to bundle resources directory"""
        self.cmd(f"mkdir -p '{arg}/Contents/Resources/{self.product.name_ver}'")
        self.cmd(
            f"cp -rf {self.prefix}/* '{arg}/Contents/Resources/{self.product.name_ver}'"
        )

    def copy_python(self):
        """copy python from homebrew to destination"""
        self.python_lib.mkdir(parents=True, exist_ok=True)
        self.prefix_bin.mkdir(parents=True, exist_ok=True)
        self.cmd.copy(
            self.project.python.prefix / "Python", self.prefix / self.product.dylib
        )
        self.cmd(f"cp -rf {self.project.python.pkgs}/*.py '{self.python_lib}'")
        self.cp_pkgs(
            [
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
        )
        self.cmd.copy(self.project.python.prefix / "include", self.prefix_include)
        self.cmd.remove(self.prefix_lib / self.product.dylib)
        self.cmd.remove(self.prefix_lib / "pkgconfig")
        self.cmd.copy(
            self.project.python.prefix / "Resources/Python.app/Contents/MacOS/Python",
            self.prefix_bin / self.product.name_ver,
        )
        self.clean_python()
        self.ziplib()

    def install(self):
        """install via symlink"""
        if not self.project.package.exists():
            self.log.info("package py-js symlink does not exist -- creating at %s", self.project.package)
            self.project.package.symlink_to(self.project.pyjs)
        else:
            self.log.info("package py-js symlink exists -- not creating")

    def install_homebrew_sys(self):
        """build externals use local homebrew python (non-portable)"""
        # self.reset_prefix()
        self.remove_externals()
        self.xcodebuild("homebrew-sys", targets=["py", "pyjs"])
        self.install()

    def install_homebrew_pkg(self):
        """build externals into package use local homebrew python (portable)"""
        self.reset_prefix()
        self.copy_python()
        self.fix_python_dylib_for_pkg()
        self.fix_python_exec()
        self.xcodebuild("homebrew-pkg", targets=["py", "pyjs"])
        self.install()

    def install_homebrew_ext(self):
        """build external into self-contained external using local homebrew python (portable)"""
        self.reset_prefix()
        self.copy_python()
        self.fix_python_exec()
        self.fix_python_dylib_for_ext_resources()
        self.cp_python_to_ext_resources(self.project.py_external)
        self.cp_python_to_ext_resources(self.project.pyjs_external)
        self.xcodebuild("homebrew-ext", targets=["py", "pyjs"])
        self.reset_prefix()
        self.install()


class LocalSystemBuilder(PyJsBuilder):
    """Builds externals from local python (non-portable)"""

    def build(self):
        """builds externals from local system python"""
        targets = ["py", "pyjs"]
        flags = dict(
            PREFIX = str(self.project.python.prefix),
            VERSION = str(self.project.python.version_short),
            ABIFLAGS = str(self.project.python.abiflags),
            LIBS = str(self.project.python.libs),
        )

        # if self.settings.deploy:
        #     flags['DSTROOT'] = str(self.project.build_externals)

        self.xcodebuild("local-sys", targets=targets, **flags)

        # if self.settings.deploy:
        #     for ext in [f"{t}.mxo" for t in targets]:
        #         src = self.project.build_externals / ext
        #         dst = self.project.externals / ext
        #         self.cmd.copy(src, dst)

class StaticExtBuilder(PyJsBuilder):
    """pyjs externals from minimal statically built python"""

    @property
    def product_exists(self):
        static_lib = self.project.build_lib / 'python-static' / 'lib' / self.project.python.staticlib  # type: ignore
        if not static_lib.exists():
            self.log.warning("static python is not built: %s", static_lib)
        return static_lib.exists()

    def build(self):
        """builds externals from statically built python"""

        if self.product_exists:
            flags = dict(
                VERSION = str(self.project.python.version_short),
                ABIFLAGS = str(self.project.python.abiflags),
            )
            self.xcodebuild("static-ext", targets=["py", "pyjs"], **flags)


class SharedExtBuilder(PyJsBuilder):
    """pyjs externals from minimal statically built python"""

    @property
    def product_exists(self):
        shared_lib = self.project.build_lib / 'python-shared' / 'lib' / self.project.python.dylib
        if not shared_lib.exists():
            self.log.warning("shared python is not built: %s", shared_lib)
        return shared_lib.exists()

    def build(self):
        """builds externals from shared python"""

        if self.product_exists:
            flags = dict(
                VERSION = str(self.project.python.version_short),
                ABIFLAGS = str(self.project.python.abiflags),
            )
            self.xcodebuild("shared-ext", targets=["py", "pyjs"], **flags)


class SharedPkgBuilder(PyJsBuilder):
    """pyjs externals in a package from minimal statically built python"""

    @property
    def product_exists(self):
        shared_lib = self.project.build_lib / 'python-shared' / 'lib' / self.project.python.dylib
        if not shared_lib.exists():
            self.log.warning("shared python is not built: %s", shared_lib)
        return shared_lib.exists()

    def build(self):
        """builds externals from shared python"""
        src = self.project.build_lib / 'python-shared'
        dst = f"{self.project.support}/{self.product.name_ver}"
        self.cmd(f"rm -rf '{dst}'") # try to remove if it exists
        self.cmd(f"cp -af '{src}' '{dst}'")

        if self.product_exists:
            flags = dict(
                VERSION = str(self.project.python.version_short),
                ABIFLAGS = str(self.project.python.abiflags),
            )
            self.xcodebuild("shared-pkg", targets=["py", "pyjs"], **flags)


class FrameworkExtBuilder(PyJsBuilder):
    """pyjs externals from minimal framework built python"""

    @property
    def product_exists(self):
        shared_lib = self.project.build_lib / "Python.framework" / "Versions" / self.product.ver / 'Python'
        if not shared_lib.exists():
            self.log.warning("framework python is not built: %s", shared_lib)
        return shared_lib.exists()

    def build(self):
        """builds externals from shared python"""
        if self.product_exists:
            flags = dict(
                VERSION = str(self.project.python.version_short),
                ABIFLAGS = str(self.project.python.abiflags),
            )
            self.xcodebuild("framework-ext", targets=["py", "pyjs"], **flags)
                # preprocessor_flags=["PY_FWK_EXT"])


class FrameworkPkgBuilder(PyJsBuilder):
    """pyjs externals in a package from minimal framework built python"""

    @property
    def product_exists(self):
        shared_lib = self.project.build_lib / "Python.framework" / "Versions" / self.product.ver / 'Python'
        if not shared_lib.exists():
            self.log.warning("framework python is not built: %s", shared_lib)
        return shared_lib.exists()

    def build(self):
        """builds externals from framework python"""
        src = self.project.build_lib / "Python.framework"
        dst = self.project.support / "Python.framework"
        self.cmd(f"rm -rf '{dst}'") # try to remove if it exists
        self.cmd(f"cp -af '{src}' '{dst}'")
        if self.product_exists:
            flags = dict(
                VERSION = str(self.project.python.version_short),
                ABIFLAGS = str(self.project.python.abiflags),
            )
            self.xcodebuild("framework-pkg", targets=["py", "pyjs"], **flags)


class RelocatablePkgBuilder(PyJsBuilder):
    """pyjs externals in a framework package using Greg Neagle's Relocatable Python

    Note: this is the only PyJsBuilder subclass which applies pre_processing and cleaning.
    That's because, it is assumed that the Python.framework is already downloaded to
    self.project.support via a previous step.

    Currently this is via Greg Neagle's code in the ext folder.
    """

    @property
    def prefix(self) -> Path:
        return self.project.support / "Python.framework" / "Versions" / self.product.ver

    def pre_process(self):
        """pre-build operations"""
        self.clean()
        self.ziplib()

    def clean(self):
        """clean everything."""
        self.clean_python_pyc(self.prefix)
        self.clean_python_tests(self.python_lib)
        # self.clean_python_site_packages(self.python_lib)

        for i in (self.python_lib / "distutils" / "command").glob("*.exe"):
            self.cmd.remove(i)

        self.cmd.remove(self.prefix_lib / "pkgconfig")
        self.cmd.remove(self.prefix / "share")

        self.remove_packages()
        self.remove_extensions()
        self.remove_binaries()
        self.remove_tkinter()

    def rm_globbed(self, names):
        """remove all named glob patterns of libraries and files"""
        for name in names:
            for f in self.prefix_lib.glob(name):
                self.cmd.remove(f)

    def remove_tkinter(self):
        """remove tkinter-related stuff"""
        targets = [
            "Tk.*",
            "itcl*",
            "libformw.*",
            "libmenuw.*",
            "libpanelw.*",
            "libncurse*",
            "libtcl*",   
            "libtclstub*",
            "sqlite3*",
            "libtk*",
            "tcl*",
            "tdbc*",
            "thread*",
            "tk*",
        ]
        self.rm_globbed(targets)

    def ziplib(self):
        """zip python package in site-packages in .zip archive"""
        temp_lib_dynload = self.prefix_lib / "lib-dynload"
        temp_os_py = self.prefix_lib / "os.py"

        # self.cmd.remove(self.site_packages)
        self.cmd.move(self.site_packages, '/tmp/site-packages')
        self.lib_dynload.rename(temp_lib_dynload)
        self.cmd.copy(self.python_lib / "os.py", temp_os_py)

        zip_path = self.prefix_lib / f"python{self.product.ver_nodot}"
        shutil.make_archive(str(zip_path), "zip", str(self.python_lib))

        self.cmd.remove(self.python_lib)
        self.python_lib.mkdir()
        temp_lib_dynload.rename(self.lib_dynload)
        temp_os_py.rename(self.python_lib / "os.py")
        # self.site_packages.mkdir()
        self.cmd.move('/tmp/site-packages', self.site_packages)

    @property
    def product_exists(self):
        py_framework = self.project.support / "Python.framework"
        if not py_framework.exists():
            self.log.warning("framework python is not built: %s", py_framework)
        return py_framework.exists()

    def build(self):
        """builds externals from framework python"""
        self.pre_process()
        if self.product_exists:
            flags = dict(
                VERSION = str(self.project.python.version_short),
                ABIFLAGS = str(self.project.python.abiflags),
            )
            self.xcodebuild("relocatable-pkg", targets=["py", "pyjs"], **flags)

