include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/cmake/common.cmake)

include(${CMAKE_SOURCE_DIR}/source/scripts/cmake/fn_python3_external.cmake)

python3_external(
    PROJECT_NAME ${PROJECT_NAME}
    BUILD_VARIANT ${BUILD_VARIANT}
)

set(XPCSERVICES_DIR "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${${PROJECT_NAME}_EXTERN_OUTPUT_NAME}.mxo/Contents/XPCServices")

add_custom_command(
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND mkdir -p ${XPCSERVICES_DIR}
	COMMAND cp -rf ${CMAKE_CURRENT_SOURCE_DIR}/PythonService.xpc ${XPCSERVICES_DIR}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMENT "Copy xpc service to bundle"
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)
