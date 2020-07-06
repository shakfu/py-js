#!/usr/bin/env bash

source "scripts/common.sh"

PREFIX=${FRAMEWORKS}/Python.framework/Versions/${VERSION}
BIN=${PREFIX}/bin
LIB=${PREFIX}/lib/python${VERSION}
RESOURCES=${PREFIX}/Resources

URL_PYTHON=https://www.python.org/ftp/python/${SEMVER}/python-${SEMVER}-macosx10.9.pkg
FWK_TARGET=Frameworks/Python.Framework/Versions/${VERSION}/lib/${DYLIB}

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

get_python() {
	fname=$(basename $URL_PYTHON)

	if [ ! -d $BUILD/unpkg ] ; then
		curl $URL_PYTHON -o $BUILD/$fname
		pkgutil --expand $BUILD/$fname $BUILD/unpkg
	fi
	ditto -xz $BUILD/unpkg/Python_Framework.pkg/Payload $FRAMEWORKS/Python.Framework
	section "Unpacking ${fname} as Python.Framework into 'support'"	
}

rm_dlib() {
	echo "removing $1"
	rm -rf ${PREFIX}/lib/$1
}

clean_extra() {
	echo "more here"

	# rm_dlib sqlite3.21.0
	# rm_dlib libssl.dylib
	# rm_dlib libssl.1.1.dylib
	# rm_dlib libcrypto.dylib

	rm_dlib libtcl8.6.dylib
	rm_dlib libpanelw.5.dylib
	rm_dlib libformw.dylib
	rm_dlib libncursesw.dylib
	rm_dlib libtclstub8.6.a
	rm_dlib tdbcpostgres1.0.6
	rm_dlib itcl4.1.1
	rm_dlib tdbc1.0.6
	rm_dlib Tk.tiff
	rm_dlib libpanelw.dylib
	rm_dlib libmenuw.dylib
	rm_dlib tk8.6
	rm_dlib tcl8.6
	rm_dlib libtkstub8.6.a
	rm_dlib tdbcodbc1.0.6
	rm_dlib tclConfig.sh
	rm_dlib thread2.8.2
	rm_dlib libformw.5.dylib
	rm_dlib Tk.icns
	rm_dlib tclooConfig.sh
	rm_dlib libtk8.6.dylib
	rm_dlib tkConfig.sh
	rm_dlib libcrypto.1.1.dylib
	rm_dlib tdbcmysql1.0.6
	rm_dlib libmenuw.5.dylib
	rm_dlib tcl8
	rm_dlib libncursesw.5.dylib

	rm_bin 2to3
	rm_bin idle3
	rm_bin pyvenv

	remove ${RESOURCES}/Python.app
}


build_python() {
	get_python
	clean_python
	clean_extra
	write_python_getpip
}

build_python_zipped() {
	build_python
	zip_python_library
}

install_python() {
	build_python_zipped
}

install_python_ext() {
	install_python
	# fix_python_dylib_for_ext
}



# if [ "$1" == "pkg" ]; then
# 	echo "Installing python from source as framework into 'support' folder of package"
# 	install_python_pkg

# elif [ "$1" == "ext" ]; then
# 	echo "Installing python from source as framework into 'py.mxo' external"
# 	install_python_ext

# elif [ "$1" == "bin" ]; then
# 	echo "Installing python framework from binary .pkg in 'support' folder"
# 	install_bin_framework_pkg

# elif [ "$1" == "build-python" ]; then
# 	echo "Building from python source as framework"
# 	build_python_zipped

# else
# 	usage
# fi
