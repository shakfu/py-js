"""builder: a builder of py-js max externals

Aims to be pure python builder without any dependencies except the standard library.

## TODO
- [ ] should check for existance of all built libs (not just static libs)
- [ ] reset should be deep, clean should be shallow
- [ ] assert builder.product_exists
- [ ] check whether it is better to remove 'exist_ok=True' param in Path.mkdir
      to raise the error.

"""
import logging
import os
import platform
import re
import shutil
import subprocess
from textwrap import dedent
from pathlib import Path
from types import SimpleNamespace

from .depend import DependencyManager

DEBUG = False
LOG_LEVEL = logging.DEBUG if DEBUG else logging.INFO
LOG_FORMAT = "%(relativeCreated)-4d %(levelname)-5s: %(name)-10s %(message)s"
PYTHON_VERSION_STRING = platform.python_version()
URL_GETPIP = "https://bootstrap.pypa.io/get-pip.py"

logging.basicConfig(format=LOG_FORMAT, level=LOG_LEVEL)


# ----------------------------------------------------------------------------
# Configuration Classes


class Project:
    """A repository for all the files, resources, and information required to
    build one or more software products.
    """

    name = "py-js"
    py_version = platform.python_version()
    py_ver = ".".join(py_version.split(".")[:2])
    py_name = f"python{py_ver}"
    
    arch = platform.machine()


    # root in this case is root assumed for build / make / scripts
    # actual project is root.parent.parent (see below)
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
        Path("/usr/local/opt/python3/Frameworks/Python.framework/Versions")
        / py_ver
    )

    homebrew_pkgs = homebrew / "lib" / py_name

    # project root here
    pyjs = root.parent.parent
    support = pyjs / "support"
    externals = pyjs / "externals"

    py_external = externals / "py.mxo"
    pyjs_external = externals / "pyjs.mxo"

    staticlib = f"libpython{py_ver}.a"
    dylib = f"libpython{py_ver}.dylib"

    # environmental vars
    HOME = os.getenv("HOME")
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


    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    def __repr__(self):
        return self.__str__()

    def __hash__(self):
        return hash((self.name, self.py_name, self.mac_dep_target))


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


# ----------------------------------------------------------------------------
# Implementation Classes


