#!/usr/bin/env bash

# shellcheck disable=SC1091
source "scripts/base.sh"

function remove() {
	info "removing $1"
	rm -rf "$1"
}

function write_setup_local() {
	cp "$PATCH"/"$PYTHON_VER"/"$1" "$BUILD_SRC_PY"/Modules/Setup.local
}

function apply_patch() {
	patch -p1 < "$PATCH"/"$PYTHON_VER"/"$1"
}

function reverse_patch() {
	patch -p1 -R < "$PATCH"/"$PYTHON_VER"/"$1"
}

function get_url() {
	section "retrieving: $1"
	mkdir -p "$BUILD_DOWNLOADS"
	mkdir -p "$BUILD_SRC"
	archive=$(basename "$1")
	if [[ $archive == *.tar.gz ]]; then
		name=$(basename -s .tar.gz "$archive")
	elif [[ $archive == *.tgz ]]; then
		name=$(basename -s .tgz "$archive")
	fi

	if [ ! -f "$BUILD_DOWNLOADS"/"${name}".tgz ]; then
		curl --fail -L "$1" -o "$BUILD_DOWNLOADS"/"${name}".tgz
	else
		info "skipping existing src archive: $archive"
	fi

	if [ ! -d "$BUILD_SRC"/"$name" ]; then
		tar -C "$BUILD_SRC" -xvf "$BUILD_DOWNLOADS"/"${name}".tgz
	else
		info "skipping existing src dir: $name"
	fi
}

function get_all() {
	for url in $URLS; do
		get_url "$url"
	done
}

function get_python() {
	get_url "$URL_PY"
}

function build_ssl() {
	cd "$BUILD_SRC_SSL" || exit
	make clean
	./config no-shared no-tests --prefix="$BUILD_LIB_SSL"
	make install_sw
	cd "$ROOT" || exit
}

function build_bz2() {
	cd "$BUILD_SRC_BZ2" || exit
	make clean
	make install PREFIX="$BUILD_LIB_BZ2"
	cd "$ROOT" || exit
}

function build_xz() {
	cd "$BUILD_SRC_XZ" || exit
	./configure \
		MACOSX_DEPLOYMENT_TARGET="${MAC_DEP_TARGET}" \
		--disable-shared --enable-static \
		--prefix="$BUILD_LIB_XZ"
	make install
	cd "$ROOT" || exit
}

function test_python_build() {
	DIR=$1
	cd "$DIR" || exit
	RESULT=$(./bin/python3.9 -c "import os; print(os.__file__)")
	EXPECTED=$(pwd)/lib/python39.zip/os.py
	if [[ $RESULT == "$EXPECTED" ]]; then
		success "$DIR"
	else
		fail "$DIR"
	fi
	cd "$ROOT" || exit
}

