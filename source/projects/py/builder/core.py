"""builder: a builder of py-js max externals.

A pure python builder without any dependencies except the standard library.

Python
Project
ShellCmd
Settings
Product
Recipe
Builder
    ConfiguredBuilder
        Bzip2Builder
        OpensslBuilder
        XzBuilder
    PythonBuilder
        PythonCmakeBuilder
        PythonSrcBuilder
            FrameworkPythonBuilder
                FrameworkPythonForExtBuilder
                FrameworkPythonForPkgBuilder
            SharedPythonBuilder
                SharedPythonForExtBuilder
                SharedPythonForPkgBuilder
            StaticPythonBuilder
            RelocatablePythonBuilder
        PyJsBuilder
            LocalSystemBuilder
            HomebrewExtBuilder
            HomebrewPkgBuilder            
            StaticExtBuilder
            StaticPkgBuilder (not implemented, useless)
            SharedExtBuilder
            SharedPkgBuilder (not used)
            FrameworkExtBuilder
            FrameworkPkgBuilder
            RelocatablePkgBuilder
        

install:
    configure -> reset -> download -> pre_process -> build -> post_process

"""

import json
import logging
import os
import shutil
import subprocess
import tempfile
from pathlib import Path
from textwrap import dedent
from types import SimpleNamespace
from typing import Dict, List, Optional

from .config import (CURRENT_PYTHON_VERSION, DEFAULT_CONFIGURE_OPTIONS,
                     LOG_FORMAT, LOG_LEVEL, URL_GETPIP,
                     URL_PYTHON_CMAKE_BUILDSYSTEM, PYJS_CMAKE_DEFAULT_OPTIONS,
                     Project)
from .depend import DependencyManager
from .ext.relocatable_python import download_relocatable_to
from .shell import ShellCmd

logging.basicConfig(format=LOG_FORMAT, level=LOG_LEVEL)


# ----------------------------------------------------------------------------
# Utility Functions

def quote(obj):
    """convert object to string and ensure it's quoted"""
    return repr(str(obj))

