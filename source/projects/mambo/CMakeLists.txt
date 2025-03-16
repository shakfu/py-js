include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/cmake/common.cmake)

option(BUILD_MAMBO_SHARED "Build a shared external variant" OFF)
option(BUILD_MAMBO_STATIC "Build a static external variant" OFF)
option(BUILD_MAMBO_FRAMEWORK "Build a shared framework external variant" OFF)
option(BUILD_MAMBO_FRAMEWORK_PKG "Build a framework external in a package" OFF)

include(${CMAKE_SOURCE_DIR}/source/scripts/cmake/configure_python_external.cmake)

configure_python_external(
    ${PROJECT_NAME}
    ${BUILD_MAMBO_SHARED}
    ${BUILD_MAMBO_STATIC}
    ${BUILD_MAMBO_FRAMEWORK}
    ${BUILD_MAMBO_FRAMEWORK_PKG}
)

# cleanup to prevent cache distorting conditionals in subsequent runs
unset(BUILD_MAMBO_SHARED CACHE)
unset(BUILD_MAMBO_STATIC CACHE)
unset(BUILD_MAMBO_FRAMEWORK CACHE)
unset(BUILD_MAMBO_FRAMEWORK_PKG CACHE)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)

