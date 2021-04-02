"""pyjs externals builder

"""

import logging
import os
import platform
import shutil

# import sys
from pathlib import Path
from typing import List


IGNORE_ERRORS = False
PYTHON_VERSION_STRING = platform.python_version()

# ------------------------------------------------------------------------------
# Projects


class Project:
    """Base project class"""

    root = Path.cwd()
    patch = root / "patch"
    scripts = root / "scripts"
    targets = root / "targets"

    build = targets / "build"
    src = build / "src"
    lib = build / "lib"
    downloads = build / "downloads"

    pyjs = root.parent.parent
    support = pyjs / "support"
    # frameworks = support / 'Frameworks'
    externals = pyjs / "externals"

    py_version = platform.python_version()
    py_ver = ".".join(py_version.split(".")[:2])
    py_name = f"python{py_ver}"
    name = py_name

    dylib = f"libpython_{py_ver}.dylib"

    py_external = externals / "py.mxo"
    pyjs_external = externals / "pyjs.mxo"

    HOME = os.getenv("HOME")
    PKG_NAME = "py"
    PACKAGE = f"{HOME}/Documents/Max 8/Packages/{PKG_NAME}"
    PKG_DIRS = [
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

    # @property
    # def prefix(self) -> Path:
    #     """compiled product destination root directory."""


class PythonProject(Project):
    "put something here"


class HomebrewProject(Project):
    """base homebrew project class"""

    root = Path.cwd()
    pyjs = root.parent.parent
    support = pyjs / "support"

    py_version = platform.python_version()
    py_ver = ".".join(py_version.split(".")[:2])
    py_name = f"python{py_ver}"

    prefix = support / py_name
    bin = prefix / "bin"
    lib = prefix / "lib" / py_name

    homebrew = Path("/usr/local/opt/python3/Frameworks/Python.framework/Versions") / py_ver

    homebrew_pkgs  = homebrew / 'lib' / py_name
    


# ------------------------------------------------------------------------------
# Builders


class Builder:
    """Abstract class to provide builder interface and common features."""

    name: str
    version: str
    url_template: str
    depends_on: list["Builder"]
    libs_static: list[str]
    project_class = Project
    mac_dep_target = "10.14"
    targets = ["py", "pyjs"]

    def __init__(self, project=None, version=None, depends_on=None):
        self.project = project or self.project_class()
        self.version = version or self.version
        self.depends_on = depends_on or self.depends_on
        self.log = logging.getLogger(self.__class__.__name__)

    def __repr__(self):
        return f"<{self.__class__.__name__} '{self.name}-{self.version}'>"

    # -------------------------------------------------------------------------
    # Name / Version Methods

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

    # -------------------------------------------------------------------------
    # Path Methods

    @property
    def prefix(self) -> Path:
        """compiled product destination root directory."""
        return self.project.lib / self.name.lower()

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
    def python_lib(self):
        """python/lib/product.major.minor: python/lib/python3.9"""
        return self.prefix_lib / self.name_ver

    @property
    def site_packages(self):
        """path to 'site-packages'"""
        return self.python_lib / "site-packages"

    @property
    def lib_dynload(self):
        """path to 'lib-dynload'"""
        return self.python_lib / "lib-dynload"

    @property
    def download_path(self) -> Path:
        """Returns path to downloaded product-version archive."""
        return self.project.downloads / self.name_archive

    @property
    def src_path(self) -> Path:
        """Return product source directory."""
        return self.project.src / self.name_version

    @property
    def lib_path(self) -> Path:
        """alias to self.prefix"""
        return self.prefix

    @property
    def url(self) -> Path:
        """Returns url to download product as a pathlib.Path instance."""
        return Path(self.url_template.format(name=self.name, version=self.version))

    # -------------------------------------------------------------------------
    # Test Methods

    @property
    def static_lib(self):
        """Name of static library: libpython.3.9.a"""
        return f"lib{self.name.lower()}{self.ver}.a"  # pylint: disable=E1101

    def libs_static_exist(self) -> bool:
        """tests for existance of all provided static libs"""
        return all((self.prefix_lib / lib).exists() for lib in self.libs_static)

    # -------------------------------------------------------------------------
    # Generic Shell Methods

    def cmd(self, shellcmd, *args, **kwargs):
        """Run shell command with args and keywords"""
        _cmd = shellcmd.format(*args, **kwargs)
        self.log.info(_cmd)
        os.system(_cmd)

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
            shutil.rmtree(path, ignore_errors=IGNORE_ERRORS)
        else:
            self.log.info("remove file: %s", path)
            path.unlink(missing_ok=True)

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
        if not targets:
            targets = self.targets
        for target in targets:
            self.xcodebuild(project, target, flag)

    # def deploy(self):
    #     """deploy to package"""
    #     self.cmd(f'mkdir -p {self.package}')
    #     for subdir in self.subdirs:
    #         self.cmd(f'rsync -a --delete {self.pyjs_rootdir}/{subdir} {self.package}')

    # -------------------------------------------------------------------------
    # Core Methods

    def reset_prefix(self):
        """remove prefix or compilation destinations"""
        self.remove(self.prefix)

    def reset(self):
        """remove product src directory and compiled product directory."""
        self.remove(self.src_path)
        self.remove(self.prefix)  # aka self.prefix

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


class PythonBuilder(Builder):
    """Generic Python from src builder."""

    name = "Python"
    project_class = PythonProject
    version = platform.python_version()  # e.g '3.9.2'
    depends_on = []
    suffix = ""
    setup_local = None
    patch = None

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

    def clean_python_pyc(self, path):
        """remove python .pyc files."""
        self.recursive_clean(path, r"__pycache__|\.pyc|\.pyo$")

    def clean_python_tests(self, path):
        """remove python tests files."""
        self.recursive_clean(path, "tests|test")

    def rm_libs(self, names):
        """remove all named python dylib libraries"""
        for name in names:
            self.remove(self.python_lib / name)

    def rm_exts(self, names):
        """remove all named extensions"""
        for name in names:
            self.remove(
                self.python_lib
                / "lib-dynload"
                / f"{name}.cpython-{self.ver_nodot}-darwin.so"
            )

    def rm_bins(self, names):
        """remove all named binary executables"""
        for name in names:
            self.remove(self.prefix_bin / name)

    def clean_python_site_packages(self):
        """remove python site-packages"""
        self.remove(self.python_lib / "site-packages")

    def remove_packages(self):
        """remove list of non-critical packages"""
        self.rm_libs(
            [
                f"config-{self.ver}{self.suffix}-darwin",
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
        self.rm_bins(
            [
                f"2to3-{self.ver}",
                f"idle{self.ver}",
                f"easy_install-{self.ver}",
                f"pip{self.ver}",
                f"pyvenv-{self.ver}",
                f"pydoc{self.ver}",
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
            self.remove(i)

        self.remove(self.prefix_lib / "pkgconfig")
        self.remove(self.prefix / "share")

        self.remove_packages()
        self.remove_extensions()
        self.remove_binaries()

    def ziplib(self):
        """zip python package in site-packages in .zip archive"""
        temp_lib_dynload = self.prefix_lib / "lib-dynload"
        temp_os_py = self.prefix_lib / "os.py"

        self.remove(self.site_packages)
        self.lib_dynload.rename(temp_lib_dynload)
        self.copyfile(self.python_lib / "os.py", temp_os_py)

        zip_path = self.prefix_lib / f"python{self.ver_nodot}"
        shutil.make_archive(str(zip_path), "zip", str(self.python_lib))

        self.remove(self.python_lib)
        self.python_lib.mkdir()
        temp_lib_dynload.rename(self.lib_dynload)
        temp_os_py.rename(self.python_lib / "os.py")
        self.site_packages.mkdir()

    def fix_python_dylib_for_pkg(self):
        self.chdir(self.prefix_lib)
        self.chmod(self.dylib)
        self.install_name_tool(
            f"@loader_path/../../../../support/{self.name}/lib/{self.dylib}", self.dylib
        )
        self.chdir(self.project.root)

    def fix_python_dylib_for_ext(self):
        self.chdir(self.prefix_lib)
        self.chmod(self.dylib)
        self.install_name_tool(f"@loader_path/{self.dylib}", self.dylib)
        self.chdir(self.project.root)


class Bzip2Builder(Builder):
    """Bzip2 static library builder"""

    name = "bzip2"
    version = "1.0.8"
    url_template = "https://sourceware.org/pub/bzip2/{name}-{version}.tar.gz"
    depends_on = []
    libs_static = ["libbz2.a"]

    def build(self):
        if not self.libs_static_exist():
            self.chdir(self.src_path)
            self.cmd(f"make install PREFIX={self.prefix}")
            self.chdir(self.project.root)


class OpensslBuilder(Builder):
    """OpenSSL static library builder"""

    name = "openssl"
    version = "1.1.1g"
    url_template = "https://www.openssl.org/source/{name}-{version}.tar.gz"
    depends_on = []
    libs_static = ["libssl.a", "libcrypto.a"]

    def build(self):
        if not self.libs_static_exist():
            self.chdir(self.src_path)
            self.cmd(f"./config no-shared no-tests --prefix={self.prefix}")
            self.cmd("make install_sw")
            self.chdir(self.project.root)


class XzBuilder(Builder):
    """Xz static library builder"""

    name = "xz"
    version = "5.2.5"
    url_template = "http://tukaani.org/xz/{name}-{version}.tar.gz"
    depends_on = []
    libs_static = ["libxz.a"]

    def build(self):
        if not self.libs_static_exist():
            self.chdir(self.src_path)
            self.cmd(
                f"""MACOSX_DEPLOYMENT_TARGET={self.mac_dep_target} \
                ./configure --disable-shared --enable-static --prefix={self.prefix}"""
            )
            self.cmd("make && make install")
            self.chdir(self.project.root)


class PythonSrcBuilder(PythonBuilder):
    """Generic Python from src builder."""

    name = "Python"
    project_class = PythonProject
    version = PYTHON_VERSION_STRING
    url_template = "https://www.python.org/ftp/python/{version}/{name}-{version}.tgz"
    depends_on = [OpensslBuilder, Bzip2Builder, XzBuilder]
    suffix = ""
    setup_local = ""
    patch = None

    def pre_process(self):
        """pre-build operations"""
        self.chdir(self.src_path)
        self.write_setup_local()
        # self.apply_patch()
        self.chdir(self.project.root)

    def post_process(self):
        """post-build operations"""
        self.clean()
        self.ziplib()
        # self.fix()
        # self.sign()

    def write_setup_local(self, setup_local: str = None):
        """Write to Setup.local file for cusom compilations of python builtins."""
        if not any([setup_local, self.setup_local]):
            return
        if not setup_local:
            setup_local = self.setup_local
        self.copyfile(
            self.project.patch / self.ver / setup_local,
            self.src_path / "Modules" / "Setup.local",
        )

    def apply_patch(self, patch=None):
        """Apply a standard patch from the patch directory (prefixed by major.minor ver)"""
        if not any([patch, self.patch]):
            return
        if not patch:
            patch = self.patch
        self.cmd(f"patch -p1 < {self.project.patch}/{self.ver}/{patch}")

    def install(self):
        """install compilation product into lib"""
        self.reset()
        self.download()
        self.pre_process()
        self.build()
        self.post_process()


class FrameworkPythonBuilder(PythonSrcBuilder):
    setup_local = "setup-shared.local"

    @property
    def prefix(self):
        return self.project.lib / "Python.framework" / "Versions" / self.ver

    def reset(self):
        self.remove(self.src_path)
        self.remove(self.project.lib / "Python.framework")

    def build(self):
        for dep in self.depends_on:
            dep.build()

        self.chdir(self.src_path)
        self.cmd(
            f"""\
        ./configure MACOSX_DEPLOYMENT_TARGET={self.mac_dep_target} \
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
        self.chdir(self.project.root)

    # def post_process(self):
    #     self.clean()
    #     self.ziplib()


class SharedPythonBuilder(PythonSrcBuilder):
    setup_local = "setup-shared.local"

    @property
    def prefix(self):
        name = f"{self.name.lower()}-shared"  # pylint: disable=E1101
        return self.project.lib / name

    def build(self):
        for dep in self.depends_on:
            dep.build()

        self.chdir(self.src_path)
        self.cmd(
            f"""\
        ./configure MACOSX_DEPLOYMENT_TARGET={self.mac_dep_target} \
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
        self.chdir(self.project.root)


class HomebrewBuilder(PythonBuilder):
    """A Python Builder using Homebrew"""

    name = "python"
    project_class = HomebrewProject
    version = platform.python_version()
    depends_on = []
    suffix = ""
    setup_local = None
    patch = None

    def __init__(self, project=None, version=None, depends_on=None):
        self.project = project or self.project_class()
        self.version = version or self.version
        self.depends_on = depends_on or self.depends_on
        self.log = logging.getLogger(self.__class__.__name__)


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
            self.remove(self.python_lib / name)

    def remove_extensions(self):
        """remove extensions: not implemented"""

    def clean_python(self):
        """clean everything."""
        self.clean_python_pyc(self.prefix)
        self.clean_python_tests(self.python_lib)
        # self.clean_python_site_packages()
        for i in (self.python_lib / "distutils" / "command").glob("*.exe"):
            self.remove(i)
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
        self.chdir(self.project.prefix)
        self.chmod(self.dylib)
        # assumes python in installed in $PREFIX
        self.install_name_tool(
            f"@loader_path/../../../../support/{self.project.name}/{self.dylib}",
            self.dylib,
        )
        self.chdir(self.project.root)

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
        self.chdir(self.prefix)
        self.chmod(self.dylib)
        self.install_name_tool(
            f"@loader_path/../Resources/{self.project.name}/{self.dylib}", self.dylib
        )
        self.chdir(self.project.root)

    def cp_python_to_ext_resources(self, arg):
        self.cmd(f"mkdir -p {arg}/Contents/Resources/{self.project.name}")
        self.cmd(f"cp -rf {self.prefix}/* {arg}/Contents/Resources/{self.project.name}")

    def copy_python(self):
        self.cmd(f"mkdir -p {self.project.lib}")
        self.cmd(f"mkdir -p {self.project.bin}")
        self.cmd(f"cp {self.project.homebrew}/Python {self.prefix}/{self.dylib}")
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
        self.cmd(f"rm -rf {self.project.prefix}/lib/{self.dylib}")
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


class StaticPythonBuilder(PythonSrcBuilder):
    setup_local = "setup-static-min3.local"
    patch = "makesetup.patch"

    @property
    def prefix(self):
        name = f"{self.name.lower()}-static"  # pylint: disable=E1101
        return self.project.lib / name

    def build(self):
        for dep in self.depends_on:
            dep.build()

        self.chdir(self.src_path)
        self.cmd(
            f"""\
        ./configure MACOSX_DEPLOYMENT_TARGET={self.mac_dep_target} \
            --prefix={self.prefix} \
            --without-doc-strings \
            --enable-ipv6 \
            --without-ensurepip \
            --with-lto \
            --enable-optimizations
        """
        )
        self.cmd("make altinstall")
        self.chdir(self.project.root)

    def remove_extensions(self):
        """remove extensions: not implemented"""

    # def post_process(self):
    #     self.clean()
    #     self.ziplib()
    # self.static_lib.rename(self.prefix / self.library)


builders = [
    # Builder,
    PythonBuilder,
    Bzip2Builder,
    OpensslBuilder,
    XzBuilder,
    PythonSrcBuilder,
    FrameworkPythonBuilder,
    SharedPythonBuilder,
    StaticPythonBuilder,
    HomebrewBuilder,
]

for builder_class in builders:
    print(builder_class, "->", "prefix: ", builder_class().prefix)
