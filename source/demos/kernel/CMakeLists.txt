

add_subdirectory(thirdparty/xeus)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)

#############################################################
# MAX EXTERNAL
#############################################################

include_directories(
	"${MAX_SDK_INCLUDES}"
	"${MAX_SDK_MSP_INCLUDES}"
	"${MAX_SDK_JIT_INCLUDES}"
	"${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/json/include"
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


target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
	xeus-static
)


target_include_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/xeus/include
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)
