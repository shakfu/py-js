"""model: schema of builder

Basically a simplified copy of the xcode model, with thhe difference that I have
renamed `Target` as `Builder` and `Workspace` as `Recipe` since it
makes more sense in this context.

    Recipe -- Settings
        |           ∆
        |*          |
        Project -- Settings
            |           ∆
            |*          |
            Builder -- Settings
                |
                ▽
                Product

"""

import logging
import os
import shutil
import platform
from abc import ABC
from pathlib import Path
from types import SimpleNamespace
from typing import List, Type

DEBUG = True
LOG_LEVEL = logging.DEBUG if DEBUG else logging.INFO
LOG_FORMAT = "%(relativeCreated)-4d %(levelname)-5s: %(name)-10s %(message)s"
PYTHON_VERSION_STRING = platform.python_version()

# ------------------------------------------------------------------------------
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


# ------------------------------------------------------------------------------
# Abstract Classes


class Product(ABC):
    """Produced by running a builder."""

    default_name: str
    default_version: str
    url_template: str
    libs_static: list[str]

    def __init__(
        self,
        name: str = None,
        version: str = None,
        path: Path = None,
        url_template: str = None,
    ):
        self.name = name or self.default_name
        self.version = version or self.default_version
        self.path = Path(path) if path else None
        self.url_template = url_template or self.url_template

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    def exists(self) -> bool:
        """returns True if product exists at self.path"""
        return self.path.exists()

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
        return f"lib{self.name.lower()}{self.ver}.dylib"  # pylint: disable=E1101

    @property
    def url(self) -> Path:
        """Returns url to download product src as a pathlib.Path instance."""
        return Path(self.url_template.format(name=self.name, version=self.version))

    @property
    def dst(self) -> Path:
        """compiled product destination root directory."""
        # return self.project.lib / self.name.lower()
        return self.path / self.name.lower()

    @property
    def prefix(self) -> Path:
        """compiled product destination root directory."""
        # return self.project.lib / self.name.lower()
        return self.dst

    @property
    def prefix_lib(self) -> Path:
        """compiled product destination lib directory."""
        return self.dst / "lib"

    @property
    def prefix_include(self) -> Path:
        """compiled product destination include directory."""
        return self.dst / "include"

    @property
    def prefix_bin(self) -> Path:
        """compiled product destination bin directory."""
        return self.dst / "bin"

    @property
    def has_static_libs(self) -> bool:
        """check for presence of static libs"""
        return all((self.prefix_lib / lib).exists() for lib in self.libs_static)


class Project(ABC):
    """A repository for all the files, resources, and information required to
    build one or more software products.
    """

    builder_classes: List[Type["Builder"]]

    def __init__(self, name: str = None, builders: list["Builder"] = None, **settings):
        self.name = name
        self.builders = builders or [
            builder_class(project=self) for builder_class in self.builder_classes
        ]

        self.settings = Settings(**settings)

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    def build(self):
        """delegate building to builders"""
        for builder in self.builders:
            builder.build()


class Builder(ABC):
    """A Builder know how to build a single product type in a project.

    A Builder is analagous to a Target in Xcode.
    """

    product_class: Type[Product]
    project_class: Type[Project]
    dependencies: List[Type["Builder"]]

    def __init__(
        self,
        product: Product = None,
        project: "Project" = None,
        depends_on: list["Builder"] = None,
        **settings,
    ):
        self.product = product or self.product_class()
        self.project = project or self.project_class()
        self.depends_on = depends_on or [
            klass(project=self.project) for klass in self.dependencies
        ]
        self.settings = Settings(**settings)
        self.log = logging.getLogger(self.__class__.__name__)
        self.cmd = ShellCmd(self.log)

    def __str__(self):
        return f"<{self.__class__.__name__}>"

    def recursive_clean(self, path, pattern):
        """generic recursive clean/remove method."""
        self.cmd(f'find {path} | grep -E "({pattern})" | xargs rm -rf')

    def install_name_tool(self, src, dst, mode="id"):
        """change dynamic shared library install names"""
        _cmd = f"install_name_tool -{mode} {src} {dst}"
        self.log.info(_cmd)
        self.cmd(_cmd)

    def xcodebuild(self, project, target, flag=None):
        """build via xcode the given targets"""
        if not flag:
            self.cmd(
                f"xcodebuild -project targets/{project}/py-js.xcodeproj -target {target}"
            )
        else:
            _flag = f"{flag}=1"
            self.cmd(
                f"xcodebuild -project targets/{project}/py-js.xcodeproj -target {target} "
                f"GCC_PREPROCESSOR_DEFINITIONS='$GCC_PREPROCESSOR_DEFINITIONS {_flag}'"
            )

    def xbuild_targets(self, project, targets=None, flag=None):
        """build via xcode the given targets"""
        for target in targets:
            self.xcodebuild(project, target, flag)

    def clean(self):
        """shallow cleanse build"""
        for builder in self.depends_on:
            builder.clean()

    def reset(self):
        """reset (deep cleanse) build"""
        for builder in self.depends_on:
            builder.reset()

    def build(self):
        """build product"""
        for builder in self.depends_on:
            builder.build()

    def install(self):
        """install product"""
        for builder in self.depends_on:
            builder.install()


