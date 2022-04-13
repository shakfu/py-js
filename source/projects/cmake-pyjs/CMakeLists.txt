include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)

#############################################################
# MAX EXTERNAL
#############################################################

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")

find_package(Python3 COMPONENTS Interpreter Development)

include_directories( 
	"${MAX_SDK_INCLUDES}"
	"${MAX_SDK_MSP_INCLUDES}"
	"${MAX_SDK_JIT_INCLUDES}"

	"${Python3_INCLUDE_DIRS}"
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

target_link_directories(
	${PROJECT_NAME} 
	PRIVATE
	"${Python3_LIBRARY_DIRS}"
)


target_link_libraries(
	${PROJECT_NAME} 
	PRIVATE
	"${Python3_LIBRARIES}"
	"-ldl"
	"-framework CoreFoundation"
)


include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)
