include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/cmake/common.cmake)

include(${CMAKE_SOURCE_DIR}/source/scripts/cmake/fn_python3_external.cmake)

python3_external(
    PROJECT_NAME ${PROJECT_NAME}
    BUILD_VARIANT ${BUILD_VARIANT}
)


include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)

