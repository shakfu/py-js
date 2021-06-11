#!/usr/bin/env bash

# shellcheck disable=SC1091
source "scripts/base.sh"

function remove() {
	echo "removing $1"
	rm -rf "$1"
}

function rm_lib() {
	echo "removing $1"
	rm -rf "${LIB:?}/$1"
}

function rm_ext() {
	echo "removing $LIB/lib-dynload/$1.cpython-${PYTHON_VER_NODOT}m-darwin.so"
	rm -rf "$LIB/lib-dynload/$1.cpython-${PYTHON_VER_NODOT}-darwin.so"
}

function rm_bin() {
	echo "removing $PREFIX/bin/$1"
	rm -rf "${PREFIX:?}/bin/$1"
}

function clean_python_cruft() {
	find . | grep -E "(__pycache__|\.pyc|\.pyo$)" | xargs rm -rf
	find . | grep -E "(tests|test)" | xargs rm -rf
}

function clean_python_pyc() {
	echo "removing __pycache__ .pyc/o from $1"
	find "$1" | grep -E "(__pycache__|\.pyc|\.pyo$)" | xargs rm -rf
}

function clean_python_tests() {
	echo "removing 'test' dirs from $1"
	find "$1" | grep -E "(tests|test)" | xargs rm -rf
}

function clean_python_site_packages() {
	echo "removing everything in $LIB/site-packages"
	rm -rf "$LIB/site-packages/*"
}

function zip_python_library() {
	remove "${LIB}"/site-packages
	mv "$LIB"/lib-dynload "$PREFIX"
	cp "$LIB"/os.py "$PREFIX"
	python -m zipfile -c "$PREFIX"/lib/python"${PYTHON_VER_NODOT}".zip "$LIB"/*
	remove "$LIB"
	mkdir -p "$LIB"
	mv "$PREFIX"/lib-dynload "$LIB"
	# Need to have a copy of os.py to remain in site-packages
	# or it will fail to pick up library.zip
	mv "$PREFIX"/os.py "$LIB"
	mkdir "$LIB"/site-packages
}

function reset_prefix() {
	remove "$PREFIX"
}

# -----------------------------------------------------------------------------------

PREFIX=${SUPPORT}/${PYTHON_NAME}
BIN=${PREFIX}/bin
LIB=${PREFIX}/lib/${PYTHON_NAME}

HOMEBREW=/usr/local/opt/python3/Frameworks/Python.framework/Versions/${PYTHON_VER}

function cp_pkg() {
	echo "copying $1"
	cp -rf "${HOMEBREW}"/lib/"${PYTHON_NAME}"/"$1" "${LIB}"/"$1"
}

function clean_python() {
	clean_python_pyc "$PREFIX"
	clean_python_tests "$LIB"

	rm "$LIB/distutils/command/*.exe"

	rm_lib config-"${PYTHON_VER}"-darwin
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

function fix_python_dylib_for_pkg() {
	cd "$PREFIX" || exit
	chmod 777 "${DYLIB}"
	# assumes python in installed in $PREFIX
	install_name_tool -id @loader_path/../../../../support/"${PYTHON_NAME}"/"${DYLIB}" "${DYLIB}"
	cd "$ROOT" || exit
}

function fix_python_dylib_for_ext_resources() {
	cd "$PREFIX" || exit
	chmod 777 "${DYLIB}"
	install_name_tool -id @loader_path/../Resources/"${PYTHON_NAME}"/"${DYLIB}" "${DYLIB}"
	cd "$ROOT" || exit
}

function cp_python_to_ext_resources() {
	mkdir -p "$1"/Contents/Resources/"${PYTHON_NAME}"
	cp -rf "$PREFIX"/* "$1"/Contents/Resources/"${PYTHON_NAME}"
}

function install_python() {
	mkdir -p "$LIB"
	mkdir -p "$BIN"
	cp -rf "${HOMEBREW}"/Python "${PREFIX}"/"${DYLIB}"
	cp -rf "${HOMEBREW}"/lib/"${PYTHON_NAME}"/*.py "${LIB}"
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

	cp -rf "${HOMEBREW}"/include "${PREFIX}"/include
	rm -rf "${PREFIX:?}/lib/${DYLIB}"
	rm -rf "${PREFIX:?}/lib/${DYLIB_NAME}.dylib"
	rm -rf "${PREFIX:?}/lib/pkgconfig"
	cp -rf "${HOMEBREW}"/Resources/Python.app/Contents/MacOS/Python "${BIN}"/"$PYTHON_NAME"
	clean_python
	zip_python_library
}

function install_python_pkg() {
	install_python
	fix_python_dylib_for_pkg
}

function install_python_ext() {
	install_python
	fix_python_dylib_for_ext_resources
	cp_python_to_ext_resources "$PY_EXTERNAL"
	# FIXME: for some reason both don't work at the same time!!!
	# you have to pick one.
}

if [ "$1" == "pkg" ]; then
	echo "Installing minimal homebrew python into 'support' folder of package"
	reset_prefix
	install_python_pkg

elif [ "$1" == "ext" ]; then
	echo "Installing minimal homebrew python into 'py.mxo' external"
	install_python_ext
else
	echo "No argument given. Can be 'pkg' or 'ext'"
	echo "for package or external installation respectively"
fi
