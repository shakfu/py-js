#!/usr/bin/env bash
# build_python.sh

# This should be run in the root of the Python source directory

# NOTE: need os.py to remain in site-packages or it will fail



NAME=xpython
PWD=$(pwd)
PREFIX=${PWD}/${NAME}
VERSION=3.8
VER="${VERSION//./}"
LIB=${PREFIX}/lib/python${VERSION}
MAC_DEP_TARGET=10.13


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
	echo "removing $LIB/lib-dynload/$1.cpython-${VER}-darwin.so"
	rm -rf $LIB/lib-dynload/$1.cpython-38-darwin.so
}

rm_bin() {
	echo "removing $PREFIX/bin/$1"
	rm -rf $PREFIX/bin/$1
}

./configure MACOSX_DEPLOYMENT_TARGET=${MAC_DEP_TARGET} \
  --prefix=$PREFIX \
  --enable-shared \
  --with-lto \
  --enable-optimizations

make altinstall

clean $PREFIX
clean_tests $LIB
clean_site_packages
remove ${LIB}/site-packages

rm_lib config-${VERSION}-darwin
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
mv $PREFIX/os.py $LIB
mkdir $LIB/site-packages

# otool -L $PREFIX/lib/libpython${VERSION}.dylib
# cp /usr/local/opt/gettext/lib/libintl.8.dylib ${PREFIX}/LIB/
# chmod 777 ${PREFIX}/LIB/libintl.8.dylib
# install_name_tool -id @executable_path/libintl.8.dylib ${PREFIX}/LIB/libintl.8.dylib
# install_name_tool -change /usr/local/opt/gettext/lib/libintl.8.dylib @executable_path/libintl.8.dylib libpython${VERSION}.dylib


