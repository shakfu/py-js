include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)

#############################################################
# MAX EXTERNAL
#############################################################

# ---------------------------------------------------------------
# common steps

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")

set(CMAKE_EXPORT_COMPILE_COMMANDS True)

find_package(Python3 COMPONENTS Interpreter Development)

set(PY_DEBUG "FALSE")

# skipped unless PYDEBUG is defined earlier
if(DEFINED PY_DEBUG)
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
#      "*.h"
# 	 "*.c"
#      "*.cpp"
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


add_library( 
	${PROJECT_NAME} 
	MODULE
	${PROJECT_SRC}
)

# if custom static compilation option is available
set(STATICPY ${CMAKE_CURRENT_SOURCE_DIR}/build/lib/python-static)

if(EXISTS ${STATICPY})
	set(PY_INCLUDE "${STATICPY}/include/python3.9")
	set(PY_LIBDIRS "${STATICPY}/lib")
else()
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
	COMMAND cython -3 api.pyx
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



if(EXISTS ${STATICPY})

	find_library(STATICINTL libintl.a)

	target_link_libraries(
		${PROJECT_NAME} 
		PRIVATE
		python3.9
		"${STATICINTL}"
		"-liconv"
		"-lz"
		"-ldl"
		"-framework CoreFoundation"
	)

	set(PY_EXT_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.mxo)

	set(PY_EXT_LIB ${PY_EXT_PATH}/Contents/Resources/lib)

	add_custom_command(TARGET ${PROJECT_NAME}
		POST_BUILD
		COMMAND mkdir -p ${PY_EXT_LIB}
		COMMAND cp -af ${PY_LIBDIRS}/python3.9 ${PY_EXT_LIB}
		COMMAND cp -af ${PY_LIBDIRS}/python39.zip ${PY_EXT_LIB}
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

