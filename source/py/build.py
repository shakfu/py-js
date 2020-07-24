#!/usr/bin/env python3

import sys
import os
import logging
from pathlib import Path
import shutil
import zipfile
from abc import ABC, abstractmethod
# import glob

IGNORE_ERRORS = False

DEBUG = True
if DEBUG:
    LOG_LEVEL = logging.DEBUG
else:
    LOG_LEVEL = logging.INFO

LOG_FORMAT = '%(relativeCreated)-4d %(levelname)-5s: %(name)-10s %(message)s'
logging.basicConfig(level=LOG_LEVEL, format=LOG_FORMAT, stream=sys.stdout)


class Project:
    root = Path.cwd()
    source = root.parent / 'source'
    support = root.parent.parent / 'support'
    scripts = root / 'scripts'
    patch = root / 'patch'
    targets =  root / 'targets'
    build = targets / 'build'
    downloads = build / 'downloads'
    src = build / 'src'
    lib = build / 'lib'


class Builder(ABC):
    name: str
    version: str
    url_template: str
    depends_on: []

    def __init__(self, project, version=None, depends_on=None):
        self.project = project or Project()
        self.version = version or self.version
        self.depends_on = ([B(project) for B in depends_on] if depends_on else
                           [B(project) for B in self.depends_on])
        self.log = logging.getLogger(self.__class__.__name__)

    def __repr__(self):
        return f"<{self.__class__.__name__} '{self.name}-{self.version}'>"

    def __iter__(self):
        for dependency in self.depends_on:
            yield dependency
            for subdependency in iter(dependency):
                yield subdependency
    @property
    def ver(self):
        return  ".".join(self.version.split('.')[:2])

    @property
    def ver_nodot(self):
        return self.ver.replace('.', '')

    @property
    def name_version(self):
        return f'{self.name}-{self.version}'

    @property
    def name_ver(self):
        return f'{self.name.lower()}{self.ver}'

    @property
    def url(self):
        return Path(self.url_template.format(name=self.name, 
                                        version=self.version))
    @property
    def name_archive(self):
        return f'{self.name_version}.tgz'

    @property
    def download_path(self):
        return self.project.downloads / self.name_archive

    @property
    def src_path(self):
        return self.project.src / self.name_version

    @property
    def lib_path(self):
        return self.prefix

    @property
    def prefix(self):
        return self.project.lib / self.name.lower()

    @property
    def prefix_lib(self):
        return self.prefix / 'lib'

    @property
    def prefix_include(self):
        return self.prefix / 'include'

    @property
    def prefix_bin(self):
        return self.prefix / 'bin'


    def libs_static_exist(self):
        for lib in self.libs_static:
            if not (self.prefix_lib / lib).exists():
                return False
        return True

    def cmd(self, shellcmd, *args, **kwargs):
        os.system(shellcmd.format(*args, **kwargs))

    def chdir(self, path):
        os.chdir(path)

    def move(self, src, dst):
        shutil.move(src, dst)

    def copytree(self, src, dst):
        shutil.copytree(src, dst)

    def copyfile(self, src, dst):
        shutil.copyfile(src, dst)

    # def zipsrc(self, zip_path, targetdir, optlevel=-1):
    #     "this is a placeholder"
    #     with zipfile.PyZipFile(zip_path, "w", optimize=optlevel) as zipfp:
    #         zipfp.writepy(targetdir)

    def remove(self, path):
        if path.is_dir():
            shutil.rmtree(path, ignore_errors=IGNORE_ERRORS)
        else:
            path.unlink(missing_ok=True)

    def reset(self):
        self.remove(self.src_path)
        self.remove(self.prefix) # aka self.prefix

    def download(self):
        "download target src"

    def build(self):
        "build target from src"


class OSXBuilder(Builder):
    mac_dep_target = '10.13'

    @property
    def dylib(self):
        return f'lib{self.name}{self.ver}.dylib'        

    def download(self):
        "download src"

        self.project.downloads.mkdir(parents=True, exist_ok=True)
        for dep in self.depends_on:
            dep.download()

        # download
        if not self.download_path.exists():
            self.log.info(f"downloading {self.download_path}")
            self.cmd(f'curl -L --fail {self.url} -o {self.download_path}')
 
        # unpack
        if not self.src_path.exists():
            self.log.info(f"unpacking {self.src_path}")
            self.cmd(f'tar -C {self.project.src} -xvf {self.download_path}')


