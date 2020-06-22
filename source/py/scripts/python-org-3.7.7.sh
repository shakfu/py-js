#!/usr/bin/env bash
# build_python.sh

# WITHOUT SSL (STILL TBD)


SHORTNAME=xpython
VERSION_MAJOR=3.7
VERSION_MINOR=7
MAC_DEP_TARGET=10.13

# --- do not modify below

SEMVER=${VERSION_MAJOR}.${VERSION_MINOR}
VERSION=${VERSION_MAJOR}
VER="${VERSION//./}"
NAME=${SHORTNAME}-${VERSION}

URL_PYTHON=https://www.python.org/ftp/python/${SEMVER}/Python-${SEMVER}.tgz
URL_OPENSSL=https://www.openssl.org/source/openssl-1.1.1g.tar.gz

ROOT=$(pwd)
SUPPORT=${ROOT}/support
SRC=${ROOT}/source
PY=${SRC}/py
BUILD=${PY}/build
PYTHON=${BUILD}/Python-${SEMVER}
PREFIX=${SUPPORT}/${NAME}
LIB=${PREFIX}/lib/python${VERSION}
SSL=${BUILD}/ssl

TMP=${BUILD}/_tmp
ARCHIVE=$(basename ${URL_PYTHON})

# SSL=${ROOT}/source/py/build/openssl-1.1.1g
# OPENSSL_LDFLAGS=-DUSE_SSL
# OPENSSL_LIBS="-L${SSL} -lssl -lcrypto"
# OPENSSL_INCLUDES="-I${SSL}/include -I${SSL}/include/openssl"

get_python() {
	mkdir $TMP
	wget -P $TMP $URL_PYTHON
	tar -C $BUILD -xvf $TMP/$ARCHIVE
	rm -rf $TMP
}


reset() {
	rm -rf $PYTHON
}

reset_support() {
	rm -rf $PREFIX
}

remove() {
	echo "removing $1"
	rm -rf $1
}

rm_lib() {
	echo "removing $1"
	rm -rf ${LIB}/$1
}

clean() {
	echo "removing __pycache__ .pyc/o from $1"
	find $1 | grep -E "(__pycache__|\.pyc|\.pyo$)" | xargs rm -rf
}

clean_tests() {
	echo "removing 'test' dirs from $1"
	find $1 | grep -E "(tests|test)" | xargs rm -rf
}

clean_site_packages() {
	echo "removing everything in $LIB/site-packages"
	rm -rf $LIB/site-packages/*
}

rm_ext() {
	echo "removing $LIB/lib-dynload/$1.cpython-${VER}m-darwin.so"
	rm -rf $LIB/lib-dynload/$1.cpython-${VER}m-darwin.so
}

rm_bin() {
	echo "removing $PREFIX/bin/$1"
	rm -rf $PREFIX/bin/$1
}

build_normal() {
	cd $PYTHON


	./configure MACOSX_DEPLOYMENT_TARGET=${MAC_DEP_TARGET} \
	 	--prefix=$PREFIX \
	 	--enable-shared \
	 	--with-openssl=$SSL \
	 	--with-lto \
	 	--enable-optimizations

	make altinstall

	cd $ROOT
}


build() {
	cd $PYTHON

	./configure MACOSX_DEPLOYMENT_TARGET=${MAC_DEP_TARGET} \
	 	--prefix=$PREFIX \
	 	--enable-shared \
	 	--with-openssl=$SSL \
	 	--with-lto \
	 	--enable-optimizations

	make altinstall

	clean $PREFIX
	clean_tests $LIB
	clean_site_packages
	remove ${LIB}/site-packages

	rm_lib config-${VERSION}m-darwin
	rm_lib idlelib
	rm_lib lib2to3
	rm_lib tkinter
	rm_lib turtledemo
	rm_lib turtle.py
	rm_lib ensurepip
	rm_lib venv

	remove $LIB/distutils/command/*.exe
	remove $PREFIX/lib/pkgconfig
	remove $PREFIX/share

	rm_ext _tkinter
	rm_ext _codecs_jp
	rm_ext _codecs_hk
	rm_ext _codecs_cn
	rm_ext _codecs_kr
	rm_ext _codecs_tw

	rm_bin 2to3-${VERSION}
	rm_bin idle${VERSION}
	rm_bin easy_install-${VERSION}
	rm_bin pip${VERSION}

	mv $LIB/lib-dynload $PREFIX
	cp $LIB/os.py $PREFIX
	clean $PREFIX
	python -m zipfile -c $PREFIX/lib/python${VER}.zip $LIB/*
	remove $LIB
	mkdir -p $LIB
	mv $PREFIX/lib-dynload $LIB
	 # Need to have a copy of os.py to remain in site-packages
	 # or it will fail to pick up library.zip
	mv $PREFIX/os.py $LIB
	mkdir $LIB/site-packages

	cd $ROOT
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
	build
}


