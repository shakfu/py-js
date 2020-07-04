#!/usr/bin/env bash

source "scripts/common.sh"


configure_python() {
	./configure MACOSX_DEPLOYMENT_TARGET=${MAC_DEP_TARGET} \
	 	--prefix=$PREFIX 	\
	 	--enable-shared 	\
	 	--with-openssl=$SSL \
	 	--with-lto 			\
	 	--enable-optimizations
}


if [ "$1" == "pkg" ]; then
    echo "Installing python from source as shared lib into 'support' folder of package"
    reset
    install_python_pkg

elif [ "$1" == "ext" ]; then
    echo "Installing python from source as shared lib into 'py.mxo' external"
    reset
    install_python_ext

elif [ "$1" == "build-python" ]; then
	echo "Building from python source as shared lib"
	build_python_zipped

elif [ "$1" == "fix-pkg" ]; then
	echo "fixing dynamic lookup refs for package installation"
	fix_python_dylib_for_pkg
	otool -L $PREFIX/lib/$DYLIB

elif [ "$1" == "fix-ext" ]; then
	echo "fixing dynamic lookup refs for external installation"
	fix_python_dylib_for_ext
	otool -L $PREFIX/lib/$DYLIB
else
    usage
fi




