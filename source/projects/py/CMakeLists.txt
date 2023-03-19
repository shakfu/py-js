
# ============================================================================
# MAX EXTERNAL

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/common.cmake)

# ----------------------------------------------------------------------------
# common steps

# configuration
# set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
# set(CMAKE_EXPORT_COMPILE_COMMANDS True)
set(SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/scripts)


# options
option(USE_STATIC_PY "build static extension")
option(USE_FRAMEWORK_PY "build framework-based extension")
option(USE_NUMPY "include numpy headers if available")
option(DEBUG "display debug variables")




find_package(Python3 COMPONENTS Interpreter Development)

if(DEBUG)
	message("CMAKE_LIBRARY_OUTPUT_DIRECTORY:${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
	message("CMAKE_CURRENT_BINARY_DIR:${CMAKE_CURRENT_BINARY_DIR}")
	message("Python3_VERSION:${Python3_VERSION}")
	message("Python3_FOUND:${Python3_FOUND}")
	message("Python3_Interpreter_FOUND:${Python3_Interpreter_FOUND}")
	message("Python3_Development_FOUND:${Python3_Development_FOUND}")

	message("Python3_EXECUTABLE:${Python3_EXECUTABLE}")
	message("Python3_INTERPRETER_ID:${Python3_INTERPRETER_ID}")
	message("Python3_STDARCH:${Python3_STDARCH}")
	message("Python3_STDLIB:${Python3_STDLIB}")
	message("Python3_SITELIB:${Python3_SITELIB}")
	message("Python3_SITEARCH:${Python3_SITEARCH}")
	message("Python3_SOABI:${Python3_SOABI}")

	message("Python3_LIBRARIES:${Python3_LIBRARIES}")
	message("Python3_INCLUDE_DIRS:${Python3_INCLUDE_DIRS}")
	message("Python3_LINK_OPTIONS:${Python3_LINK_OPTIONS}")
	message("Python3_LIBRARY_DIRS:${Python3_LIBRARY_DIRS}")
	message("Python3_RUNTIME_LIBRARY_DIRS:${Python3_RUNTIME_LIBRARY_DIRS}")
endif()


# file(GLOB PROJECT_SRC
#    "*.h"
# 	 "*.c"
#    "*.cpp"
# )

set(PROJECT_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/py.c
	${CMAKE_CURRENT_SOURCE_DIR}/api.c
)

include_directories( 
	"${MAX_SDK_INCLUDES}"
	"${MAX_SDK_MSP_INCLUDES}"
	"${MAX_SDK_JIT_INCLUDES}"
)

if(USE_NUMPY)
	# optionally add numpy_header
	execute_process(
		COMMAND ${Python3_EXECUTABLE} ${SCRIPTS}/check_numpy.py --include
		OUTPUT_VARIABLE NUMPY_HEADERS
		RESULT_VARIABLE NUMPY_HEADERS_RESULT
	)

	message("NUMPY_HEADERS_RESULT: " ${NUMPY_HEADERS_RESULT})
	message("NUMPY_HEADERS: " ${NUMPY_HEADERS})

	if (NUMPY_HEADERS_RESULT EQUAL 0) # 0 == SUCCESS
		include_directories("${NUMPY_HEADERS}")
		add_compile_definitions(INCLUDE_NUMPY=1)
		set(CYTHON_INCL_NUMPY "1")
	else()
		set(CYTHON_INCL_NUMPY "0")
	endif()
else()
	set(CYTHON_INCL_NUMPY "0")
endif()


add_library( 
	${PROJECT_NAME} 
	MODULE
	${PROJECT_SRC}
)



# variables to support custom compilation options
set(BUILD_LIB ${CMAKE_CURRENT_SOURCE_DIR}/targets/build/lib)
set(STATIC_PY ${BUILD_LIB}/python-static)
set(FRAMEWORK_PY ${BUILD_LIB}/Python.framework)



if(USE_STATIC_PY)
	message("USE_STATIC_PY : ${USE_STATIC_PY}")
	set(PY_INCLUDE "${STATIC_PY}/include/python3.10")
	set(PY_LIBDIRS "${STATIC_PY}/lib")
elseif(USE_FRAMEWORK_PY)
	message("USE_FRAMEWORK_PY: ${USE_FRAMEWORK_PY}")
	set(PY_INCLUDE "${FRAMEWORK_PY}/Headers")
	set(PY_LIBDIRS "${FRAMEWORK_PY}/lib")
else()
	message("using standard local configuration")
	set(PY_INCLUDE "${Python3_INCLUDE_DIRS}")
	set(PY_LIBDIRS "${Python3_LIBRARY_DIRS}")
endif()

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
	${PY_INCLUDE}
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




add_custom_target(
	run_cython ALL
	COMMAND cython -3 -E INCLUDE_NUMPY=${CYTHON_INCL_NUMPY} api.pyx
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMENT "Generating api.c"
)

add_dependencies(${PROJECT_NAME} run_cython)


target_link_directories(
	${PROJECT_NAME} 
	PRIVATE
	${PY_LIBDIRS}
)


# ---------------------------------------------------------------
# conditional steps


if(USE_STATIC_PY)

	find_library(STATICINTL libintl.a)

	target_link_libraries(
		${PROJECT_NAME} 
		PRIVATE
		python3.10
		"${STATICINTL}"
		"-liconv"
		"-lz"
		"-ldl"
		# "-lssl"
		# "-lbz2"
		# "-llzma"
		# "-lcrypto"
		"-framework CoreFoundation"
	)

	set(PY_EXT_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.mxo)

	set(PY_EXT_LIB ${PY_EXT_PATH}/Contents/Resources/lib)

	add_custom_command(TARGET ${PROJECT_NAME}
		POST_BUILD
		COMMAND mkdir -p ${PY_EXT_LIB}
		COMMAND cp -af ${PY_LIBDIRS}/python3.10 ${PY_EXT_LIB}
		COMMAND cp -af ${PY_LIBDIRS}/python310.zip ${PY_EXT_LIB}
		COMMENT "Add Resources" VERBATIM
	)

else()

	target_link_libraries(
		${PROJECT_NAME} 
		PRIVATE
		"${Python3_LIBRARIES}"
		"-ldl"
		"-framework CoreFoundation"
	)

endif()




include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)

# ============================================================================



