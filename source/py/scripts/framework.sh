#!/usr/bin/env bash

source "scripts/common.sh"


PREFIX=${FRAMEWORKS}/Python.framework/Versions/${VERSION}
BIN=${PREFIX}/bin


configure_python() {
	./configure MACOSX_DEPLOYMENT_TARGET=${MAC_DEP_TARGET} \
	 	--with-openssl=$SSL \
	 	--enable-framework=$FRAMEWORKS
}


if [ "$1" == "pkg" ]; then
	echo "Installing python from source as framework into 'support' folder of package"
	intall_python_pkg

elif [ "$1" == "ext" ]; then
	echo "Installing python from source as framework into 'py.mxo' external"
	intall_python_ext

elif [ "$1" == "build-python" ]; then
	echo "Building from python source as framework"
	build_python_zipped

elif [ "$1" == "fix-pkg" ]; then
	echo "fixing dynamic lookup refs for package installation"
	fix_python_dylib_for_pkg
	otool -L $PREFIX/lib/$DYLIB

elif [ "$1" == "fix-ext" ]; then
	echo "fixing dynamic lookup refs for external installation"
	fix_python_dylib_for_ext
	otool -L $PREFIX/lib/$DYLIB
fi