class ProjectRecipe(ABC):
    """A project-centric platform-specific container for multiple build projects.

    A recipe is analogous to a workspace in xcode
    """

    project_classes: List[Type["Project"]]

    def __init__(self, name: str = None, projects: list[Project] = None, **settings):
        self.name = name
        self.settings = Settings(**settings)
        self.projects = projects or [
            project_class() for project_class in self.project_classes
        ]

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    __repr__ = __str__

    def build(self):
        """build projects"""
        for project in self.projects:
            project.build()


class BuilderRecipe(ABC):
    """A platform-specific container for multiple builder-centric projects.

    A recipe is analogous to a workspace in xcode
    """

    def __init__(self, name: str = None, builders: list[Builder] = None, **settings):
        self.name = name
        self.settings = Settings(**settings)
        self.builders = builders

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    __repr__ = __str__

    def build(self):
        """build builders"""
        for builder in self.builders:
            builder.build()


# ------------------------------------------------------------------------------
# Concrete Base Classes


class BaseProject(Project):
    """Project to build Python from source with different variations."""

    # python-naming
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

    # project
    pyjs = root.parent.parent
    support = pyjs / "support"
    externals = pyjs / "externals"

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


class BaseBuilder(Builder):
    """Abstract class to provide builder interface and common features."""

    project_class: Type[BaseProject]
    dependencies: List[Type["BaseBuilder"]]

    # mac_dep_target = '10.14'

    def __init__(
        self,
        product: Product = None,
        project: "BaseProject" = None,
        depends_on: list["BaseBuilder"] = None,
        **settings,
    ):
        self.product = product or self.product_class()
        self.project = project or self.project_class()
        self.depends_on = depends_on or [
            klass(project=self.project) for klass in self.dependencies
        ]
        self.settings = Settings(**settings)
        self.log = logging.getLogger(self.__class__.__name__)
        self.cmd = ShellCmd(self.log)

    # -------------------------------------------------------------------------
    # Path Methods

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


# ------------------------------------------------------------------------------
# Implementation Classes


class Bzip2Product(Product):
    """Bzip2 product"""

    default_name = "bzip2"
    default_version = "1.0.8"
    url_template = "https://sourceware.org/pub/bzip2/{name}-{version}.tar.gz"
    libs_static = ["libbz2.a"]


class Bzip2Builder(BaseBuilder):
    """Bzip2 static library builder"""

    product_class = Bzip2Product
    dependencies = []

    def build(self):
        if not self.product.has_static_libs:
            self.cmd.chdir(self.src_path)
            self.cmd(f"make install PREFIX={self.prefix}")
            self.cmd.chdir(self.project.root)


class OpensslProduct(Product):
    """OpenSSL product"""

    default_name = "openssl"
    default_version = "1.1.1g"
    url_template = "https://www.openssl.org/source/{name}-{version}.tar.gz"
    libs_static = ["libssl.a", "libcrypto.a"]


class OpensslBuilder(BaseBuilder):
    """OpenSSL static library builder"""

    product_class = OpensslProduct
    dependencies = []

    def build(self):
        if not self.product.has_static_libs:
            self.cmd.chdir(self.src_path)
            self.cmd(f"./config no-shared no-tests --prefix={self.prefix}")
            self.cmd("make install_sw")
            self.cmd.chdir(self.project.root)


