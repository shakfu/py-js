"""python: a python builder and its subtypes

TODO: move PYTHON_VERSION_STRING to config.py

"""

import os
import platform
import re
import shutil
import subprocess

from ..projects import PythonProject
from .python import PythonBuilder
from .python_deps import Bzip2Builder, OpensslBuilder, XzBuilder

PYTHON_VERSION_STRING = platform.python_version()  # e.g '3.9.2'


# ----------------------------------------------------------------------------
# PYTHON SRC BUILDER

class PythonSrcBuilder(PythonBuilder):
    """Generic Python from src builder."""
    name = 'Python'
    project_class = PythonProject
    version = PYTHON_VERSION_STRING
    url_template = 'https://www.python.org/ftp/python/{version}/{name}-{version}.tgz'
    depends_on = [OpensslBuilder, Bzip2Builder, XzBuilder]
    suffix = ""
    setup_local = None
    patch = None

    def __init__(self, project=None, version=None, depends_on=None):
        if not project:
            project = self.project_class()
        super().__init__(project, version, depends_on)

        # dependency manager attributes (revise)
        self.install_names = {}
        self.deps = []
        self.dep_list = []

    # ------------------------------------------------------------------------
    # python properties


    # ------------------------------------------------------------------------
    # src-level operations

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

    # def remove_packages(self):
    #     """remove list of non-critical packages"""
    #     self.rm_libs([
    #         f'config-{self.ver}{self.suffix}-darwin',
    #         'idlelib',
    #         'lib2to3',
    #         'tkinter',
    #         'turtledemo',
    #         'turtle.py',
    #         'ctypes',
    #         'curses',
    #         'ensurepip',
    #         'venv',
    #     ])

    # def remove_extensions(self):
    #     """remove extensions"""
    #     self.rm_exts([
    #         '_tkinter',
    #         '_ctypes',
    #         '_multibytecodec',
    #         '_codecs_jp',
    #         '_codecs_hk',
    #         '_codecs_cn',
    #         '_codecs_kr',
    #         '_codecs_tw',
    #         '_codecs_iso2022',
    #         '_curses',
    #         '_curses_panel',
    #     ])


    # def remove_binaries(self):
    #     """remove list of non-critical executables"""
    #     self.rm_bins([
    #         f'2to3-{self.ver}',
    #         f'idle{self.ver}',
    #         f'easy_install-{self.ver}',
    #         f'pip{self.ver}',
    #         f'pyvenv-{self.ver}',
    #         f'pydoc{self.ver}',
    #         # f'python{self.ver}{self.suffix}',
    #         # f'python{self.ver}-config',
    #     ])

    # def clean(self):
    #     """clean everything."""
    #     self.clean_python_pyc(self.prefix)
    #     self.clean_python_tests(self.python_lib)
    #     self.clean_python_site_packages()

    #     for i in (self.python_lib / 'distutils' / 'command').glob('*.exe'):
    #         self.remove(i)

    #     self.remove(self.prefix_lib / 'pkgconfig')
    #     self.remove(self.prefix / 'share')

    #     self.remove_packages()
    #     self.remove_extensions()
    #     self.remove_binaries()


# ----------------------------------------------------------------------------
# FRAMEWORK PYTHON BUILDER

class FrameworkPythonBuilder(PythonSrcBuilder):
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

    # def post_process(self):
    #     self.clean()
    #     self.ziplib()


# ----------------------------------------------------------------------------
# SHARED PYTHON BUILDER

class SharedPythonBuilder(PythonSrcBuilder):
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


# ----------------------------------------------------------------------------
# STATIC PYTHON BUILDER

class StaticPythonBuilder(PythonSrcBuilder):
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

    def remove_extensions(self):
        """remove extensions: not implemented"""
    
    # def post_process(self):
    #     self.clean()
    #     self.ziplib()
        #self.static_lib.rename(self.prefix / self.library)