class OpensslBuilder(OSXBuilder):
    name = 'openssl'
    version = '1.1.1g'
    url_template = 'https://www.openssl.org/source/{name}-{version}.tar.gz'
    depends_on = []
    libs_static = ['libssl.a', 'libcrypto.a']

    def build(self):
        if not self.libs_static_exist():
            self.chdir(self.src_path)
            self.cmd(f'./config no-shared no-tests --prefix={self.prefix}')
            self.cmd('make install_sw')
            self.chdir(self.project.root)

class Bzip2Builder(OSXBuilder):
    name = 'bzip2'
    version = '1.0.8'
    url_template = 'https://sourceware.org/pub/bzip2/{name}-{version}.tar.gz'
    depends_on = []
    libs_static = ['libbz2.a']

    def build(self):
        if not self.libs_static_exist():
            self.chdir(self.src_path)
            self.cmd(f'make install PREFIX={self.prefix}')
            self.chdir(self.project.root)



class XzBuilder(OSXBuilder):
    name = 'xz'
    version = '5.2.5'
    url_template = 'http://tukaani.org/xz/{name}-{version}.tar.gz'
    depends_on = []
    libs_static = ['libxz.a']

    def build(self):
        if not self.libs_static_exist():
            self.chdir(self.src_path)
            self.cmd(f"""MACOSX_DEPLOYMENT_TARGET={self.mac_dep_target} \
                ./configure --disable-shared --enable-static --prefix={self.prefix}""")
            self.cmd(f'make && make install')
            self.chdir(self.project.root)


