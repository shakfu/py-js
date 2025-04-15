include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/cmake/common.cmake)

python3_external(
    PROJECT_NAME ${PROJECT_NAME}
    BUILD_VARIANT ${BUILD_VARIANT}
)

set(XPCSERVICES_DIR "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${${PROJECT_NAME}_EXTERN_OUTPUT_NAME}.mxo/Contents/XPCServices")

add_custom_command(
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND mkdir -p ${XPCSERVICES_DIR}
	COMMAND mv ${CMAKE_BINARY_DIR}/Release/PythonService.xpc ${XPCSERVICES_DIR}
	COMMENT "mv build xpc service bundle to external bundle"
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)
