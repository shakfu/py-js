#!/usr/bin/env bash

PYTHON_VERSION=$(python3 --version | sed s/Python[[:space:]]//) # 3.9.5

PYTHON_VER=${PYTHON_VERSION%.*}          # 3.9
PYTHON_VER_PATCH="${PYTHON_VERSION##*.}" # 5
PYTHON_VER_NODOT="${PYTHON_VER//./}"     # 39

PYTHON_NAME=python${PYTHON_VER}

OPENSSL_VERSION=1.1.1g
BZIP2_VERSION=1.0.8
XZ_VERSION=5.2.5
GETTEXT_VERSION=0.20.2

MAC_DEP_TARGET=10.14

# --- need not modify below except to change excluded modules

URL_PY="https://www.python.org/ftp/python/${PYTHON_VERSION}/Python-${PYTHON_VERSION}.tgz"
URL_SSL="https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz"
URL_BZ2="https://sourceware.org/pub/bzip2/bzip2-${BZIP2_VERSION}.tar.gz"
URL_XZ="http://tukaani.org/xz/xz-${XZ_VERSION}.tar.gz"

URL_GETPIP="https://bootstrap.pypa.io/get-pip.py"

ROOT=$(pwd)

PY=${ROOT}
SCRIPTS=${PY}/scripts
PATCH=${PY}/patch
# PROJECTS=${PY}/../projects
SOURCE=${PY}/../source

EXTERNALS=${PY}/../../externals
PY_EXTERNAL=${EXTERNALS}/py.mxo
PYJS_EXTERNAL=${EXTERNALS}/pyjs.mxo

SUPPORT=${PY}/../../support
FRAMEWORKS=${SUPPORT}/Frameworks

BUILD=${PY}/targets/build
BUILD_DOWNLOADS=${BUILD}/downloads
BUILD_SRC=${BUILD}/src
BUILD_LIB=${BUILD}/lib

PYTHON=${BUILD}/Python-${PYTHON_VERSION}
PREFIX=${SUPPORT}/${PYTHON_NAME}
BIN=${PREFIX}/bin
LIB=${PREFIX}/lib/python${PYTHON_VER}
SSL_SRC=${BUILD}/openssl-${OPENSSL_VERSION}
SSL=${BUILD}/ssl
TMP=${BUILD}/_tmp
DYLIB_NAME=libpython${PYTHON_VER}
DYLIB=${DYLIB_NAME}.dylib
PY_EXTERNAL=${EXTERNALS}/py.mxo
PYJS_EXTERNAL=${EXTERNALS}/pyjs.mxo

URLS="$URL_PY $URL_SSL $URL_BZ2 $URL_XZ"

# GETTEXT dependency
LIBINTL=/usr/local/opt/gettext/lib/libintl.a

BUILD_SRC_PY=${BUILD_SRC}/Python-${PYTHON_VERSION}
BUILD_SRC_SSL=${BUILD_SRC}/openssl-${OPENSSL_VERSION}
BUILD_SRC_BZ2=${BUILD_SRC}/bzip2-${BZIP2_VERSION}
BUILD_SRC_XZ=${BUILD_SRC}/xz-${XZ_VERSION}

BUILD_LIB_SSL=${BUILD_LIB}/openssl
BUILD_LIB_BZ2=${BUILD_LIB}/bzip2
BUILD_LIB_XZ=${BUILD_LIB}/xz
BUILD_LIB_PY_STATIC=${BUILD_LIB}/python-static
BUILD_LIB_PY_SHARED=${BUILD_LIB}/python-shared
BUILD_LIB_PY_FRAMEWORK=${BUILD_LIB}/Python.framework


debug() {

	echo "PYTHON_NAME: $PYTHON_NAME"

	echo "PYTHON_VERSION: $PYTHON_VERSION"

	echo "PYTHON_VER: $PYTHON_VER"
	echo "PYTHON_VER_PATCH: $PYTHON_VER_PATCH"
	echo "PYTHON_VER_NODOT: $PYTHON_VER_NODOT"

	echo "PYTHON_NAME: $PYTHON_NAME"

	echo "OPENSSL_VERSION: $OPENSSL_VERSION"
	echo "BZIP2_VERSION: $BZIP2_VERSION"
	echo "XZ_VERSION: $XZ_VERSION"
	echo "GETTEXT_VERSION: $GETTEXT_VERSION"

	echo "VERSION_MAJOR: $VERSION_MAJOR"
	echo "VERSION_MINOR: $VERSION_MINOR"
	echo "VER: $VER"

	echo "MAC_DEP_TARGET: $MAC_DEP_TARGET"

	echo "URL_PY: $URL_PY"
	echo "URL_SSL: $URL_SSL"
	echo "URL_GETPIP: $URL_GETPIP"

	echo "ROOT: $ROOT"
	echo "SUPPORT: $SUPPORT"
	echo "SOURCE: $SOURCE"
	echo "PY: $PY"
	echo "TARGETS: $TARGETS"
	echo "BUILD: $BUILD"
	echo "PREFIX: $PREFIX"
	echo "BIN: $BIN"
	echo "LIB: $LIB"
	echo "SSL: $SSL"
	echo "TMP: $TMP"
}

# -----------------------------------------------------------------------
# COLORS

COLOR_BOLD_YELLOW="\033[1;33m"
COLOR_BOLD_BLUE="\033[1;34m"
COLOR_BOLD_GREEN="\033[1;32m"
COLOR_BOLD_MAGENTA="\033[1;35m"
COLOR_BOLD_CYAN="\033[1;36m"
COLOR_RESET="\033[m"

function section {
	echo -e "${COLOR_BOLD_CYAN}>>> ${1} ${COLOR_RESET}"
}

function info() {
	echo -e "$COLOR_BOLD_YELLOW""INFO: ""$COLOR_RESET""$1"
}

function warn {
	echo -e "$COLOR_BOLD_YELLOW""WARNING: ""$COLOR_RESET""$1"
}

function success() {
	echo -e "$COLOR_BOLD_GREEN""SUCCESS: ""$COLOR_RESET""$1"
}

function fail() {
	echo -e "$COLOR_BOLD_MAGENTA""FAILURE: ""$COLOR_RESET""$1"
}
