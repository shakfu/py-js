#!/usr/bin/env bash

source "scripts/common.sh"

configure_python() {
	./configure MACOSX_DEPLOYMENT_TARGET=${MAC_DEP_TARGET} \
	 	--prefix=$PREFIX 	\
	 	--with-openssl=$SSL \
	 	--with-lto 			\
	 	--enable-optimizations
}

install_python() {
	compile_python
	clean_python
	# zipped library causes a crash (works otherwise)
	# zip_python_library
}


if [ "$1" == "ext" ]; then
    echo "Installing python from source as static lib into 'support' folder of package"
    install_python
fi