"""python: contains PythonBuilderm a generic mixin for python related builders

Not meant to be directly.

"""
# import os
import platform
# import re
import shutil

from ..projects import PythonProject
from .abstract import Builder

# import subprocess



class PythonBuilder(Builder):
    """Generic Python from src builder."""
    name = 'Python'
    project_class = PythonProject
    version = platform.python_version()  # e.g '3.9.2'
    depends_on = []
    suffix = ""
    setup_local = None
    patch = None

    # ------------------------------------------------------------------------
    # python properties

    @property
    def static_lib(self):
        """Name of static library: libpython.3.9.a"""
        return f'lib{self.name.lower()}{self.ver}.a'  # pylint: disable=E1101

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
        return (dep_path == '' or dep_path.startswith('/opt/local/')
                or dep_path.startswith('/usr/local/')
                or dep_path.startswith('/User/'))

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

    # def remove_extensions(self):
    #     """remove extensions: not implemented"""

    def remove_extensions(self):
        """remove extensions"""
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
        shutil.make_archive(zip_path, 'zip', str(self.python_lib))

        self.remove(self.python_lib)
        self.python_lib.mkdir()
        temp_lib_dynload.rename(self.lib_dynload)
        temp_os_py.rename(self.python_lib / 'os.py')
        self.site_packages.mkdir()

    def fix_python_dylib_for_pkg(self):
        self.chdir(self.prefix_lib)
        self.chmod(self.dylib)
        self.install_name_tool(
            f'@loader_path/../../../../support/{self.name}/lib/{self.dylib}',
            self.dylib)
        self.chdir(self.project.root)

    def fix_python_dylib_for_ext(self):
        self.chdir(self.prefix_lib)
        self.chmod(self.dylib)
        self.install_name_tool(f'@loader_path/{self.dylib}', self.dylib)
        self.chdir(self.project.root)