# ----------------------------------------------------------------------------
# Utility Classes


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
        version: str = None,    # type: ignore
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
    def ver_minor(self) -> str:
        """provides version patch element: 3.9.1 -> 9"""
        return ".".join(self.version.split(".")[1])

    @property
    def ver_patch(self) -> str:
        """provides version patch element: 3.9.1 -> 1"""
        return ".".join(self.version.split(".")[2])

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
        return self.url.name if self.url else f"{self.name_version}.tgz"

    @property
    def dylib(self) -> str:
        """name of dynamic library in macos case."""
        return f"lib{self.name.lower()}{self.ver}{self.abiflags}.dylib"

    @property
    def staticlib(self) -> str:
        """name of static library in macos case."""
        return f"lib{self.name.lower()}{self.ver}.a"
        # return f"lib{self.name.lower()}{self.ver}{self.abiflags}.a" # abiflags here?

    @property
    def abiflags(self) -> str:
        """a rare and irritating suffix appended to python versions: 3.7m
        
        Currently only python 3.7 has this.
        """
        return "m" if self.ver in ["3.7"] else ""

    @property
    def config_ver_platform(self) -> str:
        """config-{version_short}{abiflags}-darwin: config-3.11-darwin"""
        return f"config-{self.ver}{self.abiflags}-darwin"

    @property
    def DEFAULT_PKGS_TO_RM(self) -> set:
        """default packages to remove"""
        return set([
            self.config_ver_platform,
            "aifc.py",
            "cgi.py",
            "cgitb.py",
            "chunk.py",
            "crypt.py",
            "ctypes",
            "curses",
            "distutils",
            "ensurepip",
            "idlelib",
            "imghdr.py",
            "lib2to3",
            "mailcap.py",
            "nntplib.py",
            "pipes.py",
            "sndhdr.py",
            "sunau.py",
            "telnetlib.py",
            "tkinter",
            "turtle.py",
            "turtledemo",
            "uu.py",
            "venv",
            "xdrlib.py",
            # "msilib",
            # "ossaudiodev",
            # "spwd",
        ])

    @property
    def DEFAULT_EXTS_TO_RM(self) -> set:
        """default extensions to remove"""
        return set([
            "_codecs_cn",
            "_codecs_hk",
            "_codecs_iso2022",
            "_codecs_jp",
            "_codecs_kr",
            "_codecs_tw",
            "_ctypes",
            "_curses",
            "_curses_panel",
            # "_multibytecodec",
            "_tkinter",
            "audioop",
            "nis",
        ])

    @property
    def DEFAULT_BINS_TO_RM(self) -> set:
        """default binaries to remove"""
        ver = self.ver
        return set([
            f"2to3-{ver}",
            f"idle{ver}",
            f"easy_install-{ver}",
            f"pip{ver}",
            f"pyvenv-{ver}",
            f"pydoc{ver}",
        ])

    @property
    def url(self):
        """Returns url to download product src as a pathlib.Path instance."""
        assert self.url_template, "Product.url_template required"
        return Path(self.url_template.format(name=self.name, version=self.version))
        # if self.url_template:
        #     return Path(self.url_template.format(name=self.name, version=self.version))
        # raise KeyError("url_template not providing in settings")

    def to_dict(self) -> Dict:
        """convert properties to dict"""
        return {
            "name": self.name,
            "version": self.version,
            "build_dir": str(self.build_dir),
            "libs_static": self.libs_static,
            "url_template": self.url_template,
            "settings": str(self.settings),
            "ver": self.ver,
            "ver_nodot": self.ver_nodot,
            "name_version": self.name_version,
            "name_ver": self.name_ver,
            "name_archive": self.name_archive,
            "dylib": self.dylib,
            "url": str(self.url),
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
        return f"<{self.__class__.__name__}>"

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
        assert self.product.name_archive, "Build.product.name_archive required"
        return self.project.build_downloads / self.product.name_archive

    @property
    def src_path(self) -> Path:
        """Return product source directory."""
        return self.project.build_src / self.product.name_version

    @property
    def url(self) -> Optional[Path]:
        """Returns url to download product as a pathlib.Path instance."""
        return self.product.url

    @property
    def default_env_vars(self):
        """Returns a dict of default environ settings"""
        return {"MACOSX_DEPLOYMENT_TARGET": self.project.mac_dep_target}

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
            "prefix": str(self.prefix),
            "prefix_lib": str(self.prefix_lib),
            "prefix_include": str(self.prefix_include),
            "prefix_bin": str(self.prefix_bin),
            "download_path": str(self.download_path),
            "src_path": str(self.src_path),
            "url": str(self.url),
            "product_exists": self.product_exists,
            "has_static_libs": self.has_static_libs,
            "project": self.project.to_dict(),
            "product": self.product.to_dict(),
            "depends_on": [str(i) for i in self.depends_on],
        }

    def to_yaml(self):
        """convert properties to yaml"""
        try:
            import yaml
            with open("dump.yml", "w", encoding="utf8") as fopen:
                fopen.write(
                    yaml.safe_dump(
                        self.to_dict(),
                        indent=4,
                        default_flow_style=False
                    )
                )
        except ImportError:
            self.log.error("could not import yaml module")

    def to_json(self):
        """convert properties to json"""
        with open("dump.json", "w", encoding="utf8") as fopen:
            json.dump(self.to_dict(), fopen, sort_keys=True, indent=4)

    def configure(self, *options, **kwargs):
        """configure builder"""

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

    def deploy(self, targets: List[str] = None):  # type: ignore
        """copies externals from external build dir to package/externals dir"""
        for ext in [f"{t}.mxo" for t in targets]:
            src = self.project.build_externals / ext
            dst = self.project.externals / ext
            if dst.exists():
                self.cmd.remove(dst)
            if src.exists():
                self.cmd.copy(src, dst)

    def xcodebuild(self, project: str, targets: List[str],
            *preprocessor_flags, **xcconfig_flags):
        """python wrapper around command-line xcodebuild"""

        if self.product.ver == "3.7" and not "PY_37" in preprocessor_flags:
            preprocessor_flags = list(preprocessor_flags)
            preprocessor_flags.append("PY_37")

        if self.settings.python_version:
            if "PY_VERSION" not in xcconfig_flags:
                xcconfig_flags["PY_VERSION"] = self.product.version

            if "PY_SHORT_VERSION" not in xcconfig_flags:
                xcconfig_flags["PY_SHORT_VERSION"] = self.product.ver

            if "ABIFLAGS" not in xcconfig_flags:
                xcconfig_flags["ABIFLAGS"] = str(self.product.abiflags)

            # if "NUMPY_HEADERS" not in xcconfig_flags and self.project.python.numpy_includes:
            #     xcconfig_flags["NUMPY_HEADERS"] = self.project.python.numpy_includes

        else:
            # defaults
            if "PY_VERSION" not in xcconfig_flags:
                xcconfig_flags["PY_VERSION"] = self.project.python.version

            if "PY_SHORT_VERSION" not in xcconfig_flags:
                xcconfig_flags["PY_SHORT_VERSION"] = self.project.python.version_short

            if "ABIFLAGS" not in xcconfig_flags:
                xcconfig_flags["ABIFLAGS"] = str(self.project.python.abiflags)

            if "NUMPY_HEADERS" not in xcconfig_flags and self.project.python.numpy_includes:
                xcconfig_flags["NUMPY_HEADERS"] = self.project.python.numpy_includes

        if "ARCHS" not in xcconfig_flags:
            # xcconfig_flags["ARCHS"] = "arm64 x86_64"
            xcconfig_flags["ARCHS"] = "$(NATIVE_ARCH)"
            xcconfig_flags["ONLY_ACTIVE_ARCH"] = "NO"


        xcconfig_flags["PROJECT_FOLDER_NAME"] = project

        # configuration = "Deployment" if self.settings.release else "Development"
        configuration = "Release" if self.settings.release else "Debug"
        x_flags = (
            " ".join([f"{k}={repr(v)}" for k, v in xcconfig_flags.items()])
            if xcconfig_flags
            else ""
        )
        p_flags = (
            "GCC_PREPROCESSOR_DEFINITIONS='$GCC_PREPROCESSOR_DEFINITIONS {flags}'".format(
                flags=" ".join([f"{k}=1" for k in preprocessor_flags])
            )
            if preprocessor_flags
            else ""
        )

        for target in targets:
            self.cmd(
                f"xcodebuild -project 'targets/{project}/py-js.xcodeproj'"
                f" -configuration {configuration}"
                f" -target {repr(target)} {x_flags} {p_flags}"
            )
        # self.deploy(targets)

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
        assert not (
            self.src_path.exists() or self.prefix.exists()
        ), "reset not completed"

    def download(self, include_dependencies=True):
        """download src using curl and tar.

        curl and tar are automatically available on mac platforms.
        """
        self.project.build_downloads.mkdir(parents=True, exist_ok=True)
        if include_dependencies:
            for dep in self.depends_on:
                dep.download()

        # download
        if self.download_path and not self.download_path.exists():
            self.project.build_downloads.mkdir(parents=True, exist_ok=True)
            self.log.info("downloading %s to %s", self.url, self.download_path)
            self.cmd(f"curl -L --fail '{self.url}' -o '{self.download_path}'")
            assert (
                self.download_path.exists()
            ), f"could not download: {self.download_path}"

        # unpack
        if not self.src_path.exists():
            self.project.build_src.mkdir(parents=True, exist_ok=True)
            self.log.info("unpacking %s", self.src_path)
            self.cmd(
                f"tar -xvf '{self.download_path}'"
                f" --directory '{self.project.build_src}'"
            )
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


class ConfiguredBuilder(Builder):
    def configure(self, *options, **kwargs):
        """generate ./configure instructions"""
        _kwargs = {}
        options = set(DEFAULT_CONFIGURE_OPTIONS).union(set(options))
        _options = [opt.replace("_", "-") for opt in options]
        _env = {}

        if self.default_env_vars:
            _env.update(self.default_env_vars)

        prefix = " ".join(f"{k}={v}" for k, v in _env.items()) if _env else ""

        for key, val in kwargs.items():
            _key = key.replace("_", "-")
            _kwargs[_key] = val

        options=" ".join(f"--{opt}" for opt in _options)
        kwargs=" ".join(f"--{k}='{v}'" for k, v in _kwargs.items())
        self.cmd(f"{prefix} ./configure {options} {kwargs}")


