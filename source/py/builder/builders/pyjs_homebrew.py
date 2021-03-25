"""homebrew: useing homeebrew python project


"""

import pathlib

from ..models import Project
from .python import PythonBuilder


class HomebrewProject(Project):
    """Project to build Python from source with different variations."""
    # root = pathlib.Path.cwd()
    # patch = root / 'patch'
    # targets = root / 'targets'
    # build = targets / 'build'
    # downloads = build / 'downloads'
    # src = build / 'src'
    # lib = build / 'lib'

    py_version_major = '3.9'
    py_version_minor = '2'
    py_semver = '3.9.2'
    py_version = py_version_major
    py_ver = '39'
    py_name = f'python{py_version}'
    name = py_name

    root = pathlib.Path.cwd()
    targets = root / 'targets'
    pyjs = root.parent.parent
    support = pyjs / 'support'
    externals = pyjs / 'externals'

    source = root.parent / 'source'
    frameworks = support / 'Frameworks'
    scripts = root / 'scripts'
    build = root / 'targets' / 'build'

    prefix = support / py_name    
    bin = prefix / 'bin'
    lib = prefix / 'lib' / py_name
    dylib = f'libpython_{py_version}.dylib'
    homebrew = f'/usr/local/opt/python3/Frameworks/Python.framework/Versions/{py_version}'

    py_external = externals / 'py.mxo'
    pyjs_external = externals / 'pyjs.mxo'



class HomebrewBuilder(PythonBuilder):
    """A Python Builder using Homebrew"""
    name = 'python'
    project_class = PythonProject
    version = PYTHON_VERSION_STRING
    # url_template = 'https://www.python.org/ftp/python/{version}/{name}-{version}.tgz'
    depends_on = []
    suffix = ""
    setup_local = None
    patch = None

    @property
    def prefix(self):
        """compiled product destination root directory."""
        return self.project.prefix
    
    def cp_pkg(self, pkgs):
        for pkg in pkgks:
            self.log("copying %s", pkg)
            self.cmd(f"cp -rf {self.homebrew}/lib/{self.name}/{pkg} {self.lib}/{pkg}")

    def rm_libs(self, names):
        """remove all named python dylib libraries"""
        for name in names:
            self.remove(self.python_lib / name)


    def remove_extensions(self):
        """remove extensions: not implemented"""

    def clean(self):
        """clean everything."""
        self.clean_python_pyc(self.prefix)
        self.clean_python_tests(self.python_lib)
        # self.clean_python_site_packages()

        for i in (self.python_lib / 'distutils' / 'command').glob('*.exe'):
            self.remove(i)

        # self.remove(self.prefix_lib / 'pkgconfig')
        # self.remove(self.prefix / 'share')

        self.remove_packages()
        self.remove_extensions()
        # self.remove_binaries()


    def fix_python_exec(self):
        self.chdir(self.bin)
        self.cmd(f'install_name_tool -change {self.homebrew}/Python @executable_path/../{self.dylib} {self.name}')
        self.chdir(self.root)


    def fix_python_dylib_for_pkg(self):
        self.chdir(self.prefix)
        self.chmod(self.dylib)
        # assumes python in installed in $PREFIX
        self.install_name_tool(
            f'@loader_path/../../../../support/{self.name}/{self.dylib}', self.dylib)
        self.chdir(self.root)

    def fix_python_dylib_for_ext_executable(self):
        self.chdir(self.prefix)
        self.chmod(self.dylib)
        # assumes cp -rf $PREFIX/* -> same directory as py extension in py.mxo
        self.install_name_tool(f'@loader_path/{self.dylib}', self.dylib)
        self.cmd(f'cp -rf {self.prefix}/* {self.py_external}/Contents/MacOS')
        self.chdir(self.root)

    def fix_python_dylib_for_ext_executable_name(self):
        self.chdir(self.prefix)
        self.chmod(self.dylib)
        self.install_name_tool(f'@loader_path/{self.dylib}', self.dylib)
        self.cmd(f'mkdir -p {self.py_external}/Contents/MacOS/{self.name}')
        self.cmd(f'cp -rf {self.prefix}/* {self.py_external}/Contents/MacOS/{self.name}')
        self.chdir(self.root)


    def fix_python_dylib_for_ext_resources(self):
        self.chdir(self.prefix)
        self.chmod(self.dylib)
        self.install_name_tool(f'@loader_path/../Resources/{self.name}/{self.dylib}', self.dylib)
        self.chdir(self.root)

    def cp_python_to_ext_resources(self, arg):
        self.cmd(f'mkdir -p {arg}/Contents/Resources/{self.name}')
        self.cmd(f'cp -rf {self.prefix}/* {arg}/Contents/Resources/{self.name}')


    # def install_python(self):
    def build(self):
        self.cmd(f'mkdir -p {self.lib}')
        self.cmd(f'mkdir =p {self.bin}')
        self.cmd('cp -rf ${self.homebrew}/Python ${PREFIX}/${self.dylib}')
        self.cmd('cp -rf ${self.homebrew}/lib/${self.name}/*.py ${self.lib}')
        pkgs_to_cp = [
            'asyncio',
            'collections',
            'concurrent',
            #'ctypes',
            #'curses',
            'dbm',
            'distutils',
            'email',
            'encodings',
            'html',
            'http',
            'importlib',
            'json',
            'lib-dynload',
            'logging',
            'multiprocessing',
            'pydoc_data',
            'sqlite3',
            'unittest',
            'urllib',
            'wsgiref',
            'xml',
            'xmlrpc',
        ]

        self.cmd(f'cp -rf ${self.homebrew}/include ${self.prefix}/include')
        self.cmd(f'rm -rf ${self.prefix}/lib/{self.dylib}')
        self.cmd(f'rm -rf ${self.prefix}/lib/${DYLIB_NAME}.dylib')
        self.cmd(f'rm -rf ${self.prefix}/lib/pkgconfig')
        self.cmd(f'cp -rf ${self.homebrew}/Resources/Python.app/Contents/MacOS/Python {self.bin}/{self.name}')
        self.clean_python()
        self.zip_python_library()

    def install_python_pkg(self):
        self.install_python()
        self.fix_python_dylib_for_pkg()


    def install_python_ext(self):
        self.install_python()
        # fix_python_dylib_for_ext
        # fix_python_dylib_for_ext_executable_name
        self.fix_python_dylib_for_ext_resources()
        self.cp_python_to_ext_resources(self.py_external)
        # FIXME: for some reason both don't work at the same time!!!
        # you have to pick one.
        # cp_python_to_ext_resources $PYJS_EXTERNAL


# if [ "$1" == "pkg" ]; then
#     echo "Installing minimal homebrew python into 'support' folder of package"
#     reset_prefix
#     install_python_pkg

# elif [ "$1" == "ext" ]; then
#     echo "Installing minimal homebrew python into 'py.mxo' external"
#     install_python_ext
# else
#     echo "No argument given. Can be 'pkg' or 'ext'"
#     echo "for package or external installation respectively"
# fi
