#!/usr/bin/env bash

source "scripts/common.sh"

configure_python() {
	./configure MACOSX_DEPLOYMENT_TARGET=${MAC_DEP_TARGET} \
	 	--prefix=$PREFIX 	\
	 	--with-openssl=$SSL \
	 	--with-lto 			\
	 	--enable-optimizations
}

# write_python_minim_setup_local() {
# 	cp ${SCRIPTS}/Setup.local ${PYTHON}/Modules/Setup.local
# }


install_python() {
	echo "checking if previous build exists"
	mkdir -p $BUILD
	if [ ! -d $PYTHON ] ; then
		echo "retrieving $PYTHON from $URL_PYTHON"
		get_python
	fi
	if [ ! -d $SSL ] ; then
		echo "retrieving $SSL from $URL_OPENSSL"
		get_ssl
		build_ssl
	fi
	compile_python
	clean_python

	# zipped library causes a crash (works otherwise)
	# zip_python_library
}



if [ "$1" == "pkg" ]; then
    echo "Installing python from source as static lib into 'support' folder of package"
    install_python

elif [ "$1" == "ext" ]; then
    echo "Installing python from source as static lib into external"
    install_python
else
    usage
fi