
# input variables
NAME=luna
VERSION=5.4.0
MACOSX_DEPLOYMENT_TARGET=10.9

# misc variables
URL_LUA=https://www.lua.org/ftp/lua-${VERSION}.tar.gz
ARCHIVE=$(shell basename ${URL_LUA})
SRC_LUA=$(shell basename -s .tar.gz ${ARCHIVE})

ROOT=$(shell pwd)
EXTERNALS=${ROOT}/../../externals
EXTERNAL=${EXTERNALS}/${NAME}.mxo
PACKAGE=${HOME}"/Documents/Max 8/Packages/py"
BUILD=${ROOT}/build
LUA=${BUILD}/lua

CFLAGS_LUA=-I${LUA}/include
LDFLAGS_LUA=-L${LUA}/lib -llua -lm

COLOR_BOLD_CYAN="\033[1;36m"
COLOR_RESET="\033[m"

# ============================================================================
# FUNCTIONS

# $(call section,string)
section = @echo ${COLOR_BOLD_CYAN}">>> ${1}"${COLOR_RESET}


# ============================================================================
# TARGETS

all: external deploy

.PHONY: external show prep-lua test-lua clean

show:
	@echo "ROOT: ${ROOT}"
	@echo "BUILD: ${BUILD}"

prep-lua:
	$(call section,"building ${SRC_LUA} installation from source")
	@mkdir -p ${BUILD}
	@mkdir -p ${LUA}
	@curl ${URL_LUA} -o ${BUILD}/${ARCHIVE}
	@tar -C ${BUILD} -xvf ${BUILD}/${ARCHIVE}
	@make -C ${BUILD}/${SRC_LUA}
	@make -C ${BUILD}/${SRC_LUA} install INSTALL_TOP=${LUA}

external:
	$(call section,"building ${NAME} external")
	@xcodebuild -project ${NAME}.xcodeproj

deploy:
	$(call section,"deploying external to local package")
	@rm -rf ${PACKAGE}/externals/${NAME}.mxo
	@cp -rf ${EXTERNAL} ${PACKAGE}/externals

test-lua:
	@cc -o test_lua test_lua.c ${CFLAGS_LUA} ${LDFLAGS_LUA}

clean:
	@rm test_lua

