
#source "scripts/common.sh"



class HomebrewProject:
    """
    PREFIX=${SUPPORT}/${NAME}
    BIN=${SUPPORT}/${NAME}/bin
    LIB=${PREFIX}/lib/${NAME}
    HOMEBREW=/usr/local/opt/python3/Frameworks/Python.framework/Versions/${VERSION}
    """
    py_version_major = '3.9'
    py_version_minor = '2'
    py_semver = '3.9.2'
    py_version = py_version_major
    py_ver = '39'
    py_name = f'python{py_ver}'
    name = py_name

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

    @property
    def prefix(self):
        """compiled product destination root directory."""
        return self.support / self.name

    def cp_pkg(self, pkg):
        self.log("copying %s", pkg)
        self.cmd(f"cp -rf {self.hombrew}/lib/{self.name}/{pkg} {self.lib}/{pkg}")


def clean_python(self):
    clean_python_pyc $PREFIX
    clean_python_tests $LIB

    rm $LIB/distutils/command/*.exe

    rm_lib config-${VERSION}-darwin
    rm_lib idlelib
    rm_lib lib2to3
    rm_lib tkinter
    rm_lib turtledemo
    rm_lib turtle.py
    # rm_lib ctypes
    # rm_lib curses
    rm_lib ensurepip
    rm_lib venv

    rm_ext _tkinter
    rm_ext _ctypes

    rm_ext _multibytecodec
    rm_ext _codecs_jp
    rm_ext _codecs_hk
    rm_ext _codecs_cn
    rm_ext _codecs_kr
    rm_ext _codecs_tw
    rm_ext _codecs_iso2022
    rm_ext _curses
    rm_ext _curses_panel



def fix_python_exec(self):
    cd $BIN
    install_name_tool -change ${HOMEBREW}/Python @executable_path/../${DYLIB} ${NAME}
    cd $ROOT


def fix_python_dylib_for_pkg(self):
    cd $PREFIX
    chmod 777 ${DYLIB}
    # assumes python in installed in $PREFIX
    install_name_tool -id @loader_path/../../../../support/${NAME}/${DYLIB} ${DYLIB}
    cd $ROOT


def fix_python_dylib_for_ext_executable(self):
    cd $PREFIX
    chmod 777 ${DYLIB}
    # assumes cp -rf $PREFIX/* -> same directory as py extension in py.mxo
    install_name_tool -id @loader_path/${DYLIB} ${DYLIB}
    cp -rf $PREFIX/* $PY_EXTERNAL/Contents/MacOS
    cd $ROOT


def fix_python_dylib_for_ext_executable_name(self):
    cd $PREFIX
    chmod 777 ${DYLIB}
    install_name_tool -id @loader_path/${NAME}/${DYLIB} ${DYLIB}
    mkdir -p $PY_EXTERNAL/Contents/MacOS/${NAME}
    cp -rf $PREFIX/* $PY_EXTERNAL/Contents/MacOS/${NAME}
    cd $ROOT



# fix_python_dylib_for_ext_resources():
#   cd $PREFIX
#   chmod 777 ${DYLIB}
#   install_name_tool -id @loader_path/../Resources/${NAME}/${DYLIB} ${DYLIB}
#   mkdir -p $PY_EXTERNAL/Contents/Resources/${NAME}
#   cp -rf $PREFIX/* $PY_EXTERNAL/Contents/Resources/${NAME}
#   cd $ROOT
# }

def fix_python_dylib_for_ext_resources(self):
    cd $PREFIX
    chmod 777 ${DYLIB}
    install_name_tool -id @loader_path/../Resources/${NAME}/${DYLIB} ${DYLIB}
    cd $ROOT


def cp_python_to_ext_resources(self):
    mkdir -p $1/Contents/Resources/${NAME}
    cp -rf $PREFIX/* $1/Contents/Resources/${NAME}


def install_python(self):
    mkdir -p $LIB
    mkdir -p $BIN
    cp -rf ${HOMEBREW}/Python ${PREFIX}/${DYLIB}
    cp -rf ${HOMEBREW}/lib/${NAME}/*.py ${LIB}
    cp_pkg asyncio
    cp_pkg collections
    cp_pkg concurrent
    # cp_pkg ctypes
    # cp_pkg curses
    cp_pkg dbm
    cp_pkg distutils
    cp_pkg email
    cp_pkg encodings
    cp_pkg html
    cp_pkg http
    cp_pkg importlib
    cp_pkg json
    cp_pkg lib-dynload
    cp_pkg logging
    cp_pkg multiprocessing
    cp_pkg pydoc_data
    cp_pkg sqlite3
    cp_pkg unittest
    cp_pkg urllib
    cp_pkg wsgiref
    cp_pkg xml
    cp_pkg xmlrpc
    cp -rf ${HOMEBREW}/include ${PREFIX}/include
    rm -rf ${PREFIX}/lib/${DYLIB}
    rm -rf ${PREFIX}/lib/${DYLIB_NAME}.dylib
    rm -rf ${PREFIX}/lib/pkgconfig
    cp -rf ${HOMEBREW}/Resources/Python.app/Contents/MacOS/Python ${BIN}/$NAME
    clean_python
    zip_python_library

def install_python_pkg(self):
    install_python
    fix_python_dylib_for_pkg


def install_python_ext(self):
    install_python
    # fix_python_dylib_for_ext
    # fix_python_dylib_for_ext_executable_name
    fix_python_dylib_for_ext_resources
    cp_python_to_ext_resources $PY_EXTERNAL
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