class Product:
    """A product of a builder."""

    def __init__(
        self,
        name: str,
        version: str,
        build_dir: str = None,
        libs_static: list[str] = None,
        url_template: str = None,
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
        if self.url_template:
            return Path(self.url_template.format(name=self.name, version=self.version))
        raise KeyError("url_template not providing in settings")


class Builder:
    """A Builder know how to build a single product type in a project."""

    def __init__(
        self,
        product: Product,
        project: Project = None,
        depends_on: list["Builder"] = None,
        **settings,
    ):
        self.product = product
        self.project = project or Project()
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
    def url(self) -> Path:
        """Returns url to download product as a pathlib.Path instance."""
        return self.product.url

    # -------------------------------------------------------------------------
    # Core functions

    @property
    def product_exists(self) -> bool:
        """checks if product is built"""
        return self.has_static_libs

    @property
    def has_static_libs(self) -> bool:
        """check for presence of static libs"""
        if (libs := self.product.libs_static) :
            return all((self.prefix_lib / lib).exists() for lib in libs)
        return False

    def recursive_clean(self, path, pattern):
        """generic recursive clean/remove method."""
        self.cmd(f'find {path} | grep -E "({pattern})" | xargs rm -rf')

    def install_name_tool_id(self, new_id, target):
        """change dynamic shared library install names"""
        _cmd = f"install_name_tool -id {new_id} {target}"
        self.cmd(_cmd)

    def install_name_tool_change(self, src, dst, target):
        """change dependency reference"""
        _cmd = f"install_name_tool -change {src} {dst} {target}"
        self.cmd(_cmd)

    def install_name_tool_add_rpath(self, rpath, target):
        """change dependency reference"""
        _cmd = f"install_name_tool -add_rpath {rpath} {target}"
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

    def download(self):
        """download src using curl and tar.

        curl and tar are automatically available on mac platforms.
        """
        self.project.downloads.mkdir(parents=True, exist_ok=True)
        for dep in self.depends_on:
            dep.download()

        # download
        if not self.download_path.exists():
            self.log.info("downloading %s to %s", self.url, self.download_path)
            self.cmd(f"curl -L --fail {self.url} -o {self.download_path}")

        # unpack
        if not self.src_path.exists():
            self.project.src.mkdir(parents=True, exist_ok=True)
            self.log.info("unpacking %s", self.src_path)
            self.cmd(f"tar -C {self.project.src} -xvf {self.download_path}")

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
        if not self.project.package.exists():
            self.log.info("package py-js symlink does not exists -- creating at %s", self.project.package)
            self.project.pyjs.symlink_to(self.project.package)
        else:
            self.log.info("package py-js symlink exists -- not creating")

        # self.project.package.mkdir(exist_ok=True)
        # for subdir in self.project.package_dirs:
        #     self.cmd(
        #         f"rsync -a --delete {self.project.pyjs}/{subdir} {self.project.package}"
        #     )


class Recipe:
    """A platform-specific container for multiple builder-centric projects."""

    def __init__(self, name: str = None, builders: list[Builder] = None, **settings):
        self.name = name
        self.settings = Settings(**settings)
        self.builders = builders or []

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    __repr__ = __str__

    def build(self):
        """build builders"""
        for builder in self.builders:
            builder.build()


# ------------------------------------------------------------------------------------
# Specialized Classes


class Bzip2Builder(Builder):
    """Bzip2 static library builder"""

    def build(self):
        if not self.product_exists:
            self.cmd.chdir(self.src_path)
            self.cmd(
                f"""MACOSX_DEPLOYMENT_TARGET={self.project.mac_dep_target} \
                    make install PREFIX={self.prefix}""")
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
                    --prefix={self.prefix}""")
            self.cmd(
                f"""MACOSX_DEPLOYMENT_TARGET={self.project.mac_dep_target} \
                    make install_sw""")
            self.cmd.chdir(self.project.root)


class XzBuilder(Builder):
    """Xz static library builder"""

    def build(self):
        if not self.product_exists:
            self.cmd.chdir(self.src_path)
            self.cmd(
                f"""MACOSX_DEPLOYMENT_TARGET={self.project.mac_dep_target} \
                ./configure --disable-shared --enable-static --prefix={self.prefix}"""
            )
            self.cmd(
                f"""MACOSX_DEPLOYMENT_TARGET={self.project.mac_dep_target} \
                    make && make install""")
            self.cmd.chdir(self.project.root)


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
        """install compilation product into lib"""
        self.reset()
        self.pre_process()
        self.build()
        self.post_process()

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

    def install(self):
        """install compilation product into lib"""
        self.reset()
        self.download()
        self.pre_process()
        self.build()
        self.post_process()


class FrameworkPythonBuilder(PythonSrcBuilder):
    """builds python in a macos framework format."""

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


class SharedPythonBuilder(PythonSrcBuilder):
    """builds python in a shared format."""

    setup_local = "setup-shared.local"

    def pre_process(self):
        """pre-build operations"""
        self.cmd.chdir(self.src_path)
        self.write_setup_local()
        self.apply_patch(patch="configure.patch", to_file="configure")
        self.cmd.chdir(self.project.root)

    @property
    def prefix(self) -> Path:
        return self.project.lib / self.product.build_dir

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
            --enable-optimizations \
            ac_cv_lib_intl_textdomain=no
        """
        )
        self.cmd("make altinstall")
        self.cmd.chdir(self.project.root)


class SharedPythonForExtBuilder(SharedPythonBuilder):
    """builds python in a shared format for self-contained externals."""

    setup_local = "setup-shared.local"

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
        self.fix_python_dylib_for_ext_resources()
        # self.sign()

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
                f"config-{self.product.ver}-darwin",
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
        assert dylib_path.exists()
        self.cmd.chmod(self.product.dylib)
        # both of these are equivalent (and both don't work!)
        self.install_name_tool_id(
            f"@loader_path/../../../../support/{self.product.name_ver}/lib/{self.product.dylib}",
            self.product.dylib,
        )
        self.cmd.chdir(self.project.root)

    def fix_python_exe_for_pkg(self):
        """redirect ref of pythonX to libpythonX.Y.dylib"""
        self.cmd.chdir(self.prefix_bin)
        exe = self.product.name_ver
        d = DependencyManager(exe)
        dir_to_change = d.analyze_executable()[0]
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


class StaticPythonBuilder(PythonSrcBuilder):
    """builds python in a static format."""

    setup_local = "setup-static-min3.local"

    def pre_process(self):
        """pre-build operations"""
        self.cmd.chdir(self.src_path)
        self.write_setup_local()
        self.apply_patch(patch="configure.patch", to_file="configure")
        self.cmd.chdir(self.project.root)

    @property
    def prefix(self) -> Path:
        return self.project.lib / self.product.build_dir

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


class StaticPythonFullBuilder(StaticPythonBuilder):
    setup_local = "setup-static-min4.local"
    patch = "makesetup2.patch"

    def pre_process(self):
        """pre-build operations"""
        self.cmd.chdir(self.src_path)
        self.write_setup_local()
        self.apply_patch()
        self.cmd.chdir(self.project.root)

    def post_process(self):
        """post-build operations"""
        self.clean()
        self.ziplib()
        # self.fix()
        # self.sign()

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
                # "ctypes",
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


class PyJsBuilder(PythonBuilder):
    """pyjs concrete base class"""

    @property
    def prefix(self):
        return self.project.support / self.project.py_name

    def remove_externals(self):
        """remove py and pyjs externals from the py-js/externals directory"""
        self.cmd.remove(self.project.py_external)
        self.cmd.remove(self.project.pyjs_external)

    def install(self):
        for builder in self.depends_on:
            builder.install()


