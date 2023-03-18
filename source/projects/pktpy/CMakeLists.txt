include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)


set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")

set(CMAKE_EXPORT_COMPILE_COMMANDS True)

file(GLOB PROJECT_SRC
     "*.h"
	"*.c"
     "*.cpp"
)

include_directories( 
	"${MAX_SDK_INCLUDES}"
	"${MAX_SDK_MSP_INCLUDES}"
	"${MAX_SDK_JIT_INCLUDES}"
)

add_library( 
	${PROJECT_NAME} 
	MODULE
	${PROJECT_SRC}
)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
	"pocketpy.h"
)

target_compile_options(
	${PROJECT_NAME}
	PRIVATE
	--std=c++17
	-O2
	-Wall
)

# target_link_directories(
# 	${PROJECT_NAME} 
# 	PRIVATE
# )

# target_link_libraries(
# 	${PROJECT_NAME} 
# 	PRIVATE
# 	"-ldl"
# 	"-framework CoreFoundation"
# )


include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)

