#!/usr/bin/env bash
# FIXME!!! for pkg installation,
# Does not link properly to $SUPPORT/Frameworks/Python.framework
# but to py-js/source/py/targets/build/lib/Python.framework

source "scripts/build.sh"

FRAMEWORKS=${SUPPORT}/Frameworks
# PREFIX=${SUPPORT}/Frameworks/Python.framework/Versions/${PYTHON_VER}
PREFIX=${SUPPORT}/Frameworks
BIN=${PREFIX}/bin
LIB=${PREFIX}/lib/python${PYTHON_VER}

FWK_TARGET=Frameworks/Python.Framework/Versions/${PYTHON_VER}/lib/${DYLIB}

install_python_pkg() {
	mkdir -p ${FRAMEWORKS}
	cp -Rf ${BUILD_LIB}/Python.framework ${FRAMEWORKS}
	fix_python_dylib_for_pkg
}

install_python_ext() {
	mkdir -p ${FRAMEWORKS}
	cp -Rf ${BUILD_LIB}/Python.framework ${FRAMEWORKS}
	build_python_framework $PREFIX
	fix_python_dylib_for_ext
}

fix_python_dylib_for_pkg() {
	cd ${SUPPORT}/Frameworks/Python.Framework/Versions/${PYTHON_VER}/lib
	chmod 777 ${DYLIB}
	install_name_tool -id @loader_path/../../../../support/${FWK_TARGET} ${DYLIB}
	cd $ROOT
}

fix_python_dylib_for_ext() {
	cd ${SUPPORT}/Frameworks/Python.Framework/Versions/${PYTHON_VER}/lib
	chmod 777 ${DYLIB}
	install_name_tool -id @loader_path/../${FWK_TARGET} ${DYLIB}
	#cp -rf ${FRAMEWORKS} $PY_EXTERNAL/Contents
	cd $ROOT
}

reset() {
	rm -rf $PREFIX
}

if [ "$1" == "pkg" ]; then
	echo "Installing python from source as framework into 'support' folder of package"
	install_python_pkg

elif [ "$1" == "ext" ]; then
	echo "Installing python from source as framework into external"
	install_python_ext
else
	exit
fi
