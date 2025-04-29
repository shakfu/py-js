
cmake_minimum_required(VERSION 3.0)

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/min-api)
include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)


python3_external(
    MIN_API
    PROJECT_NAME ${PROJECT_NAME}
    BUILD_VARIANT ${BUILD_VARIANT}
)


include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)
