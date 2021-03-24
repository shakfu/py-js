

class PyJSProject(Project):
    """max external projects"""
    py_version_major = '3.9'
    py_version_minor = '2'
    py_semver = '3.9.2'
    py_version = py_version_major
    py_ver = '39'
    py_name = f'python{py_ver}'

    bzip2_version = '1.0.8'
    ssl_version='1.1.1g'
    mac_dep_target='10.13'

    root = pathlib.Path.cwd()
    product = 'python'
    support = root.parent.parent / 'support'
    externals = root.parent.parent / 'externals'
    source = root.parent / 'source'
    frameworks = support / 'Frameworks'
    scripts = root / 'scripts'
    build = root / 'targets' / 'build'
    prefix = support / product
    bin = prefix / 'bin'
    lib = prefix / 'lib' / py_version
    dylib = f'libpython_{py_version}.dylib'

    py_external = externals / 'py.mxo'
    pyjs_external = externals / 'pyjs.mxo'

    url_python=f'https://www.python.org/ftp/python/{py_semver}/Python-{py_semver}.tgz'
    url_openssl=f'https://www.openssl.org/source/openssl-{ssl_version}.tar.gz'
    url_getpip='https://bootstrap.pypa.io/get-pip.py'


    def reset_python(self):
        """remove python"""

    def reset_support(self):
        """remove {self.prefix}"""

    def reset(self):
        self.reset_python()
        self.reset_support()
        # self.reset_ssl()

    def remove(self, x):    
        self.cmd(f'rm -rf {x}')


    def rm_lib(self, *args):
        for arg in args:
            self.cmd(f'rm -rf {self.lib}/{arg}')


    def rm_ext(self, *args):
        for arg in args:
            print(f'{self.lib}/lib-dynload/{arg}.cpython-${self.py_ver}-darwin.so')
            self.cmd(f'rm -rf {self.lib}/lib-dynload/{arg}.cpython-${self.py_ver}-darwin.so')


    def rm_bin(self, *args):
        for arg in args:
            print(f'rm -rf {self.prefix}/bin/{arg}')
            self.cmd(f'rm -rf {self.prefix}/bin/{arg}')

    def clean_python_cruft(self):
        self.cmd('find . | grep -E "(__pycache__|\.pyc|\.pyo$)" | xargs rm -rf')
        self.cmd('find . | grep -E "(tests|test)" | xargs rm -rf')

    def clean_python_pyc(self):
        print("removing __pycache__ .pyc/o from $1")
        self.cmd('find $1 | grep -E "(__pycache__|\.pyc|\.pyo$)" | xargs rm -rf')


    def clean_python_tests(self, arg):
        print(f"removing 'test' dirs from {arg}")
        self.cmd(f'find {arg} | grep -E "(tests|test)" | xargs rm -rf')

    def clean_python_site_packages(self):
        print("removing everything in {self.lib}/site-packages")
        self.cmd('rm -rf {self.lib}/site-packages/*')

    def clean_python(self):
        self.clean_python_pyc(self.prefix)
        self.clean_python_tests(self.lib)
        self.clean_python_site_packages()

        self.remove(f'{self.lib}/distutils/command/*.exe')
        self.remove(f'{self.prefix}/lib/pkgconfig')
        self.remove(f'{self.prefix}/share')

        self.rm_lib(
            f'config-${self.py_version}m-darwin',
            'idlelib',
            'lib2to3',
            'tkinter',
            'turtledemo',
            'turtle.py',
            'ctypes',
            'curses',
            'ensurepip',
            'venv',
        )

        self.rm_ext(
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
        )

        self.rm_bin(
            f'2to3-{self.py_version}',
            f'idle{self.py_version}',
            f'easy_install-{self.py_version}',
            f'pip{self.py_version}',
            f'python{self.py_version}m',
            f'pyvenv-{self.py_version}',
            f'pydoc{self.py_version}',
        )

    def zip_python_library(self):
        self.remove(f'${self.lib}/site-packages')
        self.cmd(f'mv {self.lib}/lib-dynload {self.prefix}')
        self.cmd(f'cp {self.lib}/os.py {self.prefix}')
        self.cmd(f'python -m zipfile -c {self.prefix}/lib/python{self.py_ver}.zip {self.lib}/*')
        self.remove(self.lib)
        self.cmd(f'mkdir -p {self.lib}')
        self.cmd(f'mv {self.prefix}/lib-dynload {self.lib}')
         # Need to have a copy of os.py to remain in site-packages
         # or it will fail to pick up library.zip
        self.cmd(f'mv {self.prefix}/os.py {self.lib}')
        self.cmd(f'mkdir {self.lib}/site-packages')

    def build_ssl(self):
        self.cmd(f'cd $SSL_SRC')
        self.cmd('make clean')
        # ./config no-shared --prefix=$SSL
        self.cmd(f'./config shared --prefix={self.ssl}')
        self.cmd('make install_sw')
        self.cmd(f'cd {self.root}')

    def install_name_tool(self, old, new):
        """install_name_tool helper"""
        self.cmd(f"install_name_tool -id @executable_path/{old} {new}")

    def write_python_minim_setup_local(self):
        with open(f'{self.python}/Modules/Setup.local') as f:
            f.write(dedent("""
            *disabled*
            _ctypes
            #_sqlite3
            _tkinter 
            _curses
            _curses_panel
            #pyexpat
            #_elementtree
            xxlimited
            xxsubtype
            #unicodedata
            _multibytecodec
            _codecs_jp 
            _codecs_kr 
            _codecs_tw 
            _codecs_cn
            _codecs_hk
            _codecs_iso2022
            """))

    def write_python_getpip(self):
        with open(f'{self.prefix}/bin/get_pip.sh') as f:
                f.write(dedent("""
                curl ${URL_GETPIP} -s -o get-pip.py 
                ./bin/python3.7 get-pip.py
                rm get-pip.py
                """))

        self.cmd(f'chmod +x {self.prefix}/bin/get_pip.sh')

    def reset_prefix(self):
        self.remove(f'{self.prefix}')


    def compile_python(self):
        cd $PYTHON
        self.write_python_minim_setup_local()
        make clean
        self.configure_python()
        make altinstall
        cd $ROOT


    def build_python(self):
        self.compile_python()
        self.clean_python()
        self.write_python_getpip()


    def build_python_zipped(self):
        self.build_python()
        self.zip_python_library()



    def fix_python_dylib_for_pkg(self):
        cd {self.prefix}/lib
        chmod 777 ${DYLIB}
        # assumes python in installed in {self.prefix}
        # ../../../../support/python3.7/lib/libpython3.7m.dylib
        install_name_tool -id @loader_path/../../../../support/${NAME}/lib/${DYLIB} ${DYLIB}
        echo "fix_python_dylib_for_pkg done"
        # otool -L ${DYLIB}
        cd $ROOT


    def fix_python_dylib_for_ext(self):
        cd {self.prefix}/lib
        chmod 777 ${DYLIB}
        # assumes cp -rf {self.prefix}/* -> same directory as py extension in py.mxo
        install_name_tool -id @loader_path/${DYLIB} ${DYLIB}
        echo "fix_python_dylib_for_ext done"
        cd $ROOT


    # FIXME: not complete!
    def fix_python_libintl(self):
        #otool -L {self.prefix}/lib/libpython{self.py_version}.dylib
        cp /usr/local/opt/gettext/lib/libintl.8.dylib ${PREFIX}/lib
        chmod 777 ${PREFIX}/lib/libintl.8.dylib
        install_name_tool -id @executable_path/libintl.8.dylib ${PREFIX}/lib/libintl.8.dylib
        install_name_tool -change /usr/local/opt/gettext/lib/libintl.8.dylib @executable_path/libintl.8.dylib libpython{self.py_version}.dylib



    def install_python(self):
        echo "checking if previous build exists"
        mkdir -p $BUILD
        if [ ! -d $PYTHON ] ; then
            echo "retrieving $PYTHON from $URL_PYTHON"
            get_python
        fi
        if [ ! -d $SSL ] ; then
            echo "retrieving $SSL from $URL_OPENSSL"
            get_ssl
            build_ssl
        fi
        self.build_python_zipped()


    def install_python_pkg(self):
        self.install_python()
        self.fix_python_dylib_for_pkg()


    def install_python_ext(self):
        self.install_python()
        self.fix_python_dylib_for_ext()


    def usage(self):
        print("No argument given. Can be 'pkg' or 'ext'")
        print("for package or external installation respectively")
