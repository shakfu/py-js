#!/usr/bin/env bash

PREFIX=${PWD}/work
SRC_DIR=${PREFIX}/src
BUILD_DIR=${PREFIX}/build
INSTALL_DIR=${PREFIX}/install

git clone --depth=1 https://github.com/shakfu/python-cmake-buildsystem ${SRC_DIR}
cmake -S ${SRC_DIR} -B ${BUILD_DIR} \
    -DCMAKE_INSTALL_PREFIX:PATH=${INSTALL_DIR} \
    -DBUILD_LIBPYTHON_SHARED:BOOL=YES
cmake --build ${BUILD_DIR}
cmake --install ${BUILD_DIR}
${INSTALL_DIR}/bin/python -m test

