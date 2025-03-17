include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/cmake/common.cmake)

#############################################################
# CMX: Common Max Library
#############################################################

option(CPP_HEADER "Use in c++ external" OFF)


include_directories( 
	"${MAX_SDK_INCLUDES}"
	"${MAX_SDK_MSP_INCLUDES}"
	"${MAX_SDK_JIT_INCLUDES}"
)

if(CPP_HEADER)
	set(PROJECT_SRC cmx.cpp)
else()
	set(PROJECT_SRC cmx.c)
endif()

add_library( 
	${PROJECT_NAME}
	STATIC
	${PROJECT_SRC}
)

if(CPP_HEADER)
target_compile_features(
	${PROJECT_NAME}
	PRIVATE
	cxx_std_17
)
target_compile_definitions(
	${PROJECT_NAME}
	PRIVATE
	CMX_CPP_HEADER
)
endif()



# header propogates to sibling directories which link to this target (cmx)
# see: https://stackoverflow.com/questions/61709086/cmake-target-with-sibling-folders
target_include_directories(
	${PROJECT_NAME}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)


#include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)
