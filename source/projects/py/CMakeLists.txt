include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/common.cmake)

option(INCLUDE_NUMPY "include numpy headers if available")


set(SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/scripts)


find_package(Python3 COMPONENTS Interpreter Development)


set(PROJECT_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/py.c
	${CMAKE_CURRENT_SOURCE_DIR}/api.c
)

include_directories( 
	"${MAX_SDK_INCLUDES}"
	"${MAX_SDK_MSP_INCLUDES}"
	"${MAX_SDK_JIT_INCLUDES}"
)

if(INCLUDE_NUMPY)
	# optionally add numpy_header
	execute_process(
		COMMAND ${Python3_EXECUTABLE} ${SCRIPTS}/check_numpy.py --include
		OUTPUT_VARIABLE NUMPY_HEADERS
		RESULT_VARIABLE NUMPY_HEADERS_RESULT
	)

	if (NUMPY_HEADERS_RESULT EQUAL 0) # 0 == SUCCESS
		message(STATUS "INCLUDING NUMPY_HEADERS")
		message(STATUS "NUMPY_HEADERS: " ${NUMPY_HEADERS})
		include_directories("${NUMPY_HEADERS}")
		add_compile_definitions(INCLUDE_NUMPY=1)
	else()
		message(STATUS "NOT INCLUDING NUMPY_HEADERS: NUMPY NOT FOUND")
	endif()
endif()


add_library( 
	${PROJECT_NAME} 
	MODULE
	${PROJECT_SRC}
)

set(PY_INCLUDE "${Python3_INCLUDE_DIRS}")
set(PY_LIBDIRS "${Python3_LIBRARY_DIRS}")

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
	${PY_INCLUDE}
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
	$<$<PLATFORM_ID:Windows>:/O2>
	$<$<PLATFORM_ID:Windows>:/MD> # api module works with this
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/api.c
	COMMAND cython -3 -E INCLUDE_NUMPY=${CYTHON_INCL_NUMPY} api.pyx
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS api.pyx
	COMMENT "Generating api.c"
)

add_custom_target(api_c
	DEPENDS api.pyx
)

add_dependencies(${PROJECT_NAME} api_c)


target_link_directories(
	${PROJECT_NAME} 
	PRIVATE
	${PY_LIBDIRS}
)

target_link_libraries(
	${PROJECT_NAME} 
	PRIVATE
	"${Python3_LIBRARIES}"
	"$<$<PLATFORM_ID:Darwin>:-ldl>"
	"$<$<PLATFORM_ID:Darwin>:-framework CoreFoundation>"
)


include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)
