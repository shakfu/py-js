
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
option(INCLUDE_NUMPY "include numpy headers if available")
option(DEBUG "display debug variables")
option(BUILD_UNIVERSAL "build universal binaries (requires universal dependencies)")

if(BUILD_UNIVERSAL)
	set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
endif()

set(DEBUG ON)


find_package(Python3 COMPONENTS Interpreter Development)

if(DEBUG)
	message("CMAKE_LIBRARY_OUTPUT_DIRECTORY: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
	message("CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")
	message("Python3_VERSION: ${Python3_VERSION}")
	message("Python3_VERSION_MAJOR: ${Python3_VERSION_MAJOR}")
	message("Python3_VERSION_MINOR: ${Python3_VERSION_MINOR}")
	message("Python3_FOUND: ${Python3_FOUND}")
	message("Python3_Interpreter_FOUND: ${Python3_Interpreter_FOUND}")
	message("Python3_Development_FOUND: ${Python3_Development_FOUND}")

	message("Python3_EXECUTABLE: ${Python3_EXECUTABLE}")
	message("Python3_INTERPRETER_ID: ${Python3_INTERPRETER_ID}")
	message("Python3_STDARCH: ${Python3_STDARCH}")
	message("Python3_STDLIB: ${Python3_STDLIB}")
	message("Python3_SITELIB: ${Python3_SITELIB}")
	message("Python3_SITEARCH: ${Python3_SITEARCH}")
	message("Python3_SOABI: ${Python3_SOABI}")

	message("Python3_LIBRARIES: ${Python3_LIBRARIES}")
	message("Python3_INCLUDE_DIRS: ${Python3_INCLUDE_DIRS}")
	message("Python3_LINK_OPTIONS: ${Python3_LINK_OPTIONS}")
	message("Python3_LIBRARY_DIRS: ${Python3_LIBRARY_DIRS}")
	message("Python3_RUNTIME_LIBRARY_DIRS: ${Python3_RUNTIME_LIBRARY_DIRS}")
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




# variables to support custom compilation options
set(BUILD_LIB ${CMAKE_CURRENT_SOURCE_DIR}/targets/build/lib)
set(STATIC_PY ${BUILD_LIB}/python-static)
set(FRAMEWORK_PY ${BUILD_LIB}/Python.framework)



if(USE_STATIC_PY)
	message("USE_STATIC_PY : ${USE_STATIC_PY}")
	set(PY_INCLUDE "${STATIC_PY}/include/python3.${Python3_VERSION_MINOR}")
	set(PY_LIBDIRS 
		"${STATIC_PY}/lib"
		"${BUILD_LIB}/openssl/lib"
		"${BUILD_LIB}/bzip2/lib"
		"${BUILD_LIB}/xz/lib"
	)
elseif(USE_FRAMEWORK_PY)
	message("USE_FRAMEWORK_PY: ${USE_FRAMEWORK_PY}")

	set(PY_INCLUDE "${FRAMEWORK_PY}/Headers")
	set(PY_LIBDIRS 
		"${FRAMEWORK_PY}"
	)
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

# works if compiling -GXcode only (XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS: List)
# set_target_properties(${PROJECT_NAME} 
#     PROPERTIES
#     XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@loader_path/../Resources/Python.framework/Python"
# )

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

# ----------------------------------------------------------------------
# api.pyx -> cython -> api.c

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


# ---------------------------------------------------------------
# conditional steps


if(USE_STATIC_PY)

	target_link_libraries(
		${PROJECT_NAME} 
		PRIVATE
		python3.${Python3_VERSION_MINOR}
		-ldl
		-lssl
		-lbz2
		-llzma
		-lz
		-lcrypto
		"-framework CoreFoundation"
	)

	set(PY_EXT_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.mxo)

	set(PY_EXT_LIB ${PY_EXT_PATH}/Contents/Resources/lib)

	add_custom_command(TARGET ${PROJECT_NAME}
		POST_BUILD
		COMMAND mkdir -p ${PY_EXT_LIB}
		COMMAND cp -af "${STATIC_PY}/lib/python3.${Python3_VERSION_MINOR}" ${PY_EXT_LIB}
		COMMAND cp -af "${STATIC_PY}/lib/python3${Python3_VERSION_MINOR}.zip" ${PY_EXT_LIB}
		COMMENT "Add Resources" VERBATIM
	)

elseif(USE_FRAMEWORK_PY)

	target_link_libraries(
		${PROJECT_NAME} 
		PRIVATE
		${FRAMEWORK_PY}
		-ldl
		"-framework CoreFoundation"
	)

	set(PY_EXT_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.mxo)

	set(PY_EXT_RESOURCES ${PY_EXT_PATH}/Contents/Resources)

	# target_link_options(${PROJECT_NAME} 
	# 	PRIVATE
	# 	-rpath "@loader_path/../Resources/Python.framework/Python" 
	# )

	set(fwk_binary "Python.framework/Versions/3.${Python3_VERSION_MINOR}/Python")
	set(from_path "${BUILD_LIB}/${fwk_binary}")
	set(to_path "@loader_path/../Resources/${fwk_binary}")
	set(to_rpath "@rpath/${fwk_binary}")
	set(external_binary "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/py.mxo/Contents/MacOS/py")
	set(external_fwk_binary "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/py.mxo/Contents/Resources/${fwk_binary}")

	add_custom_command(TARGET ${PROJECT_NAME}
		POST_BUILD
		COMMAND mkdir -p ${PY_EXT_RESOURCES}
		COMMAND cp -af "${FRAMEWORK_PY}" ${PY_EXT_RESOURCES}
		COMMAND install_name_tool -id ${to_path} ${external_fwk_binary}
		COMMAND install_name_tool -change ${from_path} ${to_path} ${external_binary}
		DEPENDS ${from_path}
		COMMENT "Add Resources" VERBATIM
	)

	add_custom_target(external_framework
		DEPENDS ${from_path}
	)

	add_dependencies(${PROJECT_NAME} external_framework)


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



