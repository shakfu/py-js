# constants
COLOR_BOLD_CYAN="\033[1;36m"
COLOR_RESET="\033[m"

URL_LUA=https://www.lua.org/ftp/lua-5.1.5.tar.gz
URL_LUAJIT=http://luajit.org/download/LuaJIT-2.0.5.tar.gz 

LUAJIT_VERSION=2.0.5
MACOSX_DEPLOYMENT_TARGET=10.9

PROJECT_NAME=luna

ROOT=`pwd`
EXTERNALS=${ROOT}/../../externals
EXTERNAL=${EXTERNALS}/${PROJECT_NAME}.mxo
PACKAGE=${HOME}"/Documents/Max 8/Packages/py"
BUILD=${ROOT}/build
VERSION_LUAJIT=${BUILD}/LuaJIT-${LUAJIT_VERSION}
LUAJIT=${BUILD}/luajit
LUA=${BUILD}/lua

CFLAGS_LUA=-I${LUA}/include
LDFLAGS_LUA=-L${LUA}/lib -llua -lm

CFLAGS_LUAJIT=-I${LUAJIT}/include/luajit-2.0
LDFLAGS_LUAJIT=-L${LUAJIT}/lib -lluajit -lm ${LDFLAGS_EXTRA_LUAJIT}
LDFLAGS_EXTRA_LUAJIT=-pagezero_size 10000 -image_base 100000000

# ============================================================================
# FUNCTIONS

# $(call section,string)
section = @echo ${COLOR_BOLD_CYAN}">>> ${1}"${COLOR_RESET}


# ============================================================================
# TARGETS

all: luna deploy

.PHONY: luna show prep-luajit prog-lua prog-luajit clean

show:
	@echo "ROOT: ${ROOT}"
	@echo "BUILD: ${BUILD}"
	@echo "LUAJIT_SRC: ${VERSION_LUAJIT}"
	@echo "PREFIX=${LUAJIT}"

prep-luajit:	
	@make -C ${VERSION_LUAJIT} \
		install PREFIX=${LUAJIT} \
		MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET}
	@rm ${LUAJIT}/lib/*.dylib
	@ln -s ${LUAJIT}/lib/libluajit-5.1.a ${LUAJIT}/lib/libluajit.a

luna:
	@xcodebuild -project ${PROJECT_NAME}.xcodeproj

deploy:
	$(call section,"deploying external to local package")
	@rm -rf ${PACKAGE}/externals/luna.mxo
	@cp -rf ${EXTERNAL} ${PACKAGE}/externals



prog-lua:
	@cc -o test_lua test_lua.c ${CFLAGS_LUA} ${LDFLAGS_LUA}

prog-luajit:
	@cc -o test_lua test_luajit.c ${CFLAGS_LUAJIT} ${LDFLAGS_LUAJIT}

clean:
	@rm test_lua

