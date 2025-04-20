include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/cmake/common.cmake)

set(BUILD_SHARED_LIBS OFF)

include(${CMAKE_CURRENT_SOURCE_DIR}/micropython.cmake)

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
     # below is required for static library case due to extern definition
     $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:${EMBED_DIR}/py/mpstate.c>
)

target_link_directories(
	${PROJECT_NAME}
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
	${MPY_LIB}
)


target_include_directories(${PROJECT_NAME}
	PRIVATE
	${MPY_INCLUDE_DIRECTORIES}
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)
