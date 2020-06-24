#!/usr/bin/env bash

source "scripts/common.sh"

VERSION_MAJOR=${PY_MAJ_VER:=3.7}
VERSION_MINOR=${PY_MIN_VER:=7}

SEMVER=${VERSION_MAJOR}.${VERSION_MINOR}
VERSION=${VERSION_MAJOR}
VER="${VERSION//./}"
NAME=python${VERSION}

ROOT=$(pwd)
SUPPORT=${ROOT}/../../support
PREFIX=${SUPPORT}/${NAME}
BIN=${SUPPORT}/${NAME}/bin
LIB=${PREFIX}/lib/${NAME}

HOMEBREW=/usr/local/Cellar/python/${SEMVER}/Frameworks/Python.framework/Versions/${VERSION}
DYLIB_NAME=libpython${VERSION}
DYLIB=${DYLIB_NAME}.dylib


clean_python_pyc() {
	echo "removing __pycache__ .pyc/o from $1"
	find $1 | grep -E "(__pycache__|\.pyc|\.pyo$)" | xargs rm -rf
}

clean_python_tests() {
	echo "removing 'test' dirs from $1"
	find $1 | grep -E "(tests|test)" | xargs rm -rf
}


remove() {
	echo "removing $1"
	rm -rf $1
}

rm_lib() {
	echo "removing $1"
	rm -rf ${LIB}/$1
}


rm_ext() {
	echo "removing $LIB/lib-dynload/$1.cpython-${VER}m-darwin.so"
	rm -rf $LIB/lib-dynload/$1.cpython-${VER}m-darwin.so
}

rm_bin() {
	echo "removing $PREFIX/bin/$1"
	rm -rf $PREFIX/bin/$1
}

cp_pkg() {
	echo "copying $1"
	cp -rf ${HOMEBREW}/lib/${NAME}/$1 ${LIB}/$1
}

reset() {
	remove $PREFIX
}

zip_python_library() {
	remove ${LIB}/site-packages
	mv $LIB/lib-dynload $PREFIX
	cp $LIB/os.py $PREFIX
	python -m zipfile -c $PREFIX/lib/python${VER}.zip $LIB/*
	remove $LIB
	mkdir -p $LIB
	mv $PREFIX/lib-dynload $LIB
	 # Need to have a copy of os.py to remain in site-packages
	 # or it will fail to pick up library.zip
	mv $PREFIX/os.py $LIB
	mkdir $LIB/site-packages
}


clean_python() {
	clean_python_pyc $PREFIX
	clean_python_tests $LIB

	rm $LIB/distutils/command/*.exe

	rm_lib config-${VERSION}m-darwin
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

}

fix_python_exec() {
	cd $BIN
	install_name_tool -change ${HOMEBREW}/Python @executable_path/../${DYLIB} ${NAME}
	cd $ROOT
}

fix_python_dylib_for_pkg() {
	cd $PREFIX
	chmod 777 ${DYLIB}
	# assumes python in installed in $PREFIX
	install_name_tool -id @loader_path/../../../../support/${NAME}/${DYLIB} ${DYLIB}
	cd $ROOT
}

fix_python_dylib_for_ext() {
	cd $PREFIX
	chmod 777 ${DYLIB}
	# assumes cp -rf $PREFIX/* -> same directory as py extension in py.mxo
	install_name_tool -id @loader_path/${DYLIB} ${DYLIB}
	cd $ROOT
}


install_python() {
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
	rm -rf ${PREFIX}/lib/${DYLIB_NAME}m.dylib
	rm -rf ${PREFIX}/lib/pkgconfig
	cp -rf ${HOMEBREW}/Resources/Python.app/Contents/MacOS/Python ${BIN}/$NAME
	clean_python
	zip_python_library
}

install_python_pkg() {
	install_python
	fix_python_dylib_for_pkg
}


install_python_ext() {
	install_python
	fix_python_dylib_for_ext
}


if [ "$1" == "pkg" ]; then
    echo "Installing minimal homebrew python into 'support' folder of package"
    reset
    install_python_pkg

elif [ "$1" == "ext" ]; then
	echo "Installing minimal homebrew python into 'py.mxo' external"
	install_python_ext
else
    echo "No argument given. Can be 'pkg' or 'ext'"
    echo "for package or external installation respectively"
fi