class Recipe:
    """A platform-specific container for multiple builder-centric projects."""

    # type: ignore
    def __init__(
        self,
        name: str,
        py_version: str = None,  # type: ignore
        builders: List[Builder] = None,  # type: ignore
        **settings,
    ):
        self.name = name
        self.py_version = py_version or CURRENT_PYTHON_VERSION
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


class Bzip2Builder(ConfiguredBuilder):
    """Bzip2 static library builder"""

    def build(self):
        if not self.product_exists:
            self.download()
            self.cmd.chdir(self.src_path)
            self.cmd(
                f"""MACOSX_DEPLOYMENT_TARGET={self.project.mac_dep_target} \
                    make install PREFIX={quote(self.prefix)}"""
            )
            self.cmd.chdir(self.project.pydir)
        else:
            self.log.info("product built already")


class OpensslBuilder(ConfiguredBuilder):
    """OpenSSL static library builder"""

    def build(self):
        if not self.product_exists:
            self.download()
            self.cmd.chdir(self.src_path)
            os.environ["MACOSX_DEPLOYMENT_TARGET"] = self.project.mac_dep_target
            self.cmd(
                f"""MACOSX_DEPLOYMENT_TARGET={self.project.mac_dep_target} \
                    ./config no-shared no-tests \
                    --prefix={quote(self.prefix)}"""
            )
            self.cmd(
                f"""MACOSX_DEPLOYMENT_TARGET='{self.project.mac_dep_target}' \
                    make install_sw"""
            )
            self.cmd.chdir(self.project.pydir)
        else:
            self.log.info("product built already")


class XzBuilder(ConfiguredBuilder):
    """Xz static library builder"""

    @property
    def product_exists(self):
        return self.prefix.exists()

    def build(self):
        if not self.product_exists:
            self.download()
            self.cmd.chdir(self.src_path)
            self.configure(
                "disable_shared",
                "enable_static",
                prefix=quote(self.prefix),
            )
            self.cmd(
                f"""MACOSX_DEPLOYMENT_TARGET='{self.project.mac_dep_target}' \
                    make && make install"""
            )
            self.cmd.chdir(self.project.pydir)
        else:
            self.log.info("product built already")


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

    def configure(self, *options, **kwargs):
        """configure builder"""

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
        self.rm_libs(self.product.DEFAULT_PKGS_TO_RM)

    def remove_extensions(self):
        """remove extensions"""
        self.rm_exts(self.product.DEFAULT_EXTS_TO_RM)

    def remove_binaries(self):
        """remove list of non-critical executables"""
        self.rm_bins(self.product.DEFAULT_BINS_TO_RM)

    def write_python_getpip(self):
        """optionally provide latets pip to binary"""
        with open(f"{self.prefix}/bin/get_pip.sh", encoding="utf8") as txtfile:
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
        assert self.lib_dynload.exists(), f"not found: {self.lib_dynload}"
        self.lib_dynload.rename(temp_lib_dynload)
        self.cmd.copy(self.python_lib / "os.py", temp_os_py)

        zip_path = self.prefix_lib / f"python{self.product.ver_nodot}"
        shutil.make_archive(str(zip_path), "zip", str(self.python_lib))

        self.cmd.remove(self.python_lib)
        self.python_lib.mkdir()
        temp_lib_dynload.rename(self.lib_dynload)
        temp_os_py.rename(self.python_lib / "os.py")
        self.site_packages.mkdir()

    def fix_dylib_for_shared_pkg(self, dylib):
        """install to dylib @rpath of @loader' to dylib in a shared-pkg"""
        self.cmd.chmod(dylib)
        self.install_name_tool_id(
            (
                "@loader_path/../../../../support/"
                f"{self.product.name_ver}/lib/{self.product.dylib}"
            ),
            dylib,
        )

    def fix_dylib_for_framework_pkg(self, dylib):
        """install to dylib @rpath of @loader' to dylib in a framework-pkg"""
        self.cmd.chmod(dylib)
        self.install_name_tool_id(
            f"@loader_path/../../../../support/Python.framework/Versions/{self.product.ver}/Python",
            dylib,
        )

    def fix_dylib_for_framework_ext(self, dylib):
        """install to dylib @rpath of @loader' to dylib in a framework-ext"""
        self.cmd.chmod(dylib)
        self.install_name_tool_id(
            f"@loader_path/../Resources/Python.framework/Versions/{self.product.ver}/Python",
            dylib,
        )

    def fix_dylib_for_shared_ext(self, dylib):
        """install to dylib @rpath of @loader' to dylib in a shared-ext"""
        self.cmd.chmod(dylib)
        self.install_name_tool_id(
            f"@loader_path/../Resources/lib/{self.product.dylib}", dylib
        )

    def fix_exe_for_shared_pkg(self, executable):
        """redirect ref of pythonX to libpythonX.Y.dylib"""
        dirs = DependencyManager(executable).analyze_executable()
        if dirs:
            dir_to_change = dirs[0]
            self.install_name_tool_change(
                dir_to_change,
                f"@executable_path/../lib/{self.product.dylib}",
                executable,
            )

    def fix_execs_for_framework_ext_or_pkg(self, py_exec, app_exec):
        """redirect ref of pythonX to libpythonX.Y.dylib"""
        fixes = [
            (py_exec, "@executable_path/../Python"),
            (app_exec, "@executable_path/../../../../Python"),
        ]
        for exe, backref in fixes:
            dirs = DependencyManager(exe).analyze_executable()
            if dirs:
                dir_to_change = dirs[0]
                self.install_name_tool_change(dir_to_change, backref, exe)


