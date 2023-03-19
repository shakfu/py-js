include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/common.cmake)


#find_package(Python3 COMPONENTS Interpreter Development)

file(GLOB PROJECT_SRC
     "*.h"
	 "*.c"
     "*.cpp"
)


include_directories( 
	"${MAX_SDK_INCLUDES}"
	"${MAX_SDK_MSP_INCLUDES}"
	"${MAX_SDK_JIT_INCLUDES}"
	"/usr/local/include"
)

add_library( 
	${PROJECT_NAME} 
	MODULE
	${PROJECT_SRC}
	"${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/c74support/max-includes/common/commonsyms.c"
)

#set(PY_INCLUDE "${Python3_INCLUDE_DIRS}")
#set(PY_LIBDIRS "${Python3_LIBRARY_DIRS}")

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
#	${PY_INCLUDE}
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
	# -lstdc++
)

target_link_directories(
	${PROJECT_NAME} 
	PRIVATE
#	${PY_LIBDIRS}
	"/usr/local/lib"
)

target_link_libraries(
	${PROJECT_NAME} 
	PRIVATE
#	"${Python3_LIBRARIES}"
	"-ldl"
	"-framework CoreFoundation"
	"-lzmq"
	"-lsodium"
)


include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)
