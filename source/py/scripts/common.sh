#!/usr/bin/env bash

PYTHON_VERSION=3.8.4
OPENSSL_VERSION=1.1.1g
BZIP2_VERSION=1.0.8
XZ_VERSION=5.2.5

MAC_DEP_TARGET=10.13

# --- need not modify below except to change excluded modules

PYTHON_VER=${PYTHON_VERSION%.*} 		# 3.8
PYTHON_VER_NODOT="${PYTHON_VER//./}"	# 38 
PYTHON_NAME=python${PYTHON_VER}			# python3.8

URL_PY=https://www.python.org/ftp/python/${PYTHON_VERSION}/Python-${PYTHON_VERSION}.tgz
URL_SSL=https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz
URL_BZ2=https://sourceware.org/pub/bzip2/bzip2-${BZIP2_VERSION}.tar.gz
URL_XZ=http://tukaani.org/xz/xz-${XZ_VERSION}.tar.gz
URLS="$URL_PY $URL_SSL $URL_BZ2 $URL_XZ"

ROOT=$(pwd)
SUPPORT=${ROOT}/../../support
EXTERNALS=${ROOT}/../../externals
SOURCE=${ROOT}/../source
FRAMEWORKS=${SUPPORT}/Frameworks
SCRIPTS=${ROOT}/scripts

BUILD=${ROOT}/targets/build
BUILD_DOWNLOADS=${BUILD}/downloads

BUILD_SRC=${BUILD}/src
BUILD_SRC_PY=${BUILD_SRC}/Python-${PYTHON_VERSION}
BUILD_SRC_SSL=${BUILD_SRC}/openssl-${OPENSSL_VERSION}
BUILD_SRC_BZ2=${BUILD_SRC}/bzip2-${BZIP2_VERSION}
BUILD_SRC_XZ=${BUILD_SRC}/bzip2-${XZ_VERSION}

BUILD_LIB=${BUILD}/lib
BUILD_LIB_PY=${BUILD_LIB}/python
BUILD_LIB_SSL=${BUILD_LIB}/ssl
BUILD_LIB_BZ2=${BUILD_LIB}/bz2
BUILD_LIB_XZ=${BUILD_LIB}/xz


PREFIX=${SUPPORT}/${PYTHON_NAME}
BIN=${SUPPORT}/${PYTHON_NAME}/bin
LIB=${PREFIX}/lib/python${PYTHON_VER}


DYLIB_NAME=libpython${PYTHON_VER}
DYLIB=${DYLIB_NAME}.dylib
PY_EXTERNAL=${EXTERNALS}/py.mxo
PYJS_EXTERNAL=${EXTERNALS}/pyjs.mxo


COLOR_BOLD_YELLOW="\033[1;33m"
COLOR_BOLD_BLUE="\033[1;34m"
COLOR_BOLD_MAGENTA="\033[1;35m"
COLOR_BOLD_CYAN="\033[1;36m"
COLOR_RESET="\033[m"

function section {
	echo -e "${COLOR_BOLD_CYAN}>>> ${1} ${COLOR_RESET}"
}

function info {
    echo -e $COLOR_BOLD_YELLOW"WARNING: "$COLOR_RESET$1
}

function warn {
    echo -e $COLOR_BOLD_MAGENTA"WARNING: "$COLOR_RESET$1
}


get_url() {
	section "retrieving: $1"
	mkdir -p $BUILD_DOWNLOADS
	mkdir -p $BUILD_SRC
	archive=$(basename $1)
	if [[ $archive == *.tar.gz ]]; then
		name=$(basename -s .tar.gz $archive)
	elif [[ $archive == *.tgz ]]; then
		name=$(basename -s .tgz $archive)
	fi

	if [ ! -f $BUILD_DOWNLOADS/$archive ]; then
		curl --fail -L $1 -o $BUILD_DOWNLOADS/$archive
	else
		info "skipping existing src archive: $archive"
	fi

	if [ ! -d $BUILD_SRC/$name ]; then
		tar -C $BUILD_SRC -xvf $BUILD_DOWNLOADS/$archive
	else
		info "skipping existing src dir: $name"
	fi
}

get_all() {
	for url in $URLS; do
		get_url $url
	done;
}

remove() {
	echo "removing $1"
	rm -rf $1
}

reset() {
	#remove $BUILD
	#remove $DOWNLOADS
	#remove $BUILD_LIB
	#remove $BUILD_LIB
	echo "implementation deferred"
}

rm_lib() {
	echo "removing $1"
	rm -rf ${LIB}/$1
}

rm_ext() {
	echo "removing $LIB/lib-dynload/$1.cpython-${PYTHON_VER_NODOT}m-darwin.so"
	rm -rf $LIB/lib-dynload/$1.cpython-${PYTHON_VER_NODOT}-darwin.so
}

rm_bin() {
	echo "removing $PREFIX/bin/$1"
	rm -rf $BIN/$1
}


