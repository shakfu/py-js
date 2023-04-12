include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/common.cmake)

#############################################################
# MAX EXTERNAL
#############################################################

include_directories( 
	"${MAX_SDK_INCLUDES}"
	"${MAX_SDK_MSP_INCLUDES}"
	"${MAX_SDK_JIT_INCLUDES}"
)

file(GLOB PROJECT_SRC
     "*.h"
	"*.c"
     "*.cpp"
)


add_library( 
	${PROJECT_NAME} 
	MODULE
	${PROJECT_SRC}
)


target_compile_definitions(${PROJECT_NAME} 
	PRIVATE	
	$<$<CONFIG:Release>:RELEASE>
)


if(CMAKE_BUILD_TYPE STREQUAL Release)
	set(RESOURCES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${${PROJECT_NAME}_EXTERN_OUTPUT_NAME}.mxo/Contents/Resources")

	add_custom_command(
		TARGET ${PROJECT_NAME}
		POST_BUILD
		COMMAND mkdir -p ${RESOURCES}
		COMMAND cp -rf web/public ${RESOURCES}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Generating public web directory"
	)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)