# takes $PREFIX as as $1
function clean_python() {
	DIR=$1
	BIN=$1/bin
	LIB=$1/lib/python${PYTHON_VER}

	find "$DIR" | grep -E "(__pycache__|\.pyc|\.pyo$)" | xargs rm -rf
	find "$LIB" | grep -E "(tests|test)" | xargs rm -rf
	rm -rf "$LIB"/site-packages/*

	rm -rf "$LIB"/distutils/command/*.exe
	rm -rf "$DIR"/lib/pkgconfig
	rm -rf "$DIR"/share

	MODS="\
		cgi.py
		cgitb.py
		config-${PYTHON_VER}-darwin
		ctypes
		curses
		distutils
		ensurepip
		gettext.py
		idlelib
		lib2to3
		pydoc.py
		pydoc_data
		sqlite3
		sunau.py
		turtle.py
		turtledemo
		venv
	"
	for mod in $MODS; do
		rm -rf "$LIB"/"${mod:?}"
	done

	EXTS="\
		_tkinter
		_ctypes
		_multibytecodec
		_codecs_jp
		_codecs_hk
		_codecs_cn
		_codecs_kr
		_codecs_tw
		_codecs_iso2022
		_curses
		_curses_panel
	"
	for ext in $EXTS; do
		rm -rf "$LIB"/lib-dynload/"${ext}".cpython-"${PYTHON_VER_NODOT}"-darwin.so
	done

	BINS="\
		2to3-${PYTHON_VER}
		idle${PYTHON_VER}
		2to3-${PYTHON_VER}
		idle${PYTHON_VER}
		easy_install-${PYTHON_VER}
		pip${PYTHON_VER}
		python${PYTHON_VER}m
		pyvenv-${PYTHON_VER}
		pydoc${PYTHON_VER}
	"
	for executable in $BINS; do
		rm -rf "$BIN"/"${executable:?}"
	done
}

function zip_python_library() {
	DIR=$1
	LIB=$1/lib/python${PYTHON_VER}
	rm -rf "${LIB}"/site-packages
	mv "$LIB"/lib-dynload "$DIR"
	cp "$LIB"/os.py "$DIR"
	python3 -m zipfile -c "$DIR"/lib/python"${PYTHON_VER_NODOT}".zip "$LIB"/*
	rm -rf "$LIB"
	mkdir -p "$LIB"
	mv "$DIR"/lib-dynload "$LIB"
	# Need to have a copy of os.py to remain in site-packages
	# or it will fail to pick up library.zip
	mv "$DIR"/os.py "$LIB"
	mkdir "$LIB"/site-packages
}

function build_python_shared() {
	local PREFIX=${1:-$BUILD_LIB_PY_SHARED}
	cd "$BUILD_SRC_PY" || exit
	make clean
	write_setup_local setup-shared.local
	#apply_patch
	./configure \
		MACOSX_DEPLOYMENT_TARGET="$MAC_DEP_TARGET" \
		--prefix="$PREFIX" \
		--enable-shared \
		--with-openssl="$BUILD_LIB_SSL" \
		--without-doc-strings \
		--enable-ipv6 \
		--without-ensurepip \
		--with-lto \
		--enable-optimizations
	make altinstall
	cd "$ROOT" || exit
	clean_python "$PREFIX"
	zip_python_library "$PREFIX"
	test_python_build "$PREFIX"
}

function build_python_static() {
	local PREFIX=${1:-$BUILD_LIB_PY_STATIC}
	cd "$BUILD_SRC_PY" || exit
	make clean
	# write_setup_local setup-static-min2.local
	#apply_patch makesetup.patch
	./configure \
		MACOSX_DEPLOYMENT_TARGET="$MAC_DEP_TARGET" \
		--prefix="$PREFIX" \
		--without-doc-strings \
		--enable-ipv6 \
		--without-ensurepip \
		--with-lto \
		--enable-optimizations
	make altinstall
	#reverse_patch makesetup.patch
	cd "$ROOT" || exit
	clean_python "$PREFIX"
	zip_python_library "$PREFIX"
	test_python_build "$PREFIX"
}

function build_python_framework() {
	local PREFIX=${1:-$BUILD_LIB}
	# assuming PREFIX=${SUPPORT}/Frameworks
	local FWK_PREFIX=${PREFIX}/Python.framework/Versions/${PYTHON_VER}
	cd "$BUILD_SRC_PY" || exit
	make clean
	write_setup_local setup-shared.local
	#apply_patch makesetup.patch
	./configure \
		MACOSX_DEPLOYMENT_TARGET="${MAC_DEP_TARGET}" \
		--enable-framework="${PREFIX}" \
		--with-openssl="${BUILD_LIB_SSL}" \
		--without-doc-strings \
		--enable-ipv6 \
		--without-ensurepip \
		--with-lto \
		--enable-optimizations
	make altinstall
	#reverse_patch makesetup.patch
	cd "$ROOT" || exit
	clean_python "$FWK_PREFIX"
	zip_python_library "$FWK_PREFIX"
	test_python_build "$FWK_PREFIX"
}

function reset_build_downloads() {
	rm -rf "$BUILD_DOWNLOADS"
}

function reset_build_src() {
	rm -rf "$BUILD_DOWNLOADS"
}

# reset() {
# 	rm -rf $BUILD_LIB
# }

function reset_all() {
	rm -rf "$BUILD_DOWNLOADS"
	rm -rf "$BUILD_SRC"
	rm -rf "$BUILD_LIB"
}

function fix_python_dylib_for_pkg() {
	cd "$PREFIX"/lib || exit
	chmod 777 "${DYLIB}"
	# assumes python in installed in $PREFIX
	# ../../../../support/python3.7/lib/libpython3.7m.dylib
	install_name_tool -id @loader_path/../../../../support/"${PYTHON_NAME}"/lib/"${DYLIB}" "${DYLIB}"
	echo "fix_python_dylib_for_pkg done"
	# otool -L ${DYLIB}
	cd "$ROOT" || exit
}

function fix_python_dylib_for_ext() {
	cd "$PREFIX"/lib || exit
	chmod 777 "${DYLIB}"
	# assumes cp -rf $PREFIX/* -> same directory as py extension in py.mxo
	install_name_tool -id @loader_path/"${DYLIB}" "${DYLIB}"
	echo "fix_python_dylib_for_ext done"
	cd "$ROOT" || exit
}

function install_dependencies() {
	echo "checking if previous build exists"
	mkdir -p "$BUILD"
	if [ ! -d "$BUILD_DOWNLOADS" ]; then
		echo "retrieving downloads"
		get_all
	fi
	if [ ! -d "$BUILD_LIB_SSL" ]; then
		echo "building ssl"
		build_ssl
	fi
	if [ ! -d "$BUILD_LIB_BZ2" ]; then
		echo "building bz2"
		build_bz2
	fi
	if [ ! -d "$BUILD_LIB_XZ" ]; then
		echo "building xz"
		build_xz
	fi
}

function install_python_pkg() {
	install_dependencies
	install_python
	fix_python_dylib_for_pkg
}

function install_python_ext() {
	install_dependencies
	install_python
	fix_python_dylib_for_ext
}

# Allows to call a function based on arguments passed to the script
"$*"