class PythonCmakeBuilder(PythonBuilder):
    """Generic Python builder from src using cmake buildsystem."""

    @property
    def prefix(self) -> Path:
        return self.project.build_lib / self.product.build_dir

    def rm_exts(self, names):
        """remove all named extensions"""
        for name in names:
            self.cmd.remove(
                self.python_lib
                / "lib-dynload"
                / f"{name}.so"
                # / f"{name}.cpython-{self.product.ver_nodot}-darwin.so"
            )

    def remove_packages(self):
        """remove list of non-critical packages"""
        self.rm_libs(self.product.DEFAULT_PKGS_TO_RM)

    def remove_extensions(self):
        """remove extensions"""
        self.rm_exts(self.product.DEFAULT_EXTS_TO_RM)

    def remove_binaries(self):
        """remove list of non-critical executables"""
        self.rm_bins(self.product.DEFAULT_BINS_TO_RM)

    def post_process(self):
        """post-build operations"""
        self.clean()
        # self.ziplib() # FIXME: python-cmake fails tests for zip
        self.fix_dylib_for_shared_ext(
            dylib=self.prefix_lib / self.product.dylib,
        )

    def git_clone(self, repo, to_dir):
        """retrieve git clone of repo"""
        self.cmd(f"git clone --depth=1 {repo} {to_dir}")

    def apply_patch(self, to_file, patch):
        """Apply a patch to a file.
        """
        self.cmd(f"patch {to_file} < '{patch}'")

    def cmake_generate(self, src_dir, build_dir, **options):
        """activate cmake configuration / generation stage"""
        _options = PYJS_CMAKE_DEFAULT_OPTIONS.copy()
        _options.update(options)
        opts = " ".join(f"-D{k}={v}" for k, v in _options.items())
        self.cmd(
            f"cmake -S {src_dir} -B {build_dir} {opts}"
        )

    def cmake_build(self, build_dir):
        """activate cmake build stage"""
        self.cmd(f"cmake --build {build_dir}")

    def cmake_install(self, build_dir):
        """activate cmake install stage"""
        self.cmd(f"cmake --install {build_dir}")

    def build(self):
        """build python"""
        python_cmake_buildsystem = self.project.build_downloads / 'python-cmake-buildsystem'
        python_cmake_build = self.project.build_src / 'python-cmake-build'
        python_cmake_install = self.project.build_lib / 'python-cmake'
        if not python_cmake_buildsystem.exists():
            self.git_clone(URL_PYTHON_CMAKE_BUILDSYSTEM, python_cmake_buildsystem)
        for _dir in [python_cmake_build, python_cmake_install]: # reset dirs every run
            if _dir.exists():
                shutil.rmtree(_dir)
            _dir.mkdir(exist_ok=True)
        self.cmake_generate(python_cmake_buildsystem, python_cmake_build, 
            CMAKE_INSTALL_PREFIX=python_cmake_install,
        )
        self.cmake_build(python_cmake_build)
        self.cmake_install(python_cmake_build)



class PythonSrcBuilder(PythonBuilder):
    """Generic Python builder from src using default python buildsystem."""

    setup_local: str = ""
    patch: str = ""

    def install(self):
        """install and build compilation product"""
        # self.configure()
        self.reset()
        self.download()
        self.pre_process()
        self.build()
        self.post_process()

    def configure(self, *options, **kwargs):
        """generate ./configure instructions"""
        _kwargs = {}
        options = set(DEFAULT_CONFIGURE_OPTIONS).union(set(options))
        _options = [opt.replace("_", "-") for opt in options]
        _env = {}

        if self.default_env_vars:
            _env.update(self.default_env_vars)

        prefix = " ".join(f"{k}={v}" for k, v in _env.items()) if _env else ""

        for key, val in kwargs.items():
            _key = key.replace("_", "-")
            _kwargs[_key] = val

        options=" ".join(f"--{opt}" for opt in _options)
        kwargs=" ".join(f"--{k}='{v}'" for k, v in _kwargs.items())
        self.cmd(f"{prefix} ./configure {options} {kwargs}")

    def pre_process(self):
        """pre-build operations"""
        self.cmd.chdir(self.src_path)
        self.write_setup_local()
        # python 11.x requires a lot of special cases for patching configure
        if self.product.ver == "11" and int(self.product.ver_patch) < 4:
            self.apply_patch(patch="configure_pre_11_4.patch", to_file="configure")
        elif self.product.ver == "11" and int(self.product.ver_patch) in [4, 5]:
            self.apply_patch(patch="configure_11_4to5.patch", to_file="configure")
        else:
            self.apply_patch(patch="configure.patch", to_file="configure")
        self.cmd.chdir(self.project.pydir)

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
        """Apply a standard patch from the patch directory.

        (prefixed by major.minor ver)

        Patches are stored in their short_version subdirectory.
        if param `to_file` is given
            then patch is applied directly to the file (diff method)
        otherwise:
            the patch is applied to the directory itself (git method)
        """
        if not any([patch, self.patch]):
            return
        if not patch:
            patch = self.patch
        if to_file:
            self.cmd(
                f"patch {to_file} < '{self.project.patch}/{self.product.ver}/{patch}'"
            )
        else:
            self.cmd(f"patch -p1 < '{self.project.patch}/{self.product.ver}/{patch}'")


# ------------------------------------------------------------------------------------
# PYTHON BUILDERS (BASE)


class FrameworkPythonBuilder(PythonSrcBuilder):
    """builds python in a macos framework format."""

    setup_local = "setup-shared.local"

    @property
    def prefix(self) -> Path:
        return (
            self.project.build_lib / "Python.framework" / "Versions" / self.product.ver
        )

    def reset(self):
        self.cmd.remove(self.src_path)
        self.cmd.remove(self.project.build_lib / "Python.framework")

    def build(self):
        for dep in self.depends_on:
            dep.build()

        self.cmd.chdir(self.src_path)

        kwargs = {
            "enable_framework": quote(self.project.build_lib), 
            "with_openssl": quote(self.project.build_lib / 'openssl'),
        }

        if self.project.python.arch == 'arm64':
            kwargs['with_universal_archs'] = 'universal2'

        self.configure(
            # "enable_ipv6",
            # "enable_optimizations",
            # "with_lto",
            # "without_doc_strings",
            "without_ensurepip",
            # "disable_test_modules",
            # "with_system_libmpdec",
            **kwargs,
        )

        self.cmd("make altinstall")
        self.cmd.chdir(self.project.pydir)

    # PYTHONBUG: Python.framework/Versions/3.X/Resources/Python.app
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
        self.configure(
            # "enable_ipv6",
            # "enable_optimizations",
            "enable_shared",
            "without_static_libpython",
            # "with_lto",
            # "without_doc_strings",
            "without_ensurepip",
            # "disable_test_modules",
            # "with_system_libmpdec",
            prefix=quote(self.prefix),
            with_openssl=quote(self.project.build_lib / "openssl"),
        )

        self.cmd("make altinstall")
        self.cmd.chdir(self.project.pydir)


