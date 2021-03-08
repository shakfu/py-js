"""python: a python builder and its subtypes

TODO: move PYTHON_VERSION_STRING to config.py

"""

import os
import platform
import re
import shutil
import subprocess

from .bzip2 import Bzip2Builder
from .openssl import OpensslBuilder
from .opsys import OSXBuilder
from .xz import XzBuilder

PYTHON_VERSION_STRING = platform.python_version()  # e.g '3.9.1'


class PythonBuilder(OSXBuilder):
    """Generic Python from src builder."""
    name = 'Python'
    version = PYTHON_VERSION_STRING
    url_template = 'https://www.python.org/ftp/python/{version}/{name}-{version}.tgz'
    depends_on = [OpensslBuilder, Bzip2Builder, XzBuilder]
    suffix = ""
    setup_local = None
    patch = None

    def __init__(self, project=None, version=None, depends_on=None):
        super().__init__(project, version, depends_on)

        # dependency manager attributes (revise)
        self.install_names = {}
        self.deps = []
        self.dep_list = []

    # ------------------------------------------------------------------------
    # python properties

    @property
    def static_lib(self):
        """Name of static library: libpython.3.9.a"""
        return f'lib{self.name.lower()}{self.ver}.a' # pylint: disable=E1101

    @property
    def python_lib(self):
        """python/lib/product.major.minor: python/lib/python3.9"""
        return self.prefix_lib / self.name_ver

    @property
    def site_packages(self):
        """path to 'site-packages'"""
        return self.python_lib / 'site-packages'

    @property
    def lib_dynload(self):
        """path to 'lib-dynload'"""
        return self.python_lib / 'lib-dynload'

    # ------------------------------------------------------------------------
    # src-level operations

    def pre_process(self):
        """pre-build operations"""
        self.chdir(self.src_path)
        self.write_setup_local()
        #self.apply_patch()
        self.chdir(self.project.root)

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
        self.copyfile(self.project.patch / self.ver / setup_local,
                      self.src_path / 'Modules' / 'Setup.local')

    def apply_patch(self, patch=None):
        """Apply a standard patch from the patch directory (prefixed by major.minor ver)"""
        if not any([patch, self.patch]):
            return
        if not patch:
            patch = self.patch
        self.cmd(f'patch -p1 < {self.project.patch}/{self.ver}/{patch}')

    def install(self):
        """install compilation product into lib"""
        self.reset()
        self.download()
        self.pre_process()
        self.build()
        self.post_process()

    # def install_python_pkg(self):
    #     self.install_python()
    #     self.fix_python_dylib_for_pkg()

    # def install_python_ext(self):
    #     self.install_python()
    #     self.fix_python_dylib_for_ext()

    # ------------------------------------------------------------------------
    # post-processing operations

    def is_valid_path(self, dep_path):
        """check if dependency path is a valid path."""
        return (dep_path == '' or dep_path.startswith('/opt/local/')
                or dep_path.startswith('/usr/local/')
                or dep_path.startswith('/User/'))

    def get_deps(self, target):
        """get dependencies of dylibs.

        check if they non-system (i.e. non-portable)

        """
        # if not target:
        #     target = self.target
        key = os.path.basename(target)
        self.install_names[key] = []
        result = subprocess.check_output(['otool', '-L', target])
        entries = [
            line.decode('utf-8').strip() for line in result.splitlines()
        ]
        for entry in entries:
            match = re.match(r'\s*(\S+)\s*\(compatibility version .+\)$',
                             entry)
            if match:
                path = match.group(1)
                (dep_path, dep_filename) = os.path.split(path)
                if self.is_valid_path(dep_path):
                    if dep_path == '':
                        path = os.path.join('/usr/local/lib', dep_filename)
                    dep_path, dep_filename = os.path.split(path)
                    item = (path, '@rpath/' + dep_filename)
                    self.install_names[key].append(item)
                    if path not in self.deps:
                        self.deps.append(path)
                        self.get_deps(path)

    def recursive_clean(self, name, pattern):
        """generic recursive clean/remove method."""
        self.cmd(f'find {name} | grep -E "({pattern})" | xargs rm -rf')

    def clean_python_pyc(self, name):
        """remove python .pyc files."""
        self.recursive_clean(name, r"__pycache__|\.pyc|\.pyo$")

    def clean_python_tests(self, name):
        """remove python tests files."""
        self.recursive_clean(name, "tests|test")

    def rm_libs(self, names):
        """remove all named python dylib libraries"""
        for name in names:
            self.remove(self.python_lib / name)

    def rm_exts(self, names):
        """remove all named extensions"""
        for name in names:
            self.remove(self.python_lib / 'lib-dynload' /
                        f'{name}.cpython-{self.ver_nodot}-darwin.so')

    def rm_bins(self, names):
        """remove all named binary executables"""
        for name in names:
            self.remove(self.prefix_bin / name)

    def clean_python_site_packages(self):
        """remove python site-packages"""
        self.remove(self.python_lib / 'site-packages')

    def remove_packages(self):
        """remove list of non-critical packages"""
        self.rm_libs([
            f'config-{self.ver}{self.suffix}-darwin',
            'idlelib',
            'lib2to3',
            'tkinter',
            'turtledemo',
            'turtle.py',
            'ctypes',
            'curses',
            'ensurepip',
            'venv',
        ])

    def remove_extensions(self):
        """remove extensions: not implemented"""
        pass

    def remove_binaries(self):
        """remove list of non-critical executables"""
        self.rm_bins([
            f'2to3-{self.ver}',
            f'idle{self.ver}',
            f'easy_install-{self.ver}',
            f'pip{self.ver}',
            f'pyvenv-{self.ver}',
            f'pydoc{self.ver}',
            # f'python{self.ver}{self.suffix}',
            # f'python{self.ver}-config',
        ])

    def clean(self):
        """clean everything."""
        self.clean_python_pyc(self.prefix)
        self.clean_python_tests(self.python_lib)
        self.clean_python_site_packages()

        for i in (self.python_lib / 'distutils' / 'command').glob('*.exe'):
            self.remove(i)

        self.remove(self.prefix_lib / 'pkgconfig')
        self.remove(self.prefix / 'share')

        self.remove_packages()
        self.remove_extensions()
        self.remove_binaries()

    def ziplib(self):
        """zip python package in site-packages in .zip archive"""
        temp_lib_dynload = self.prefix_lib / 'lib-dynload'
        temp_os_py = self.prefix_lib / 'os.py'

        self.remove(self.site_packages)
        self.lib_dynload.rename(temp_lib_dynload)
        self.copyfile(self.python_lib / 'os.py', temp_os_py)

        zip_path = self.prefix_lib / f'python{self.ver_nodot}'
        shutil.make_archive(zip_path, 'zip', self.python_lib)

        self.remove(self.python_lib)
        self.python_lib.mkdir()
        temp_lib_dynload.rename(self.lib_dynload)
        temp_os_py.rename(self.python_lib / 'os.py')
        self.site_packages.mkdir()

    def fix_python_dylib_for_pkg(self):
        self.chdir(self.prefix_lib)
        self.cmd(f'chmod 777 {self.dylib}')
        self.cmd(
            'install_name_tool -id '
            '@loader_path/../../../../support/{self.name}/lib/{self.dylib} '
            '${self.dylib}')
        self.chdir(self.project.root)

    def fix_python_dylib_for_ext(self):
        self.chdir(self.prefix_lib)
        self.cmd(f'chmod 777 {self.dylib}')
        self.cmd(
            'install_name_tool -id @loader_path/{self.dylib} {self.dylib}')
        self.chdir(self.project.root)


