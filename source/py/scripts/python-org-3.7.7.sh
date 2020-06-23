#!/usr/bin/env bash
# build_python.sh

# WITHOUT SSL (STILL TBD)


SHORTNAME=xpython
VERSION_MAJOR=3.7
VERSION_MINOR=7
MAC_DEP_TARGET=10.13
SSL_VERSION=openssl-1.1.1g

# --- need not modify below except to change excluded modules

SEMVER=${VERSION_MAJOR}.${VERSION_MINOR}
VERSION=${VERSION_MAJOR}
VER="${VERSION//./}"
NAME=${SHORTNAME}-${VERSION}

URL_PYTHON=https://www.python.org/ftp/python/${SEMVER}/Python-${SEMVER}.tgz
URL_OPENSSL=https://www.openssl.org/source/${SSL_VERSION}.tar.gz
URL_GETPIP=https://bootstrap.pypa.io/get-pip.py

ROOT=$(pwd)
SUPPORT=${ROOT}/support
SRC=${ROOT}/source
PY=${SRC}/py
BUILD=${PY}/build
PYTHON=${BUILD}/Python-${SEMVER}
PREFIX=${SUPPORT}/${NAME}
BIN=${SUPPORT}/${NAME}/bin
LIB=${PREFIX}/lib/python${VERSION}
SSL_SRC=${BUILD}/${SSL_VERSION}
SSL=${BUILD}/ssl
TMP=${BUILD}/_tmp


get_url() {
	mkdir $TMP
	fname=$(basename $1)
	curl $1 -o $TMP/$fname
	tar -C $BUILD -xvf $TMP/$fname
	rm -rf $TMP
}

get_python() {
	get_url $URL_PYTHON
}

get_ssl() {
	get_url $URL_OPENSSL
}

reset_python() {
	remove $PYTHON
}

reset_support() {
	remove $PREFIX
}

reset_ssl() {
	remove $SSL_SRC
	remove $SSL
}

reset() {
	reset_python
	reset_support
	reset_ssl
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


clean_python_pyc() {
	echo "removing __pycache__ .pyc/o from $1"
	find $1 | grep -E "(__pycache__|\.pyc|\.pyo$)" | xargs rm -rf
}

clean_python_tests() {
	echo "removing 'test' dirs from $1"
	find $1 | grep -E "(tests|test)" | xargs rm -rf
}

clean_python() {
	clean_python_pyc $PREFIX
	clean_python_tests $LIB
	clean_python_site_packages

	remove $LIB/distutils/command/*.exe
	remove $PREFIX/lib/pkgconfig
	remove $PREFIX/share

	rm_lib config-${VERSION}m-darwin
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

	rm_bin 2to3-${VERSION}
	rm_bin idle${VERSION}
	rm_bin easy_install-${VERSION}
	rm_bin pip${VERSION}
	rm_bin python${VERSION}m
	rm_bin pyvenv-${VERSION}
}

clean_python_site_packages() {
	echo "removing everything in $LIB/site-packages"
	rm -rf $LIB/site-packages/*
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


build_ssl() {
	cd $SSL_SRC

	./config no-shared --prefix=$SSL
	# ./config shared --prefix=$SSL
	make install_sw

	cd $ROOT
}


write_python_minim_setup_local() {

FILE="${PYTHON}/Modules/Setup.local"

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

write_python_getpip() {

FILE="${PREFIX}/bin/get_pip.sh"
/bin/cat <<EOM >$FILE
curl ${URL_GETPIP} -s -o get-pip.py 
./bin/python3.7 get-pip.py
rm get-pip.py
EOM
chmod +x get_pip.sh
}

compile_python_from_source() {

	cd $PYTHON

	write_python_minim_setup_local

	./configure MACOSX_DEPLOYMENT_TARGET=${MAC_DEP_TARGET} \
	 	--prefix=$PREFIX \
	 	--enable-shared \
	 	--with-openssl=$SSL \
	 	--with-lto \
	 	--enable-optimizations
	make altinstall

	cd $ROOT

}

build_python() {
	
	compile_python_from_source

	clean_python

	write_python_getpip
}

build_python_zipped() {

	build_python

	zip_python_library
}

fix() {
	otool -L $PREFIX/lib/libpython${VERSION}.dylib
	cp /usr/local/opt/gettext/lib/libintl.8.dylib ${PREFIX}/LIB/
	chmod 777 ${PREFIX}/LIB/libintl.8.dylib
	install_name_tool -id @executable_path/libintl.8.dylib ${PREFIX}/LIB/libintl.8.dylib
	install_name_tool -change /usr/local/opt/gettext/lib/libintl.8.dylib @executable_path/libintl.8.dylib libpython${VERSION}.dylib
}

install_python() {
	get_python
	get_ssl
	build_ssl
	build_python_zipped
	#fix
}


