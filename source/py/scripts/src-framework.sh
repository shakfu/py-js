#!/usr/bin/env bash

source "scripts/common.sh"

PREFIX=${FRAMEWORKS}/Python.framework/Versions/${VERSION}
BIN=${PREFIX}/bin
LIB=${PREFIX}/lib/python${VERSION}

FWK_TARGET=Frameworks/Python.Framework/Versions/${VERSION}/lib/${DYLIB}

configure_python() {
	./configure MACOSX_DEPLOYMENT_TARGET=${MAC_DEP_TARGET} \
	 	--with-openssl=$SSL \
	 	--enable-framework=$FRAMEWORKS
}

fix_python_dylib_for_pkg() {
	cd $PREFIX/lib
	chmod 777 ${DYLIB}
	install_name_tool -id @loader_path/../../../../support/${FWK_TARGET} ${DYLIB}
	cd $ROOT
}

fix_python_dylib_for_ext() {
	cd $PREFIX/lib
	chmod 777 ${DYLIB}
	install_name_tool -id @loader_path/../${FWK_TARGET} ${DYLIB}
	cp -rf ${FRAMEWORKS} $PY_EXTERNAL/Contents
	cd $ROOT
}

fix() {
	cp -rf ${FRAMEWORKS} $PY_EXTERNAL/Contents
	cd $PY_EXTERNAL/Contents/${FWK_TARGET}/lib
	chmod 777 ${DYLIB}
	install_name_tool -id @loader_path/../${FWK_TARGET} ${DYLIB}
	cd $ROOT
}


install_python_ext() {
	install_python
	# fix_python_dylib_for_ext
}

if [ "$1" == "pkg" ]; then
	echo "Installing python from source as framework into 'support' folder of package"
	install_python_pkg

elif [ "$1" == "ext" ]; then
	echo "Installing python from source as framework into 'py.mxo' external"
	install_python_ext

elif [ "$1" == "bin" ]; then
	echo "Installing python framework from binary .pkg in 'support' folder"
	install_bin_framework_pkg

elif [ "$1" == "build-python" ]; then
	echo "Building from python source as framework"
	build_python_zipped
else
	usage
fi