class StaticPythonBuilder(PythonSrcBuilder):
    """builds python in a static format."""

    setup_local = "setup-static-min3.local"

    @property
    def prefix(self) -> Path:
        return self.project.build_lib / self.product.build_dir

    def build(self):
        for builder in self.depends_on:
            builder.build()

        self.cmd.chdir(self.src_path)

        self.configure(
            # "enable_ipv6",
            # "enable_optimizations",
            # "with_lto",
            # "without_doc_strings",
            "without_ensurepip",
            # "disable_test_modules",
            prefix=quote(self.prefix),
            with_openssl=quote(self.project.build_lib / "openssl"),
        )

        self.cmd("make altinstall")
        self.cmd.chdir(self.project.pydir)

    def remove_extensions(self):
        """remove extensions: not implemented"""


class BeewarePythonBuilder(StaticPythonBuilder):
    """builds python in a macos static format."""

    setup_local = "setup.beeware"

    def pre_process(self):
        """pre-build operations"""
        self.cmd.chdir(self.src_path)
        self.write_setup_local()
        self.apply_patch(patch="beeware.patch") # FIXME: not available
        self.apply_patch(patch="configure.patch", to_file="configure")
        self.cmd.chdir(self.project.pydir)

    def remove_extensions(self):
        """remove extensions"""
        self.rm_exts(
            [
                "_codecs_cn",
                "_codecs_hk",
                "_codecs_iso2022",
                "_codecs_jp",
                "_codecs_kr",
                "_codecs_tw",
                "_crypt",
                "_curses",
                "_curses_panel",
                "_dbm",
                "_gdbm",
                # "_multibytecodec",
                "_posixshmem",
                "_tkinter",
                "_xxsubinterpreters",
                "_zoneinfo",
                "audioop",
                "fcntl",
                "grp",
                "nis",
                "readline",
                "resource",
                "termios",
                "xxlimited",
                "xxlimited_35",
            ]
        )

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
                "curses",
                "ensurepip",
                "venv",
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
            ]
        )


class TinyStaticPythonBuilder(PythonSrcBuilder):
    """builds python in a static format."""

    setup_local = "setup-static-min6.local"

    @property
    def prefix(self) -> Path:
        return self.project.build_lib / self.product.build_dir

    def rm_exts(self, names):
        """remove all named extensions"""
        for name in names:
            self.cmd.remove(
                self.python_lib
                / "lib-dynload"
                / f"{name}.cpython-{self.product.ver_nodot}-darwin.so"
            )

    def remove_extensions(self):
        """remove extensions"""
        self.rm_exts(
            self.product.DEFAULT_EXTS_TO_RM.union(set(
            [
                "_blake2",
                "_csv",
                "_elementtree",
                "_json",
                "_multiprocessing",
                "_pickle",
                "_zoneinfo",
                "pyexpat",
                # "unicodedata",
            ]))
        )

    def remove_encodings(self):
        """remove all uneeded encodings"""
        keep = [
            "__init__.py",
            "aliases.py",
            "ascii.py",
            "latin_1.py",
            "utf_8.py",
        ]
        encodings = self.python_lib / "encodings"
        for filename in encodings.iterdir():
            if filename.name in keep:
                continue
            self.cmd.remove(filename)

    def remove_packages(self):
        """remove list of non-critical packages"""
        self.remove_encodings()
        self.rm_libs(
            self.product.DEFAULT_PKGS_TO_RM.union(set([
                "argparse.py",
                "dbm",
                "difflib.py",
                "email",
                "html",
                "mailbox",
                "mailbox.py",
                "multiprocessing",
                "optparse.py",
                "pickletools.py",
                "pydoc.py",
                "pydoc_data",
                "sqlite3",
                "ssl.py",
                "urllib",
                "wsgiref",
                "xml",
                "zoneinfo",
            ]))
        )

    def install(self):
        """install and build compilation product"""
        # self.configure()
        self.reset()
        self.download()
        self.pre_process()
        self.build()
        self.post_process()

    def post_process(self):
        """post-build operations"""
        self.clean()
        self.ziplib()

    def remove_binaries(self):
        """remove list of non-critical executables"""
        ver = self.product.ver
        self.rm_bins(self.product.DEFAULT_BINS_TO_RM)

    def build(self):
        self.cmd.chdir(self.src_path)

        self.configure(
            # "enable_ipv6",
            # "enable_optimizations",
            # "with_lto",
            # "without_doc_strings",
            "without_ensurepip",
            # "disable_test_modules",
            prefix=quote(self.prefix),
            # with_openssl=quote(self.project.build_lib / "openssl"),
        )

        self.cmd("make altinstall")
        self.cmd.chdir(self.project.pydir)