class XzBuilderProduct(Product):
    """Xz static product"""

    default_name = "xz"
    default_version = "5.2.5"
    url_template = "http://tukaani.org/xz/{name}-{version}.tar.gz"
    libs_static = ["libxz.a"]


class XzBuilder(BaseBuilder):
    """Xz static library builder"""

    product_class = XzBuilderProduct
    dependencies = []

    def build(self):
        if not self.product.has_static_libs:
            self.cmd.chdir(self.src_path)
            self.cmd(
                f"""MACOSX_DEPLOYMENT_TARGET={self.project.mac_dep_target} \
                ./configure --disable-shared --enable-static --prefix={self.prefix}"""
            )
            self.cmd("make && make install")
            self.cmd.chdir(self.project.root)


class PythonProject(BaseProject):
    """generic python project"""

    builder_classes = []


class PythonBuilder(BaseBuilder):
    """Generic Python from src builder."""

    project_class = PythonProject
    # setup_local = None
    # patch = None

    # ------------------------------------------------------------------------
    # python properties

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
        """install compilation product into lib"""
        self.reset()
        self.pre_process()
        self.build()
        self.post_process()

    # def install_python(self):
    #     """install python"""
    #     self.build_python_zipped()

    # def build_python_zipped(self):
    #     self.build_python()
    #     self.ziplib()
    #
    # def build_python(self):
    #     self.compile_python()
    #     self.clean_python()
    #     self.write_python_getpip()

    def install_python_pkg(self):
        """install python product as a package"""
        self.install_python()
        self.fix_python_dylib_for_pkg()

    def install_python_ext(self):
        """install python product as a max external"""
        self.install_python()
        self.fix_python_dylib_for_ext()

    def install_python(self):
        """install python"""

    # ------------------------------------------------------------------------
    # post-processing operations

    def is_valid_path(self, dep_path):
        """check if dependency path is a valid path."""
        return (
            dep_path == ""
            or dep_path.startswith("/opt/local/")
            or dep_path.startswith("/usr/local/")
            or dep_path.startswith("/User/")
        )

    # def get_deps(self, target):
    #     """get dependencies of dylibs.
    #
    #     check if they non-system (i.e. non-portable)
    #
    #     """
    #     # if not target:
    #     #     target = self.target
    #     key = os.path.basename(target)
    #     self.install_names[key] = []
    #     result = subprocess.check_output(['otool', '-L', target])
    #     entries = [
    #         line.decode('utf-8').strip() for line in result.splitlines()
    #     ]
    #     for entry in entries:
    #         match = re.match(r'\s*(\S+)\s*\(compatibility version .+\)$',
    #                          entry)
    #         if match:
    #             path = match.group(1)
    #             (dep_path, dep_filename) = os.path.split(path)
    #             if self.is_valid_path(dep_path):
    #                 if dep_path == '':
    #                     path = os.path.join('/usr/local/lib', dep_filename)
    #                 dep_path, dep_filename = os.path.split(path)
    #                 item = (path, '@rpath/' + dep_filename)
    #                 self.install_names[key].append(item)
    #                 if path not in self.deps:
    #                     self.deps.append(path)
    #                     self.get_deps(path)

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

    def clean_python_site_packages(self):
        """remove python site-packages"""
        self.cmd.remove(self.python_lib / "site-packages")

    def remove_packages(self):
        """remove list of non-critical packages"""
        self.rm_libs(
            [
                f"config-{self.product.ver}-darwin",
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

    # def remove_extensions(self):
    #     """remove extensions: not implemented"""

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
                # f'python{self.ver}{self.suffix}',
                # f'python{self.ver}-config',
            ]
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
        self.cmd.copyfile(self.python_lib / "os.py", temp_os_py)

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
        self.install_name_tool(
            f"@loader_path/../../../../support/{self.product.name}/lib/{self.product.dylib}",
            self.product.dylib,
        )
        self.cmd.chdir(self.project.root)

    def fix_python_dylib_for_ext(self):
        """redirect ref of dylib to loader in a self-contained external deployment."""
        self.cmd.chdir(self.prefix_lib)
        self.cmd.chmod(self.product.dylib)
        self.install_name_tool(f"@loader_path/{self.product.dylib}", self.product.dylib)
        self.cmd.chdir(self.project.root)


class PythonSrcBuilder(PythonBuilder):
    """Generic Python from src builder."""

    project_class = PythonProject
    version = PYTHON_VERSION_STRING
    url_template = "https://www.python.org/ftp/python/{version}/{name}-{version}.tgz"
    dependencies = [Bzip2Builder, OpensslBuilder, XzBuilder]
    setup_local = ""
    patch = None

    # ------------------------------------------------------------------------
    # python properties

    # ------------------------------------------------------------------------
    # src-level operations

    def pre_process(self):
        """pre-build operations"""
        self.cmd.chdir(self.src_path)
        self.write_setup_local()
        # self.apply_patch()
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
        self.cmd.copyfile(
            self.project.patch / self.product.ver / setup_local,
            self.src_path / "Modules" / "Setup.local",
        )

    def apply_patch(self, patch=None):
        """Apply a standard patch from the patch directory (prefixed by major.minor ver)"""
        if not any([patch, self.patch]):
            return
        if not patch:
            patch = self.patch
        self.cmd(f"patch -p1 < {self.project.patch}/{self.product.ver}/{patch}")

    def install(self):
        """install compilation product into lib"""
        self.reset()
        self.download()
        self.pre_process()
        self.build()
        self.post_process()


class FrameworkPythonProduct(Product):
    """framework python product"""

    default_name = "framework-python"
    default_version = PYTHON_VERSION_STRING
    url_template = "https://www.python.org/ftp/python/{version}/Python-{version}.tgz"
    libs_static = ["libpython3.9.a"]


class FrameworkPythonBuilder(PythonSrcBuilder):
    """builds python in a macos framework format."""

    product_class = FrameworkPythonProduct
    setup_local = "setup-shared.local"

    @property
    def prefix(self) -> Path:
        return self.project.lib / "Python.framework" / "Versions" / self.product.ver

    def reset(self):
        self.cmd.remove(self.src_path)
        self.cmd.remove(self.project.lib / "Python.framework")

    def build(self):
        for dep in self.depends_on:
            dep.build()

        self.cmd.chdir(self.src_path)
        self.cmd(
            f"""\
        ./configure MACOSX_DEPLOYMENT_TARGET={self.project.mac_dep_target} \
            --enable-framework={self.project.lib} \
            --with-openssl={self.project.lib / 'openssl'} \
            --without-doc-strings \
            --enable-ipv6 \
            --without-ensurepip \
            --with-lto \
            --enable-optimizations
        """
        )
        self.cmd("make altinstall")
        self.cmd.chdir(self.project.root)

    # def post_process(self):
    #     self.clean()
    #     self.ziplib()


class SharedPythonProduct(Product):
    """shared python product"""

    default_name = "shared-python"
    default_version = PYTHON_VERSION_STRING
    url_template = "https://www.python.org/ftp/python/{version}/Python-{version}.tgz"
    libs_static = ["libpython3.9.a"]


class SharedPythonBuilder(PythonSrcBuilder):
    """builds python in a shared format."""

    product_class = SharedPythonProduct
    setup_local = "setup-shared.local"

    @property
    def prefix(self) -> Path:
        name = f"{self.product.name.lower()}-shared"  # pylint: disable=E1101
        return self.project.lib / name

    def build(self):
        for dep in self.depends_on:
            dep.build()

        self.cmd.chdir(self.src_path)
        self.cmd(
            f"""\
        ./configure MACOSX_DEPLOYMENT_TARGET={self.project.mac_dep_target} \
            --prefix={self.prefix} \
            --enable-shared \
            --with-openssl={self.project.lib / 'openssl'} \
            --without-doc-strings \
            --enable-ipv6 \
            --without-ensurepip \
            --with-lto \
            --enable-optimizations
        """
        )
        self.cmd("make altinstall")
        self.cmd.chdir(self.project.root)


class StaticPythonProduct(Product):
    """static python product"""

    default_name = "static-python"
    default_version = PYTHON_VERSION_STRING
    url_template = "https://www.python.org/ftp/python/{version}/Python-{version}.tgz"
    libs_static = ["libpython3.9.a"]


class StaticPythonBuilder(PythonSrcBuilder):
    """builds python in a static format."""

    product_class = StaticPythonProduct
    project_class = PythonProject
    setup_local = "setup-static-min3.local"
    patch = "makesetup.patch"

    @property
    def prefix(self) -> Path:
        name = f"{self.product.name.lower()}-static"  # pylint: disable=E1101
        return self.project.lib / name

    def build(self):
        for dep in self.depends_on:
            dep.build()

        self.cmd.chdir(self.src_path)
        self.cmd(
            f"""\
        ./configure MACOSX_DEPLOYMENT_TARGET={self.project.mac_dep_target} \
            --prefix={self.prefix} \
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

    # def post_process(self):
    #     self.clean()
    #     self.ziplib()
    # self.static_lib.rename(self.prefix / self.library)


class PyJsProduct(Product):
    default_name = "Python"
    default_version = PYTHON_VERSION_STRING
    url_template = ""


class PyJsProject(PythonProject):
    """base pyjs project class"""

    builder_classes = []

    root = Path.cwd()
    pyjs = root.parent.parent
    support = pyjs / "support"

    py_version = PYTHON_VERSION_STRING
    py_ver = ".".join(py_version.split(".")[:2])
    py_name = f"python{py_ver}"

    prefix = support / py_name
    bin = prefix / "bin"
    lib = prefix / "lib" / py_name

    homebrew = (
        Path("/usr/local/opt/python3/Frameworks/Python.framework/Versions") / py_ver
    )

    homebrew_pkgs = homebrew / "lib" / py_name


class PyJsBuilder(PythonBuilder):
    project_class: Type[PyJsProject]
    dependencies: List[Type["BaseBuilder"]]

    def __init__(
        self,
        product: PyJsProduct = None,
        project: PyJsProject = None,
        depends_on: list["BaseBuilder"] = None,
        **settings,
    ):
        self.product = product or self.product_class()
        self.project = project or self.project_class()
        self.depends_on = depends_on or [
            klass(project=self.project) for klass in self.dependencies
        ]
        self.settings = Settings(**settings)
        self.log = logging.getLogger(self.__class__.__name__)
        self.cmd = ShellCmd(self.log)


class HomebrewBuilder(PyJsBuilder):
    product_class = PyJsProduct
    project_class = PyJsProject
    dependencies = []
    suffix = ""
    setup_local = None
    patch = None

    @property
    def prefix(self):
        """compiled product destination root directory."""
        return self.project.prefix

    # @property
    # def python_lib(self):
    #     """python/lib/product.major.minor: python/lib/python3.9"""
    #     return self.prefix / 'lib' / self.name_ver

    def cp_pkgs(self, pkgs):
        for pkg in pkgs:
            # self.log("copying %s", pkg)
            self.cmd(
                f"cp -rf {self.project.homebrew_pkgs}/{pkg} {self.project.lib}/{pkg}"
            )

    def rm_libs(self, names):
        """remove all named python dylib libraries"""
        for name in names:
            self.cmd.remove(self.python_lib / name)

    def remove_extensions(self):
        """remove extensions: not implemented"""

    def clean_python(self):
        """clean everything."""
        self.clean_python_pyc(self.prefix)
        self.clean_python_tests(self.python_lib)
        # self.clean_python_site_packages()
        for i in (self.python_lib / "distutils" / "command").glob("*.exe"):
            self.cmd.remove(i)
        # self.remove(self.prefix_lib / 'pkgconfig')
        # self.remove(self.prefix / 'share')

        self.remove_packages()
        self.remove_extensions()
        # self.remove_binaries()

    # def fix_python_exec(self):
    #     self.chdir(self.project.bin)
    #     self.cmd(f'install_name_tool -change {self.project.homebrew}/Python'
    #              f' @executable_path/../{self.dylib} {self.project.name}')
    #     self.chdir(self.project.root)

    def fix_python_dylib_for_pkg(self):
        self.cmd.chdir(self.project.prefix)
        self.cmd.chmod(self.product.dylib)
        # assumes python in installed in $PREFIX
        self.install_name_tool(
            f"@loader_path/../../../../support/{self.project.name}/{self.product.dylib}",
            self.product.dylib,
        )
        self.cmd.chdir(self.project.root)

    # def fix_python_dylib_for_ext_executable(self):
    #     self.chdir(self.project.prefix)
    #     self.chmod(self.dylib)
    #     # assumes cp -rf $PREFIX/* -> same directory as py extension in py.mxo
    #     self.install_name_tool(f'@loader_path/{self.dylib}', self.dylib)
    #     self.cmd(f'cp -rf {self.prefix}/* {self.project.py_external}/Contents/MacOS')
    #     self.chdir(self.project.root)

    # def fix_python_dylib_for_ext_executable_name(self):
    #     self.chdir(self.prefix)
    #     self.chmod(self.dylib)
    #     self.install_name_tool(f'@loader_path/{self.dylib}', self.dylib)
    #     self.cmd(f'mkdir -p {self.project.py_external}/Contents/MacOS/{self.project.name}')
    #     self.cmd(f'cp -rf {self.prefix}/* {self.project.py_external}/Contents/MacOS/{self.project.name}')
    #     self.chdir(self.project.root)

    def fix_python_dylib_for_ext_resources(self):
        self.cmd.chdir(self.prefix)
        self.cmd.chmod(self.product.dylib)
        self.install_name_tool(
            f"@loader_path/../Resources/{self.project.name}/{self.product.dylib}",
            self.product.dylib,
        )
        self.cmd.chdir(self.project.root)

    def cp_python_to_ext_resources(self, arg):
        self.cmd(f"mkdir -p {arg}/Contents/Resources/{self.project.name}")
        self.cmd(f"cp -rf {self.prefix}/* {arg}/Contents/Resources/{self.project.name}")

    def copy_python(self):
        self.cmd(f"mkdir -p {self.project.lib}")
        self.cmd(f"mkdir -p {self.project.bin}")
        self.cmd(
            f"cp {self.project.homebrew}/Python {self.prefix}/{self.product.dylib}"
        )
        self.cmd(f"cp -rf {self.project.homebrew_pkgs}/*.py {self.project.lib}")
        # from IPython import embed; embed(colors="neutral")
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
        self.cmd(f"cp -rf {self.project.homebrew}/include {self.prefix}/include")
        self.cmd(f"rm -rf {self.project.prefix}/lib/{self.product.dylib}")
        self.cmd(f"rm -rf {self.project.prefix}/lib/pkgconfig")
        self.cmd(
            f"cp -rf {self.project.homebrew}/Resources/Python.app/Contents/MacOS/Python"
            f" {self.project.bin}/{self.project.name}"
        )
        self.clean_python()
        self.ziplib()

    def install_homebrew_sys(self):
        self.reset_prefix()
        self.xbuild_targets("bin-homebrew-sys", targets=["py", "pyjs"])

    def install_homebrew_pkg(self):
        self.reset_prefix()
        self.copy_python()
        self.fix_python_dylib_for_pkg()
        self.xbuild_targets("bin-homebrew-pkg", targets=["py", "pyjs"])
        # MISSING: copy package to $HOME/Max 8/Packages/py

    # def install_homebrew_ext(self):
    #     self.reset_prefix()
    #     self.copy_python()
    #     # fix_python_dylib_for_ext
    #     # fix_python_dylib_for_ext_executable_name
    #     self.fix_python_dylib_for_ext_resources()
    #     self.cp_python_to_ext_resources(self.project.py_external)
    #     self.cp_python_to_ext_resources(self.project.pyjs_external)
    #     # FIXME: for some reason both don't work at the same time!!!
    #     # you have to pick one.
    #     self.xbuild_targets('bin-homebrew-ext', targets=['py', 'pyjs'])
    #     self.reset_prefix()

    def install_homebrew_ext_py(self):
        self.reset_prefix()
        self.copy_python()
        self.fix_python_dylib_for_ext_resources()
        self.cp_python_to_ext_resources(self.project.py_external)
        self.xbuild_targets("bin-homebrew-ext", targets=["py"])
        self.reset_prefix()

    def install_homebrew_ext_pyjs(self):
        self.reset_prefix()
        self.copy_python()
        self.fix_python_dylib_for_ext_resources()
        self.cp_python_to_ext_resources(self.project.pyjs_external)
        self.xbuild_targets("bin-homebrew-ext", targets=["pyjs"])
        self.reset_prefix()
