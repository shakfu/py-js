
cmake_minimum_required(VERSION 3.0)

set(C74_MIN_API_DIR ${CMAKE_SOURCE_DIR}/source/min-api)
include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)
include(${CMAKE_SOURCE_DIR}/source/scripts/cmake/common.cmake)

python3_external(
    MIN_API
    PROJECT_NAME ${PROJECT_NAME}
    OTHER_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/fmt/src/format.cc
    INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/fmt/include
    BUILD_VARIANT ${BUILD_VARIANT}
)


include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)