class TinySharedPythonBuilder(PythonSrcBuilder):
    """builds python in a tiny shared format."""

    setup_local = "setup-static-min6.local"

    @property
    def prefix(self) -> Path:
        return self.project.build_lib / self.product.build_dir

    def rm_exts(self, names):
        """remove all named extensions"""
        for name in names:
            self.cmd.remove(
                self.python_lib
                / "lib-dynload"
                / f"{name}.cpython-{self.product.ver_nodot}-darwin.so"
            )

    def remove_extensions(self):
        """remove extensions"""
        self.rm_exts(
            self.product.DEFAULT_EXTS_TO_RM.union(set(
            [
                "_blake2",
                "_csv",
                "_elementtree",
                "_json",
                "_multiprocessing",
                "_pickle",
                "_zoneinfo",
                "pyexpat",
                # "unicodedata",
            ]))
        )

    def remove_encodings(self):
        """remove all uneeded encodings"""
        keep = [
            "__init__.py",
            "aliases.py",
            "ascii.py",
            "latin_1.py",
            "utf_8.py",
        ]
        encodings = self.python_lib / "encodings"
        for filename in encodings.iterdir():
            if filename.name in keep:
                continue
            self.cmd.remove(filename)

    def remove_packages(self):
        """remove list of non-critical packages"""
        self.remove_encodings()
        self.rm_libs(
            self.product.DEFAULT_PKGS_TO_RM.union(set([
                "argparse.py",
                "dbm",
                "difflib.py",
                "email",
                "html",
                "mailbox",
                "mailbox.py",
                "multiprocessing",
                "optparse.py",
                "pickletools.py",
                "pydoc.py",
                "pydoc_data",
                "sqlite3",
                "ssl.py",
                "urllib",
                "wsgiref",
                "xml",
                "zoneinfo",
            ]))
        )

    def install(self):
        """install and build compilation product"""
        # self.configure()
        self.reset()
        self.download()
        self.pre_process()
        self.build()
        self.post_process()

    def post_process(self):
        """post-build operations"""
        self.clean()
        self.ziplib()

    def remove_binaries(self):
        """remove list of non-critical executables"""
        ver = self.product.ver
        self.rm_bins(self.product.DEFAULT_BINS_TO_RM)

    def build(self):
        self.cmd.chdir(self.src_path)

        self.configure(
            # "enable_ipv6",
            # "enable_optimizations",
            # "with_lto",
            "without_doc_strings",
            "enable_shared",
            "without_static_libpython",
            "without_ensurepip",
            "disable_test_modules",
            prefix=quote(self.prefix),
            # with_openssl=quote(self.project.build_lib / "openssl"),
        )

        self.cmd("make altinstall")
        self.cmd.chdir(self.project.pydir)

# ------------------------------------------------------------------------------------
# PYTHON BUILDERS (BINARY)


class RelocatablePythonBuilder(PythonBuilder):
    """pyjs externals in a framework package using Greg Neagle's Relocatable Python"""

    @property
    def prefix(self) -> Path:
        return self.project.support / "Python.framework" / "Versions" / self.product.ver

    def install(self):
        """install and build compilation product"""
        self.reset()
        self.configure()
        self.download()
        self.post_process()

    def configure(self, *options, **kwargs):
        """configures overrides to defaults from commandline"""
        if self.settings.python_version != self.product.version:
            self.product.version = self.settings.python_version

    def reset(self):
        """remove framework in suppor directory"""
        framework = self.project.support / "Python.framework"
        self.cmd.remove(framework)
        assert not framework.exists(), "reset not completed"

    def download(self, include_dependencies=True):
        """download relocatable python"""
        self.project.build_downloads.mkdir(parents=True,  exist_ok=True)
        download_relocatable_to(self.project.support, self.settings)

    def post_process(self):
        """post-build operations"""
        self.clean()
        self.ziplib()

    def temp_remove_site_packages(self):
        """move site_packages to a tmp directory"""
        tmp_dir = tempfile.mkdtemp()
        self.cmd.move(self.site_packages, tmp_dir)
        return tmp_dir

    def restore_site_packages(self, tmp_dir):
        """restore site_packages from tmp directory"""
        tmp_dir = Path(tmp_dir)
        self.cmd.move(tmp_dir / 'site-packages', self.site_packages)

    def clean(self):
        """clean everything."""
        self.clean_python_pyc(self.prefix)
        tmp_dir = self.temp_remove_site_packages()
        self.clean_python_tests(self.python_lib)
        # self.clean_python_site_packages(self.python_lib)
        self.restore_site_packages(tmp_dir)
        for i in (self.python_lib / "distutils" / "command").glob("*.exe"):
            self.cmd.remove(i)

        self.cmd.remove(self.prefix_lib / "pkgconfig")
        self.cmd.remove(self.prefix / "share")

        self.remove_packages()
        self.remove_extensions()
        self.remove_binaries()
        self.remove_tkinter()

    def remove_extensions(self):
        """remove extensions"""
        self.rm_exts(
            [
                "_tkinter",
                # "_ctypes",
                # "_multibytecodec",
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
                # "ctypes",
                "curses",
                "ensurepip",
                "venv",
            ]
        )

    def remove_binaries(self):
        """remove list of non-critical executables"""
        ver = self.project.python.version_short
        self.rm_bins(
            [
                f"2to3-{ver}",
                "2to3",
                f"idle{ver}",
                "idle3",
                f"easy_install-{ver}",
                f"pip{ver}",
                # f"pip3",
                f"pyvenv-{ver}",
                f"pydoc{ver}",
                "pydoc3",
                # f"python3",
                # f"python3-config",
            ]
        )

    def rm_globbed(self, names):
        """remove all named glob patterns of libraries and files"""
        for name in names:
            for filename in self.prefix_lib.glob(name):
                self.cmd.remove(filename)

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
        self.cmd.move(self.site_packages, "/tmp/site-packages")
        self.lib_dynload.rename(temp_lib_dynload)
        self.cmd.copy(self.python_lib / "os.py", temp_os_py)

        zip_path = self.prefix_lib / f"python{self.product.ver_nodot}"
        shutil.make_archive(str(zip_path), "zip", str(self.python_lib))

        self.cmd.remove(self.python_lib)
        self.python_lib.mkdir()
        temp_lib_dynload.rename(self.lib_dynload)
        temp_os_py.rename(self.python_lib / "os.py")
        # self.site_packages.mkdir()
        self.cmd.move("/tmp/site-packages", self.site_packages)


# ------------------------------------------------------------------------------------
# PYTHON BUILDERS (SPECIALIZED)


