include(${CMAKE_SOURCE_DIR}/source/scripts/cmake/max-pretarget.cmake)



set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Release")


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
	${CMAKE_CURRENT_SOURCE_DIR}
)


target_compile_options(
	${PROJECT_NAME}
	PRIVATE
	$<$<PLATFORM_ID:Darwin>:-Wno-unused-variable>
	$<$<PLATFORM_ID:Darwin>:-Wno-sign-compare>
	$<$<PLATFORM_ID:Darwin>:-Wfatal-errors>
	$<$<PLATFORM_ID:Darwin>:-Wall>	
	$<$<PLATFORM_ID:Darwin>:-O2>
	$<$<PLATFORM_ID:Darwin>:-frtti>
	$<$<PLATFORM_ID:Darwin>:-stdlib=libc++>
	$<$<PLATFORM_ID:Darwin>:--std=c++17>
	$<$<PLATFORM_ID:Windows>:/std:c++17>
	$<$<PLATFORM_ID:Windows>:/EHsc>
	$<$<PLATFORM_ID:Windows>:/utf-8>
	$<$<PLATFORM_ID:Windows>:/O2>
)


include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)
