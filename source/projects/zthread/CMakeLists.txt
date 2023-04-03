include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/common.cmake)


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
	"${local_prefix}/include"
)

target_compile_options(
	${PROJECT_NAME}
	PRIVATE
	-Wno-unused-result
	-Wsign-compare
	-Wunreachable-code
	-DNDEBUG
	-g
	-fwrapv
	-O3
	-Wall
)

target_link_directories(
	${PROJECT_NAME} 
	PRIVATE
	"${local_prefix}/lib"
)

target_link_libraries(
	${PROJECT_NAME} 
	PRIVATE
	"-ldl"
	"-framework CoreFoundation"
	"-lzmq"
	"-lsodium"
)


include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)
