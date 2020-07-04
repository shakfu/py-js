#!/usr/bin/env python3

import sys
import os
from pathlib import Path
import shutil
import zipfile
# import glob

env = os.getenv
cmd = os.system
ver = tuple(sys.version_info)  # e.g. (3, 7, 7, 'final', 0)
# major, minor, micro

VERSION_MAJOR = env('PY_MAJ_VER', '{0}.{1}'.format(*ver))
VERSION_MINOR = env('PY_MIN_VER', '{2}'.format(*ver))
SSL_VERSION = env('SSL_VER', '1.1.1g')
MAC_DEP_TARGET = env('MAC_DEP', '10.13')

# --- need not modify below except to change excluded modules


class PyBuilder:
    def __init__(self, py_version, ssl_version, mac_dep_target=None, packages=None, suffix=None):
        self.semver = py_version
        self.version = ".".join(py_version.split('.')[:2])
        self.ssl_version = ssl_version
        self.mac_dep_target = mac_dep_target if mac_dep_target else '1.13'
        self.packages = packages if packages else []
        self.suffix = suffix if suffix else ""

        # directories
        self.root = Path.cwd()

    # ------------------------------------------------------------------------
    # propertiess

    @property
    def ver(self):
        return self.version.replace('.', '')

    @property
    def name(self):
        return f'python{self.version}'

    @property
    def url_python(self):
        return f'https://www.python.org/ftp/python/{self.version}/Python-{self.version}.tgz'

    @property
    def url_ssl(self):
        return f'https://www.openssl.org/source/openssl-{self.ssl_version}.tar.gz'

    @property
    def url_getpip(self):
        return 'https://bootstrap.pypa.io/get-pip.py'

    @property
    def source(self):
        return self.root.parent / 'source'

    @property
    def support(self):
        return self.root.parent.parent / 'source'

    @property
    def targets(self):
        return self.root / 'targets'

    @property
    def target(self):
        return self.targets / 'python-org'

    @property
    def build(self):
        return self.target / 'build'

    @property
    def python(self):
        return self.build / f'Python-{self.semver}'

    @property
    def prefix(self):
        return self.support / self.name

    @property
    def bin(self):
        return self.prefix / 'bin'

    @property
    def lib(self):
        return self.prefix / 'lib' / self.name

    @property
    def ssl_src(self):
        return self.build / self.ssl_version

    @property
    def ssl(self):
        return self.build / self.ssl

    @property
    def tmp(self):
        return self.build / self.tmp

    @property
    def dylib(self):
        return f'libpython{self.version}{self.suffix}.dylib'

    # ------------------------------------------------------------------------
    # operations

    def cmd(self, shellcmd, *args, **kwargs):
        os.system(shellcmd.format(*args, **kwargs))

    def chdir(self, path):
        os.chdir(path)

    def move(self, src, dst):
        shutil.move(src, dst)

    def copy(self, src, dst):
        shutil.copytree(src, dst)

    def zipsrc(self, zip_path, targetdir, optlevel=-1):
        "this is a placeholder"
        with zipfile.PyZipFile(zip_path, "w", optimize=optlevel) as zipfp:
            zipfp.writepy(targetdir)

    def remove(self, path):
        if hasattr(path, '__iter__'):
            for f in path:
                shutil.rmtree(f)
        else:
            shutil.rmtree(path)

    def rm_libs(self, names):
        for name in names:
            self.remove(self.lib / name)

    def rm_exts(self, names):
        for name in names:
            self.remove(self.lib / 'lib-dynload' /
                        f'{name}.cpython-{self.ver}m-darwin.so')

    def rm_bins(self, names):
        for name in names:
            self.remove(self.prefix / 'bin' / name)

    def get_url(self, url):
        self.tmp.mkdir()
        fname = Path(url).name
        src_dir = self.tmp / fname
        self.cmd(f'curl {url} -o {src_dir}')
        self.cmd(f'tar -C {self.build} -xvf {src_dir}')
        self.remove(self.tmp)

    def get_python(self):
        self.get_url(self.url_python)

    def get_ssl(self):
        self.get_url(self.url_ssl)

    def reset_python(self):
        self.remove(self.python)

    def reset_support(self):
        self.remove(self.prefix)

    def reset_ssl(self):
        self.remove(self.ssl_src)
        self.remove(self.ssl)

    def reset(self):
        self.reset_python()
        self.reset_support()
        self.reset_ssl()

    def recursive_clean(self, name, pattern):
        cmd(f'find {name} | grep -E "({pattern})" | xargs rm -rf')

    def clean_python_pyc(self, name):
        self.recursive_clean(name, r"__pycache__|\.pyc|\.pyo$")

    def clean_python_tests(self, name):
        self.recursive_clean(name, "tests|test")

    def clean_python_site_packages(self):
        self.remove((self.lib / 'site-packages').glob('*'))

    def remove_packages(self):
        self.rm_libs([
            f'config-{self.version}{self.suffix}-darwin',
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
        self.rm_bins([
            f'2to3-${self.version}',
            f'idle${self.version}',
            f'easy_install-${self.version}',
            f'pip${self.version}',
            f'python${self.version}m',
            f'pyvenv-${self.version}',
        ])

    def clean_python(self):
        self.clean_python_pyc(self.prefix)
        self.clean_python_tests(self.lib)
        self.clean_python_site_packages()

        self.remove((self.lib / 'distutils' / 'command').glob('*.exe'))

        self.remove(self.prefix / 'lib' / 'pkgconfig')
        self.remove(self.prefix / 'share')

        self.remove_packages()
        self.remove_extensions()
        self.remove_binaries()

    def zip_python_library(self):
        self.remove(self.lib / 'site-packages')
        self.move(self.lib / 'lib-dynload', self.prefix)
        self.copy(self.lib / 'os.py', self.prefix)
        self.zipsrc(self.prefix / 'lib' / f'python{self.ver}.zip',
                    self.lib.glob('*'))
        self.remove(self.lib)
        self.lib.mkdir()
        self.move(self.prefix / 'lib-dynload', self.lib)
        self.move(self.lib / 'os.py', self.lib)
        (self.lib / 'site-packages').mkdir()

    def build_ssl(self):
        self.chdir(self.ssl_src)
        self.cmd(f'./config no-shared --prefix={self.ssl}')
        self.cmd('make install_sw')
        self.chdir(self.root)

    def write_python_setup_local(self):
        lines = ['*disabled*'] + [
            '_ctypes'
            #'_sqlite3'
            '_tkinter'
            '_curses'
            '_curses_panel'
            'xxlimited'
            'xxsubtype'
            '_multibytecodec'
            '_codecs_jp'
            '_codecs_kr'
            '_codecs_tw'
            '_codecs_cn'
            '_codecs_hk'
            '_codecs_iso2022'
        ]
        with open(self.python / 'Modules' / 'Setup.local', 'w') as f:
            f.writelines(lines)

    def write_python_getpip(self):
        script = f"""
            curl ${self.url_getpip} -s -o get-pip.py 
            ./bin/python{self.version} get-pip.py
        """
        self.cmd("chmod +x get_pip.sh")
        with open(self.python / 'bin' / 'get_pip.sh', 'w') as f:
            f.write(script)

    def compile_python_from_source(self):

        self.chdir(self.python)

        self.write_python_setup_local()
        self.cmd(f"""
        ./configure MACOSX_DEPLOYMENT_TARGET={self.mac_dep_target} \
            --prefix={self.prefix} \
            --enable-shared \
            --with-openssl={self.ssl} \
            --with-lto \
            --enable-optimizations
        """)
        self.cmd('make altinstall')
        self.chdir(self.root)

    def build_python(self):
        self.compile_python_from_source()
        self.clean_python()
        self.write_python_getpip()

    def build_python_zipped(self):
        self.build_python()
        self.zip_python_library()

    def fix_python_dylib_for_pkg(self):
        self.chdir(self.prefix / self.lib)
        self.cmd(f'chmod 777 {self.dylib}')
        self.cmd(
            'install_name_tool -id '
            '@loader_path/../../../../support/{self.name}/lib/{self.dylib} '
            '${self.dylib}')
        self.chdir(self.root)


    def fix_python_dylib_for_ext(self):
        self.chdir(self.prefix / self.lib)
        self.cmd(f'chmod 777 {self.dylib}')
        self.cmd('install_name_tool -id @loader_path/{self.dylib} {self.dylib}')
        self.chdir(self.root)


# FIXME: not complete!
# def fix_python_libintl(self):
#     #otool -L $PREFIX/lib/libpython${VERSION}.dylib
#     cp /usr/local/opt/gettext/lib/libintl.8.dylib ${PREFIX}/lib
#     chmod 777 ${PREFIX}/lib/libintl.8.dylib
#     install_name_tool -id @executable_path/libintl.8.dylib ${PREFIX}/lib/libintl.8.dylib
#     install_name_tool -change /usr/local/opt/gettext/lib/libintl.8.dylib @executable_path/libintl.8.dylib libpython${VERSION}.dylib


    def install_python(self):
        self.get_python()
        self.get_ssl()
        self.build_ssl()
        self.build_python_zipped()


    def install_python_pkg(self):
        self.install_python()
        self.fix_python_dylib_for_pkg()


    def install_python_ext(self):
        self.install_python()
        self.fix_python_dylib_for_ext()
        # FIXME: not complete!
        # cp python to py.mxo
