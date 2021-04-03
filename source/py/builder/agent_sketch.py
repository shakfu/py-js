"""model: schema of builder
"""
import logging
import os
import platform
import shutil
from pathlib import Path
from types import SimpleNamespace

# from typing import List, Optional, Type, Union

DEBUG = False
LOG_LEVEL = logging.DEBUG if DEBUG else logging.INFO
LOG_FORMAT = "%(relativeCreated)-4d %(levelname)-5s: %(name)-10s %(message)s"
PYTHON_VERSION_STRING = platform.python_version()

logging.basicConfig(format=LOG_FORMAT, level=LOG_LEVEL)


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

    def copytree(self, src, dst):
        """Copy recursively from src path to dst path."""
        self.log.info("move tree %s to %s", src, dst)
        shutil.copytree(src, dst)

    def copyfile(self, src, dst):
        """Copy file from src path to dst path."""
        self.log.info("copy %s to %s", src, dst)
        shutil.copyfile(src, dst)

    def remove(self, path):
        """Remove file or folder."""
        if path.is_dir():
            self.log.info("remove folder: %s", path)
            shutil.rmtree(path, ignore_errors=not DEBUG)
        else:
            self.log.info("remove file: %s", path)
            path.unlink(missing_ok=True)


class Settings(SimpleNamespace):
    """A dictionary object with dotted access to its members.

    >>> settings = Settings(**dict)
    """

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


class Project:
    """A repository for all the files, resources, and information required to
    build one or more software products.
    """

    name = "Python"
    py_version = platform.python_version()
    py_ver = ".".join(py_version.split(".")[:2])
    py_name = f"python{py_ver}"

    # current working directory
    root = Path.cwd()

    # project-build section
    scripts = root / "scripts"
    patch = root / "patch"
    targets = root / "targets"
    build = targets / "build"
    downloads = build / "downloads"
    src = build / "src"
    lib = build / "lib"

    homebrew = (
        Path("/usr/local/opt/python3/Frameworks/Python.framework/Versions") / py_ver
    )

    homebrew_pkgs = homebrew / "lib" / py_name

    # project
    pyjs = root.parent.parent
    support = pyjs / "support"
    externals = pyjs / "externals"

    # prefix = support / py_name
    # bin = prefix / "bin"
    # lib = prefix / "lib" / py_name

    py_external = externals / "py.mxo"
    pyjs_external = externals / "pyjs.mxo"

    dylib = f"libpython_{py_ver}.dylib"

    # environmental vars
    HOME = os.getenv("HOME")
    package_name = "py"
    package = f"{HOME}/Documents/Max 8/Packages/{package_name}"
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
    mac_dep_target = "10.14"

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    def __repr__(self):
        return self.__str__()

    def __hash__(self):
        return hash((self.name, self.py_name, self.mac_dep_target))


class Product:
    """A product of a builder."""
    def __init__(self, name: str, version: str, **settings):
        self.name = name
        self.version = version
        self.settings = Settings(**settings)

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    @property
    def ver(self) -> str:
        """provides major.minor version: 3.9.1 -> 3.9"""
        return ".".join(self.version.split(".")[:2])

    @property
    def ver_nodot(self) -> str:
        """provides 'majorminor' version without space in between: 3.9.1 -> 39"""
        return self.ver.replace(".", "")

    @property
    def name_version(self) -> str:
        """Product-version: Python-3.9.1"""
        return f"{self.name}-{self.version}"

    @property
    def name_ver(self) -> str:
        """Product-major.minor: python-3.9"""
        return f"{self.name.lower()}{self.ver}"

    @property
    def name_archive(self) -> str:
        """Archival name of Product-version: Python-3.9.1.tgz"""
        return f"{self.name_version}.tgz"

    @property
    def dylib(self) -> str:
        """name of dynamic library in macos case."""
        return f"lib{self.name.lower()}{self.ver}.dylib"

    @property
    def url(self) -> Path:
        """Returns url to download product src as a pathlib.Path instance."""
        if url_template := self.settings.url_template:
            return Path(url_template.format(name=self.name, version=self.version))
        raise KeyError("url_template not providing in settings")


class Builder:
    """A Builder know how to build a single product type in a project."""

    def __init__(
        self,
        product: Product,
        project: Project,
        depends_on: list["Builder"] = None,
        **settings,
    ):
        self.product = product
        self.project = project
        self.depends_on = depends_on or []
        self.settings = Settings(**settings)
        self.log = logging.getLogger(self.__class__.__name__)
        self.cmd = ShellCmd(self.log)

    def __str__(self):
        return f"<{self.__class__.__name__}: '{self.project.name}/{self.product.name}'>"

    @property
    def prefix(self) -> Path:
        """compiled product destination root directory."""
        return self.project.lib / self.product.name.lower()

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
    def download_path(self) -> Path:
        """Returns path to downloaded product-version archive."""
        return self.project.downloads / self.product.name_archive

    @property
    def src_path(self) -> Path:
        """Return product source directory."""
        return self.project.src / self.product.name_version

    @property
    def lib_path(self) -> Path:
        """alias to self.prefix"""
        return self.prefix

    @property
    def url(self) -> Path:
        """Returns url to download product as a pathlib.Path instance."""
        return self.product.url

    @property
    def has_static_libs(self) -> bool:
        """check for presence of static libs"""
        if (libs := self.product.settings.libs_static) :
            return all((self.prefix_lib / lib).exists() for lib in libs)
        return False

    # -------------------------------------------------------------------------
    # Core Methods

    def reset_prefix(self):
        """remove prefix or compilation destinations"""
        self.cmd.remove(self.prefix)

    def reset(self):
        """remove product src directory and compiled product directory."""
        self.cmd.remove(self.src_path)
        self.cmd.remove(self.prefix)  # aka self.prefix

    def download(self):
        """download src using curl and tar.

        curl and tar are automatically available on mac platforms.
        """
        self.project.downloads.mkdir(parents=True, exist_ok=True)
        for dep in self.depends_on:
            dep.download()

        # download
        if not self.download_path.exists():
            self.log.info("downloading %s", self.download_path)
            self.cmd(f"curl -L --fail {self.url} -o {self.download_path}")

        # unpack
        if not self.src_path.exists():
            self.project.src.mkdir(parents=True, exist_ok=True)
            self.log.info("unpacking %s", self.src_path)
            self.cmd(f"tar -C {self.project.src} -xvf {self.download_path}")

    def build(self):
        """build target from src"""

    def pre_process(self):
        """pre-build operations"""

    def post_process(self):
        """post-build operations"""

    def deploy(self):
        """deploy to package"""
        self.cmd(f"mkdir -p {self.project.package}")
        for subdir in self.project.package_dirs:
            self.cmd(
                f"rsync -a --delete {self.project.pyjs}/{subdir} {self.project.package}"
            )