class StaticPythonBuilder(PythonBuilder):
    setup_local = 'setup-static-min3.local'
    patch = 'makesetup.patch'

    @property
    def prefix(self):
        name = f'{self.name.lower()}-static' # pylint: disable=E1101
        return self.project.lib / name

    def build(self):
        for dep in self.depends_on:
            dep.build()

        self.chdir(self.src_path)
        self.cmd(f"""\
        ./configure MACOSX_DEPLOYMENT_TARGET={self.mac_dep_target} \
            --prefix={self.prefix} \
            --without-doc-strings \
            --enable-ipv6 \
            --without-ensurepip \
            --with-lto \
            --enable-optimizations
        """)
        self.cmd('make altinstall')
        self.chdir(self.project.root)

    def post_process(self):
        self.clean()
        self.ziplib()
        #self.static_lib.rename(self.prefix / self.library)


class SharedPythonBuilder(PythonBuilder):
    setup_local = 'setup-shared.local'

    @property
    def prefix(self):
        name = f'{self.name.lower()}-shared' # pylint: disable=E1101
        return self.project.lib / name

    def build(self):
        for dep in self.depends_on:
            dep.build()

        self.chdir(self.src_path)
        self.cmd(f"""\
        ./configure MACOSX_DEPLOYMENT_TARGET={self.mac_dep_target} \
            --prefix={self.prefix} \
            --enable-shared \
            --with-openssl={self.project.lib / 'openssl'} \
            --without-doc-strings \
            --enable-ipv6 \
            --without-ensurepip \
            --with-lto \
            --enable-optimizations
        """)
        self.cmd('make altinstall')
        self.chdir(self.project.root)

    def remove_extensions(self):
        self.rm_exts([
            '_tkinter',
            '_ctypes',
            '_multibytecodec',
            '_codecs_jp',
            '_codecs_hk',
            '_codecs_cn',
            '_codecs_kr',
            '_codecs_tw',
            '_codecs_iso2022',
            '_curses',
            '_curses_panel',
        ])


class FrameworkPythonBuilder(PythonBuilder):
    setup_local = 'setup-shared.local'

    @property
    def prefix(self):
        return self.project.lib / 'Python.framework' / 'Versions' / self.ver

    def reset(self):
        self.remove(self.src_path)
        self.remove(self.project.lib / 'Python.framework')

    def build(self):
        for dep in self.depends_on:
            dep.build()

        self.chdir(self.src_path)
        self.cmd(f"""\
        ./configure MACOSX_DEPLOYMENT_TARGET={self.mac_dep_target} \
            --enable-framework={self.project.lib} \
            --with-openssl={self.project.lib / 'openssl'} \
            --without-doc-strings \
            --enable-ipv6 \
            --without-ensurepip \
            --with-lto \
            --enable-optimizations
        """)
        self.cmd('make altinstall')
        self.chdir(self.project.root)

    def remove_extensions(self):
        self.rm_exts([
            '_tkinter',
            '_ctypes',
            '_multibytecodec',
            '_codecs_jp',
            '_codecs_hk',
            '_codecs_cn',
            '_codecs_kr',
            '_codecs_tw',
            '_codecs_iso2022',
            '_curses',
            '_curses_panel',
        ])

    def post_process(self):
        self.clean()
        self.ziplib()