class SharedPythonForExtBuilder(SharedPythonBuilder):
    """builds python in a shared format for self-contained externals."""

    setup_local = "setup-shared.local"

    def post_process(self):
        """post-build operations"""
        self.clean()
        self.ziplib()
        self.fix_dylib_for_shared_ext(
            dylib=self.prefix_lib / self.product.dylib,
        )


class SharedPythonForPkgBuilder(SharedPythonBuilder):
    """builds python in a shared format for self-contained externals."""

    setup_local = "setup-shared.local"

    def pre_process(self):
        """pre-build operations"""
        self.cmd.chdir(self.src_path)
        self.write_setup_local()
        self.apply_patch(patch="configure.patch", to_file="configure")
        self.cmd.chdir(self.project.pydir)

    def remove_packages(self):
        """remove list of non-critical packages"""
        self.rm_libs(self.product.DEFAULT_PKGS_TO_RM)

    def post_process(self):
        """post-build operations"""
        self.clean()
        self.ziplib()
        self.fix_dylib_for_shared_pkg(
            dylib=self.prefix / "lib" / self.product.dylib,
        )
        self.fix_exe_for_shared_pkg(executable=self.prefix_bin / self.product.name_ver)


class FrameworkPythonForExtBuilder(FrameworkPythonBuilder):
    """builds python in a framework format for self-contained externals."""

    setup_local = "setup-shared.local"

    def post_process(self):
        """post-build operations"""
        self.clean()
        self.ziplib()
        self.fix_dylib_for_framework_ext(self.prefix / "Python")
        self.fix_execs_for_framework_ext_or_pkg(
            py_exec=self.prefix_bin / self.product.name_ver,
            app_exec=(
                self.prefix_resources / "Python.app" / "Contents" / "MacOS" / "Python"
            ),
        )


class FrameworkPythonForPkgBuilder(FrameworkPythonBuilder):
    """builds python in a framework format for relocatable max packages."""

    setup_local = "setup-shared.local"

    def pre_process(self):
        """pre-build operations"""
        self.cmd.chdir(self.src_path)
        self.write_setup_local()
        self.apply_patch(patch="configure.patch", to_file="configure")
        self.cmd.chdir(self.project.pydir)

    def remove_packages(self):
        """remove list of non-critical packages"""
        self.rm_libs(self.product.DEFAULT_PKGS_TO_RM)

    def post_process(self):
        """post-build operations"""
        self.clean()
        self.ziplib()
        self.fix_dylib_for_framework_pkg(self.prefix / "Python")
        self.fix_execs_for_framework_ext_or_pkg(
            py_exec=self.prefix_bin / self.product.name_ver,
            app_exec=(
                self.prefix_resources / "Python.app" / "Contents" / "MacOS" / "Python"
            ),
        )


# ------------------------------------------------------------------------------------
# PYJS EXTERNAL BUILDERS (ABSTRACT)


class PyJsBuilder(PythonBuilder):
    """pyjs concrete base class"""

    @property
    def prefix(self):
        # return self.project.support / self.project.python.name
        return self.project.support / self.product.name_ver

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


class HomebrewExtBuilder(PyJsBuilder):
    """homebrew python builder"""

    suffix = ""
    setup_local: str = ""
    patch: str = ""
    NAME = "homebrew-ext"

    def cp_pkgs(self, pkgs):
        """copy package dirs from homebrew python lib to target python lib"""
        for pkg in pkgs:
            self.cmd.copy(self.project.python.pkgs / pkg, self.python_lib / pkg)

    def rm_libs(self, names):
        """remove all named python dylib libraries"""
        for name in names:
            self.cmd.remove(self.python_lib / name)

    def clean_python(self):
        """clean everything."""
        self.clean_python_pyc(self.prefix)
        self.clean_python_tests(self.python_lib)
        for i in (self.python_lib / "distutils" / "command").glob("*.exe"):
            self.cmd.remove(i)

        self.remove_packages()
        self.remove_extensions()

    def fix_exe_for_shared_ext_or_pkg(self, executable):
        """redirect ref of pythonX to libpythonX.Y.dylib" """
        dirs = DependencyManager(executable).analyze_executable()
        if dirs:
            dir_to_change = dirs[0]
            self.install_name_tool_change(
                dir_to_change, f"@executable_path/../{self.product.dylib}", executable
            )

    def fix_dylib_for_shared_ext(self, dylib):
        """install to dylib @rpath of @loader' to dylib in a shared-ext"""
        self.cmd.chmod(dylib)
        self.install_name_tool_id(
            f"@loader_path/../Resources/{self.product.name_ver}/{self.product.dylib}",
            dylib,
        )

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
                "re",
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
        """build external into self-contained external using local homebrew python (portable)"""
        self.reset_prefix()
        self.copy_python()
        self.fix_exe_for_shared_ext_or_pkg(self.prefix_bin / self.product.name_ver)
        self.fix_dylib_for_shared_ext(self.prefix / self.product.dylib)
        self.cp_python_to_ext_resources(self.project.py_external)
        self.cp_python_to_ext_resources(self.project.pyjs_external)
        self.xcodebuild(self.NAME, targets=["py", "pyjs"])
        self.reset_prefix()


class HomebrewPkgBuilder(PyJsBuilder):
    """homebrew python builder"""

    suffix = ""
    setup_local: str = ""
    patch: str = ""

    NAME = "homebrew-pkg"

    @property
    def prefix(self):
        return (
            self.project.support
            / "Python.framework"
            / "Versions"
            / f"{self.project.python.version_short}"
        )

    def remove_binaries(self):
        """remove list of non-critical executables"""
        ver = self.project.python.version_short
        self.rm_bins(
            [
                f"2to3-{ver}",
                "2to3",
                f"idle{ver}",
                "idle3",
                f"easy_install-{ver}",
                f"pip{ver}",
                "pip3",
                f"pyvenv-{ver}",
                f"pydoc{ver}",
                "pydoc3",
                "python3",
                "python3-config",
            ]
        )

    def copy_python(self):
        """copy python framework from homebrew to support directory"""
        src = self.project.python.prefix.parent.parent
        self.cmd(f'ditto {src} {self.project.support / "Python.framework"}')
        self.clean()
        self.ziplib()

    def reset(self):
        """remove framework in support"""
        self.cmd.remove(self.project.support / "Python.framework")

    def install(self):
        """build externals into package use local homebrew python (portable)"""
        self.reset()
        self.copy_python()
        assert self.prefix.exists()
        self.fix_dylib_for_framework_pkg(self.prefix / "Python")
        self.fix_execs_for_framework_ext_or_pkg(
            py_exec=self.prefix_bin / self.product.name_ver,
            app_exec=(
                self.prefix_resources / "Python.app" / "Contents" / "MacOS" / "Python"
            ),
        )
        self.xcodebuild(self.NAME, targets=["py", "pyjs"])


