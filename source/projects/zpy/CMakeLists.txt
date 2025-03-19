include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/cmake/common.cmake)



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
	"${MAX_SDK_INCLUDES}/common/commonsyms.c"
)


target_include_directories(
	${PROJECT_NAME}
	PRIVATE
	"$<$<PLATFORM_ID:Darwin>:${local_prefix}/include>"
)

target_compile_options(
	${PROJECT_NAME}
	PRIVATE
	$<$<PLATFORM_ID:Darwin>:-Wno-unused-result>
	$<$<PLATFORM_ID:Darwin>:-Wsign-compare>
	$<$<PLATFORM_ID:Darwin>:-Wunreachable-code>
	$<$<PLATFORM_ID:Darwin>:-Wall>	
	$<$<PLATFORM_ID:Darwin>:-g>
	$<$<PLATFORM_ID:Darwin>:-fwrapv>
	$<$<PLATFORM_ID:Darwin>:-O3>
)

target_link_directories(
	${PROJECT_NAME} 
	PRIVATE
	"$<$<PLATFORM_ID:Darwin>:${local_prefix}/lib>"
)

target_link_libraries(
	${PROJECT_NAME} 
	PRIVATE
	"$<$<PLATFORM_ID:Darwin>:-ldl>"
	"$<$<PLATFORM_ID:Darwin>:-framework CoreFoundation>"
	"-lczmq"
	"-lzmq"
	"-lsodium"
)


include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)