clean_python_cruft() {
	find . | grep -E "(__pycache__|\.pyc|\.pyo$)" | xargs rm -rf
	find . | grep -E "(tests|test)" | xargs rm -rf
}

clean_python_pyc() {
	echo "removing __pycache__ .pyc/o from $1"
	find $1 | grep -E "(__pycache__|\.pyc|\.pyo$)" | xargs rm -rf
}

clean_python_tests() {
	echo "removing 'test' dirs from $1"
	find $1 | grep -E "(tests|test)" | xargs rm -rf
}

clean_python_site_packages() {
	echo "removing everything in $LIB/site-packages"
	rm -rf $LIB/site-packages/*
}

clean_python() {
	clean_python_pyc $PREFIX
	clean_python_tests $LIB
	clean_python_site_packages

	remove $LIB/distutils/command/*.exe
	remove $PREFIX/lib/pkgconfig
	remove $PREFIX/share

	rm_lib config-${PYTHON_VER}m-darwin
	rm_lib idlelib
	rm_lib lib2to3
	rm_lib tkinter
	rm_lib turtledemo
	rm_lib turtle.py
	rm_lib ctypes
	rm_lib curses
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

	rm_bin 2to3-${PYTHON_VER}
	rm_bin idle${PYTHON_VER}
	rm_bin easy_install-${PYTHON_VER}
	rm_bin pip${PYTHON_VER}
	rm_bin python${PYTHON_VER}m
	rm_bin pyvenv-${PYTHON_VER}
	rm_bin pydoc${PYTHON_VER}
}

zip_python_library() {
	remove ${LIB}/site-packages
	mv $LIB/lib-dynload $PREFIX
	cp $LIB/os.py $PREFIX
	python -m zipfile -c $PREFIX/lib/python${PYTHON_VER_NODOT}.zip $LIB/*
	remove $LIB
	mkdir -p $LIB
	mv $PREFIX/lib-dynload $LIB
	 # Need to have a copy of os.py to remain in site-packages
	 # or it will fail to pick up library.zip
	mv $PREFIX/os.py $LIB
	mkdir $LIB/site-packages
}


build_ssl() {
	cd $BUILD_SRC_SSL
	make clean
	# ./config no-shared --prefix=$BUILD_LIB_SSL
	./config shared --prefix=$BUILD_LIB_SSL
	make install_sw
	cd $ROOT
}


write_python_minim_setup_local() {

FILE="${BUILD_PYTHON}/Modules/Setup.local"

/bin/cat <<EOM >$FILE

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
EOM

}

reset_prefix() {
	remove $PREFIX
}

compile_python() {
	cd $BUILD_PYTHON
	write_python_minim_setup_local
	make clean
	configure_python
	make altinstall
	cd $ROOT
}


build_python() {
	compile_python
	clean_python
}

build_python_zipped() {
	build_python
	zip_python_library
}


fix_python_dylib_for_pkg() {
	cd $PREFIX/lib
	chmod 777 ${DYLIB}
	# assumes python in installed in $PREFIX
	# ../../../../support/python3.7/lib/libpython3.7m.dylib
	install_name_tool -id @loader_path/../../../../support/${PYTHON_NAME}/lib/${DYLIB} ${DYLIB}
	echo "fix_python_dylib_for_pkg done"
	# otool -L ${DYLIB}
	cd $ROOT
}

fix_python_dylib_for_ext() {
	cd $PREFIX/lib
	chmod 777 ${DYLIB}
	# assumes cp -rf $PREFIX/* -> same directory as py extension in py.mxo
	install_name_tool -id @loader_path/${DYLIB} ${DYLIB}
	echo "fix_python_dylib_for_ext done"
	cd $ROOT
}


# FIXME: not complete!
fix_python_libintl() {
	#otool -L $PREFIX/lib/libpython${PYTHON_VER}.dylib
	cp /usr/local/opt/gettext/lib/libintl.8.dylib ${PREFIX}/lib
	chmod 777 ${PREFIX}/lib/libintl.8.dylib
	install_name_tool -id @executable_path/libintl.8.dylib ${PREFIX}/lib/libintl.8.dylib
	install_name_tool -change /usr/local/opt/gettext/lib/libintl.8.dylib @executable_path/libintl.8.dylib libpython${PYTHON_VER}.dylib
}


install_python() {
	echo "checking if previous build exists"
	mkdir -p $BUILD
	if [ ! -d $BUILD_PYTHON ] ; then
		echo "retrieving $BUILD_PYTHON from $URL_PYTHON"
		get_python
	fi
	if [ ! -d $BUILD_LIB_SSL ] ; then
		echo "retrieving $BUILD_LIB_SSL from $URL_OPENSSL"
		get_ssl
		build_ssl
	fi
	build_python_zipped
}

install_python_pkg() {
	install_python
	fix_python_dylib_for_pkg
}

install_python_ext() {
	install_python
	fix_python_dylib_for_ext
}

usage() {
    echo "No argument given. Can be 'pkg' or 'ext'"
    echo "for package or external installation respectively"
}
