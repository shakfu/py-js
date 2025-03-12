include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/common.cmake)

set(Python3_ROOT_DIR "${CMAKE_SOURCE_DIR}/build/install/python-shared")
set(Python3_EXECUTABLE "${Python3_ROOT_DIR}/bin/python3")
find_package (Python3 COMPONENTS Interpreter Development REQUIRED)

MESSAGE(STATUS "Python3_ROOT_DIR: ${Python3_ROOT_DIR}")
MESSAGE(STATUS "Python3_EXECUTABLE: ${Python3_EXECUTABLE}")

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
	${Python3_INCLUDE_DIRS}
)

target_compile_definitions(
	${PROJECT_NAME}
	PRIVATE
	-DNDEBUG
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
	${Python3_LIBRARY_DIRS}
)

# set_target_properties(
# 	${PROJECT_NAME} 
#     PROPERTIES
#     XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@loader_path/../Resources/lib/libpython3.13.dylib"
#     # XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@loader_path/../Resources/python/lib/libpython3.13.dylib"
#     # XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@loader_path;@loader_path/../Resources/python/lib"
# )

target_link_libraries(
	${PROJECT_NAME} 
	PRIVATE
	"${Python3_LIBRARIES}"
	"$<$<PLATFORM_ID:Darwin>:-ldl>"
	"$<$<PLATFORM_ID:Darwin>:-framework CoreFoundation>"
)



file(COPY "${Python3_ROOT_DIR}/lib" DESTINATION "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.mxo/Contents/Resources")
# file(COPY "${Python3_ROOT_DIR}/" DESTINATION "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.mxo/Contents/Resources/python")


include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)