class LocalSystemBuilder(PyJsBuilder):
    """Builds externals from local python (non-portable)"""

    NAME = "local-sys"

    def build(self):
        """builds externals from local system python"""

        flags = {
            "PREFIX": str(self.project.python.prefix), 
            "LIBS": str(self.project.python.libs),
        }

        self.xcodebuild(self.NAME, targets=["py", "pyjs"], **flags)


class StaticExtBuilder(PyJsBuilder):
    """pyjs externals from minimal statically built python"""

    NAME = "static-ext"

    @property
    def product_exists(self):
        static_lib = (
            self.project.build_lib
            / "python-static"
            / "lib"
            / self.product.staticlib  # type: ignore
        )  # type: ignore
        if not static_lib.exists():
            self.log.warning("static python is not built: %s", static_lib)
        return static_lib.exists()

    def build(self):
        """builds externals from statically built python"""

        if self.product_exists:
            self.xcodebuild(self.NAME, targets=["py", "pyjs"])


class SharedExtBuilder(PyJsBuilder):
    """pyjs externals from minimal statically built python"""

    NAME = "shared-ext"

    @property
    def product_exists(self):
        shared_lib = (
            # self.project.build_lib / "python-shared" / "lib" / self.project.python.dylib
            self.project.build_lib / "python-shared" / "lib" / self.product.dylib
        )
        if not shared_lib.exists():
            self.log.warning("shared python is not built: %s", shared_lib)
        return shared_lib.exists()

    def build(self):
        """builds externals from shared python"""

        if self.product_exists:
            self.xcodebuild(self.NAME, targets=["py", "pyjs"])


class SharedPkgBuilder(PyJsBuilder):
    """pyjs externals in a package from minimal statically built python"""

    NAME = "shared-pkg"

    @property
    def product_exists(self):
        shared_lib = (
            self.project.build_lib / "python-shared" / "lib" / self.product.dylib
        )
        if not shared_lib.exists():
            self.log.warning("shared python is not built: %s", shared_lib)
        return shared_lib.exists()

    def build(self):
        """builds externals from shared python"""
        src = self.project.build_lib / "python-shared"
        dst = f"{self.project.support}/{self.product.name_ver}"
        self.cmd(f"rm -rf '{dst}'")  # try to remove if it exists
        self.cmd(f"cp -af '{src}' '{dst}'")

        if self.product_exists:
            self.xcodebuild(self.NAME, targets=["py", "pyjs"])


class FrameworkExtBuilder(PyJsBuilder):
    """pyjs externals from minimal framework built python"""

    NAME = "framework-ext"

    @property
    def product_exists(self):
        shared_lib = (
            self.project.build_lib
            / "Python.framework"
            / "Versions"
            / self.product.ver
            / "Python"
        )
        if not shared_lib.exists():
            self.log.warning("framework python is not built: %s", shared_lib)
        return shared_lib.exists()

    def build(self):
        """builds externals from shared python"""
        if self.product_exists:
            self.xcodebuild(self.NAME, targets=["py", "pyjs"])


class FrameworkPkgBuilder(PyJsBuilder):
    """pyjs externals in a package from minimal framework built python"""

    NAME = "framework-pkg"

    @property
    def product_exists(self):
        shared_lib = (
            self.project.build_lib
            / "Python.framework"
            / "Versions"
            / self.product.ver
            / "Python"
        )
        if not shared_lib.exists():
            self.log.warning("framework python is not built: %s", shared_lib)
        return shared_lib.exists()

    def build(self):
        """builds externals from framework python"""
        src = self.project.build_lib / "Python.framework"
        dst = self.project.support / "Python.framework"
        self.cmd(f"rm -rf '{dst}'")  # try to remove if it exists
        self.cmd(f"cp -af '{src}' '{dst}'")
        if self.product_exists:
            self.xcodebuild(self.NAME, targets=["py", "pyjs"])


class RelocatablePkgBuilder(PyJsBuilder):
    """External builder related to Relocatable Python"""

    NAME = "relocatable-pkg"

    @property
    def product_exists(self):
        py_framework = self.project.support / "Python.framework"
        if not py_framework.exists():
            self.log.warning("framework python is not built: %s", py_framework)
        return py_framework.exists()

    def build(self):
        """builds externals from framework python"""

        if self.product_exists:
            py_exe = (
                self.project.support
                / "Python.framework"
                / "Versions"
                / "Current"
                / "bin"
                / "python3"
            )
            def get(name):
                return subprocess.check_output([
                    py_exe,
                    "-c",
                    f"import sysconfig; print(sysconfig.get_config_var('{name}'))"
                ],
                text=True).strip()

            self.xcodebuild(
                self.NAME,
                targets=["py", "pyjs"],
                PY_VERSION=get("py_version"),
                PY_SHORT_VERSION=get("py_version_short"),
                ABIFLAGS=get("abiflags"),
            )


class BeewareExtBuilder(PyJsBuilder):
    """pyjs externals from minimal statically built python"""

    NAME = "beeware-ext"

    @property
    def product_exists(self):
        static_lib = (
            self.project.build_lib
            / "python-beeware"
            / "lib"
            / self.project.python.staticlib  # type: ignore
        )  # type: ignore
        if not static_lib.exists():
            self.log.warning("static python is not built: %s", static_lib)
        return static_lib.exists()

    def build(self):
        """builds externals from statically built python"""

        if self.product_exists:
            self.xcodebuild(self.NAME, targets=["py", "pyjs"])