class HomebrewBuilder(PyJsBuilder):
    """homebrew python builder"""

    suffix = ""
    setup_local: str = ""
    patch: str = ""

    def cp_pkgs(self, pkgs):
        """copy package dirs from homebrew python lib to target python lib"""
        for pkg in pkgs:
            self.cmd.copy(self.project.homebrew_pkgs / pkg, self.python_lib / pkg)

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
        for i in (self.python_lib / "distutils" / "command").glob("*.exe"):
            self.cmd.remove(i)

        self.remove_packages()
        self.remove_extensions()

    def fix_python_exec(self):
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
        self.cmd(f"mkdir -p {arg}/Contents/Resources/{self.product.name_ver}")
        self.cmd(
            f"cp -rf {self.prefix}/* {arg}/Contents/Resources/{self.product.name_ver}"
        )

    def copy_python(self):
        """copy python from homebrew to destination"""
        self.python_lib.mkdir(parents=True, exist_ok=True)
        self.prefix_bin.mkdir(parents=True, exist_ok=True)
        self.cmd.copy(
            self.project.homebrew / "Python", self.prefix / self.product.dylib
        )
        self.cmd(f"cp -rf {self.project.homebrew_pkgs}/*.py {self.python_lib}")
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
        self.cmd.copy(self.project.homebrew / "include", self.prefix_include)
        self.cmd.remove(self.prefix_lib / self.product.dylib)
        self.cmd.remove(self.prefix_lib / "pkgconfig")
        self.cmd.copy(
            self.project.homebrew / "Resources/Python.app/Contents/MacOS/Python",
            self.prefix_bin / self.product.name_ver,
        )
        self.clean_python()
        self.ziplib()

    def install_homebrew_sys(self):
        """build externals use local homebrew python (non-portable)"""
        # self.reset_prefix()
        self.remove_externals()
        self.xbuild_targets("bin-homebrew-sys", targets=["py", "pyjs"])

    def install_homebrew_pkg(self):
        """build externals into package use local homebrew python (portable)"""
        self.reset_prefix()
        self.copy_python()
        self.fix_python_dylib_for_pkg()
        self.fix_python_exec()
        self.xbuild_targets("bin-homebrew-pkg", targets=["py", "pyjs"])

    def install_homebrew_ext(self):
        """build external into self-contained external using local homebrew python (portable)"""
        # self.reset_prefix()
        self.copy_python()
        self.fix_python_dylib_for_ext_resources()
        self.cp_python_to_ext_resources(self.project.py_external)
        self.cp_python_to_ext_resources(self.project.pyjs_external)
        self.xbuild_targets("bin-homebrew-ext", targets=["py", "pyjs"])
        self.reset_prefix()


class StaticExtBuilder(PyJsBuilder):
    """pyjs externals from minimal statically built python"""

    @property
    def product_exists(self):
        static_lib = self.project.lib / 'python-static' / 'lib' / self.project.staticlib
        if not static_lib.exists():
            self.log.warning("static python is not built: %s", static_lib)
        return static_lib.exists()

    def build(self):
        """builds externals from statically built python"""
        if self.product_exists:
            self.xbuild_targets("static-ext", targets=["py", "pyjs"])


class StaticExtFullBuilder(StaticExtBuilder):
    """pyjs externals from fully-loaded statically built python"""

    def build(self):
        """builds externals from statically built python"""
        if self.product_exists:
            self.xbuild_targets("static-ext-full", targets=["py", "pyjs"])


class SharedExtBuilder(PyJsBuilder):
    """pyjs externals from minimal statically built python"""

    @property
    def product_exists(self):
        shared_lib = self.project.lib / 'python-shared' / 'lib' / self.project.dylib
        if not shared_lib.exists():
            self.log.warning("shared python is not built: %s", shared_lib)
        return shared_lib.exists()

    def build(self):
        """builds externals from shared python"""
        if self.product_exists:
            self.xbuild_targets("shared-ext", targets=["py", "pyjs"])

class SharedPkgBuilder(PyJsBuilder):
    """pyjs externals in a package from minimal statically built python"""

    @property
    def product_exists(self):
        shared_lib = self.project.lib / 'python-shared' / 'lib' / self.project.dylib
        if not shared_lib.exists():
            self.log.warning("shared python is not built: %s", shared_lib)
        return shared_lib.exists()

    def build(self):
        """builds externals from shared python"""
        src = self.project.lib / 'python-shared'
        dst = f"{self.project.support}/{self.product.name_ver}"
        self.cmd(f"rm -rf {dst}") # try to remove if it exists
        self.cmd(f"cp -af {src} {dst}")
        # self.xbuild_targets("src-shared-pkg", targets=["py", "pyjs"])
        if self.product_exists:
            self.xbuild_targets("shared-pkg", targets=["py", "pyjs"])

