include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/cmake/common.cmake)

option(INCLUDE_NUMPY "include numpy headers if available")
option(INCLUDE_COMMONSYMS "include commmonsym.c")
option(INCLUDE_API_MODULE "include api c module" ON)
option(REGEN_API_MODULE "enable cython regen of api.c if api.pyx is modified")


set(SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/scripts)
set(API_PYX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/api.pyx)
set(API_C_FILE ${CMAKE_CURRENT_SOURCE_DIR}/api.c)


find_package(Python3 COMPONENTS Interpreter Development)


set(PROJECT_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/py.c
	$<$<BOOL:${INCLUDE_API_MODULE}>:${API_C_FILE}>
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
	$<$<BOOL:${INCLUDE_COMMONSYMS}>:${MAX_SDK_INCLUDES}/common/commonsyms.c>
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
	$<$<BOOL:${INCLUDE_COMMONSYMS}>:-DINCLUDE_COMMONSYMS>
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

if(INCLUDE_API_MODULE AND REGEN_API_MODULE)

add_custom_command(
	OUTPUT ${API_C_FILE}
	DEPENDS ${API_PYX_FILE}
	COMMAND cython -3 --timestamps -E INCLUDE_NUMPY=${CYTHON_INCL_NUMPY} ${API_PYX_FILE}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMENT "Generating api.c"
)

add_custom_target(api_c
	ALL
	DEPENDS ${API_C_FILE}
)

add_dependencies(${PROJECT_NAME} api_c)

endif()

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