class PythonBuilder(OSXBuilder):
    name = 'Python'
    version = '3.8.4'
    url_template = 'https://www.python.org/ftp/python/{version}/{name}-{version}.tgz'
    depends_on = [OpensslBuilder, Bzip2Builder, XzBuilder]
    suffix = ""

    def __init__(self, project=None, version=None, depends_on=None):
        super().__init__(project, version, depends_on)

    # ------------------------------------------------------------------------
    # python properties

    @property
    def static_lib(self):
        return f'lib{self.name.lower()}{self.ver}.a'

    @property
    def python_lib(self):
        return self.prefix_lib / self.name_ver

    @property
    def site_packages(self):
        return self.python_lib / 'site-packages'

    @property
    def lib_dynload(self):
        return self.python_lib / 'lib-dynload'    

    # ------------------------------------------------------------------------
    # src-level operations

    def write_setup_local(self, setupfile):
        self.copyfile(self.project.patch / setupfile, 
                  self.src_path /'Modules' / 'Setup.local')

    def patch_src(self, patchfile):
        self.cmd(f'patch -p1 < {self.project.patch}/{patchfile}')

    def install(self):
        self.download()
        self.build()
        self.clean()
        self.zib_lib()

    def install_python_pkg(self):
        self.install_python()
        self.fix_python_dylib_for_pkg()


    def install_python_ext(self):
        self.install_python()
        self.fix_python_dylib_for_ext()

    # ------------------------------------------------------------------------
    # prefix-level operations

    def recursive_clean(self, name, pattern):
        self.cmd(f'find {name} | grep -E "({pattern})" | xargs rm -rf')

    def clean_python_pyc(self, name):
        self.recursive_clean(name, r"__pycache__|\.pyc|\.pyo$")

    def clean_python_tests(self, name):
        self.recursive_clean(name, "tests|test")

    def rm_libs(self, names):
        for name in names:
            self.remove(self.python_lib / name)

    def rm_exts(self, names):
        for name in names:
            self.remove(self.python_lib / 'lib-dynload' /
                        f'{name}.cpython-{self.ver_nodot}-darwin.so')

    def rm_bins(self, names):
        for name in names:
            self.remove(self.prefix_bin / name)

    def clean_python_site_packages(self):
        self.remove(self.python_lib / 'site-packages')

    def remove_packages(self):
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

    def remove_extensions(self): pass

    def remove_binaries(self):
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

    # def move(self, src, dst):
    #     src, dst = Path(src), Path(dst)

    #     if src.is_dir() and dst.is_dir():
    #         src.replace(dst)

    #     elif src.is_file() and dst.is_file():
    #         src.replace(dst)

    #     elif src.is_file() and dst.is_dir():
    #         src.rename(dst / src.name)

    #     elif src.is_file() and not dst.exists():
    #         dst.mkdir()
    #         src.rename(dst / src.name)

    #     elif src.is_dir():
    #         src.rename(dst)

    #     else:
    #         raise NotImplementedError

    def zip_python_library(self):
        temp_lib_dynload = self.prefix_lib / 'lib-dynload'
        temp_os_py = self.prefix_lib / 'os.py'

        self.remove(self.site_packages)
        self.lib_dynload.rename(temp_lib_dynload)
        self.copyfile(self.python_lib / 'os.py', temp_os_py)

        zip_path = self.prefix_lib  / f'python{self.ver_nodot}'
        shutil.make_archive(zip_path, 'zip', self.python_lib)

        self.remove(self.python_lib)
        self.python_lib.mkdir()
        temp_lib_dynload.rename(self.lib_dynload)
        temp_os_py.rename(self.python_lib / 'os.py')
        self.site_packages.mkdir()
        (self.prefix_lib / self.static_lib).rename(self.prefix / self.static_lib)

    # def zip_python_library(self):
    #     self.remove(self.site_packages)
    #     self.move(self.python_lib / 'lib-dynload', self.prefix_lib)
    #     self.copyfile(self.python_lib / 'os.py', temp_os_py)

    #     zip_path = self.prefix_lib  / f'python{self.ver_nodot}'
    #     shutil.make_archive(zip_path, 'zip', self.python_lib)

    #     self.remove(self.python_lib)
    #     self.python_lib.mkdir()
    #     self.move(self.prefix_lib / 'lib-dynload', self.python_lib)
    #     self.move(self.prefix_lib / 'os.py', self.python_lib)
    #     self.site_packages.mkdir()
    #     self.move(self.prefix_lib / self.static_lib, self.prefix)

class StaticPythonBuilder(PythonBuilder):
    # setup_local = 'setup-static-min2.local'
    # patch_src = 'makesetup.patch'

    def build(self):
        for dep in self.depends_on:
            dep.build()

        self.chdir(self.src_path)
        self.write_setup_local('setup-static-min2.local')
        self.patch_src('makesetup.patch')
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


class SharedPythonBuilder(PythonBuilder):
    # setup_local = 'setup-shared.local'


    def build(self):
        for dep in self.depends_on:
            dep.build()

        self.chdir(self.src_path)
        # self.write_python_setup_local()
        self.write_setup_local('setup-shared.local')
        self.cmd(f"""\
        ./configure MACOSX_DEPLOYMENT_TARGET={self.mac_dep_target} \
            --prefix={self.prefix} \
            --enable-shared \
            --with-openssl={self.project.lib / 'openssl'} \
            --with-lto \
            --enable-optimizations \
            --no-warn-script-location
        """)
        self.cmd('make altinstall')
        self.chdir(self.project.root)

    def fix_python_dylib_for_pkg(self):
        self.chdir(self.prefix_lib)
        self.cmd(f'chmod 777 {self.dylib}')
        self.cmd(
            'install_name_tool -id '
            '@loader_path/../../../../support/{self.name}/lib/{self.dylib} '
            '${self.dylib}')
        self.chdir(self.root)

    def fix_python_dylib_for_ext(self):
        self.chdir(self.prefix_lib)
        self.cmd(f'chmod 777 {self.dylib}')
        self.cmd('install_name_tool -id @loader_path/{self.dylib} {self.dylib}')
        self.chdir(self.root)

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








if __name__ == '__main__':
    p = StaticPythonBuilder()
    p.reset()
    p.download()
    p.build()
    p.clean()
    p.zip_python_library()
