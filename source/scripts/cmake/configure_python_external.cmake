function(configure_python_external PROJECT_NAME
    BUILD_SHARED BUILD_STATIC BUILD_FRAMEWORK BUILD_FRAMEWORK_PKG)

set(DEPS_DIR "${CMAKE_SOURCE_DIR}/build/install")
set(EXTERNAL_DIR "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.mxo")
set(EXTERNAL_RESOURCES_DIR "${EXTERNAL_DIR}/Contents/Resources")
set(SUPPORT_DIR "${CMAKE_SOURCE_DIR}/support")

if(BUILD_SHARED)
    MESSAGE(STATUS "BUILD_SHARED: ${BUILD_SHARED}")
    set(Python3_ROOT_DIR "${DEPS_DIR}/python-shared")
    set(Python3_EXECUTABLE "${Python3_ROOT_DIR}/bin/python3")
endif()

if(BUILD_STATIC)
    MESSAGE(STATUS "BUILD_STATIC: ${BUILD_STATIC}")
    set(Python3_ROOT_DIR "${DEPS_DIR}/python-static")
    set(Python3_EXECUTABLE "${Python3_ROOT_DIR}/bin/python3")
endif()

if(BUILD_FRAMEWORK)
    MESSAGE(STATUS "BUILD_FRAMEWORK: ${BUILD_FRAMEWORK}")
    set(Python3_ROOT_DIR "${DEPS_DIR}/Python.framework")
    set(Python3_EXECUTABLE "${Python3_ROOT_DIR}/Versions/Current/bin/python3")
endif()

if(BUILD_FRAMEWORK_PKG)
    MESSAGE(STATUS "BUILD_FRAMEWORK_PKG: ${BUILD_FRAMEWORK_PKG}")
    set(Python3_ROOT_DIR "${SUPPORT_DIR}/Python.framework")
    set(Python3_EXECUTABLE "${Python3_ROOT_DIR}/Versions/Current/bin/python3")
endif()

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
    $<$<BOOL:${BUILD_STATIC}>:${DEPS_DIR}/bzip2/include>
    $<$<BOOL:${BUILD_STATIC}>:${DEPS_DIR}/openssl/include>
    $<$<BOOL:${BUILD_STATIC}>:${DEPS_DIR}/xz/include>
)

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
    -DNDEBUG
    $<$<BOOL:${BUILD_STATIC}>:-DBUILD_STATIC> # help static find pyhome
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
    $<$<BOOL:${BUILD_STATIC}>:${DEPS_DIR}/bzip2/lib>
    $<$<BOOL:${BUILD_STATIC}>:${DEPS_DIR}/openssl/lib>
    $<$<BOOL:${BUILD_STATIC}>:${DEPS_DIR}/xz/lib>
)

set(STATIC_LINK_DEPS
    "$<$<PLATFORM_ID:Darwin>:-framework SystemConfiguration>"
    "$<$<PLATFORM_ID:Darwin>:-lssl>"
    "$<$<PLATFORM_ID:Darwin>:-lbz2>"
    "$<$<PLATFORM_ID:Darwin>:-llzma>"
    "$<$<PLATFORM_ID:Darwin>:-lz>"
    "$<$<PLATFORM_ID:Darwin>:-lcrypto>"
    "$<$<PLATFORM_ID:Darwin>:-lsqlite3>"
)

target_link_libraries(
    ${PROJECT_NAME} 
    PRIVATE
    "${Python3_LIBRARIES}"
    "$<$<PLATFORM_ID:Darwin>:-ldl>"
    "$<$<PLATFORM_ID:Darwin>:-framework CoreFoundation>"
    "$<$<BOOL:${BUILD_STATIC}>:${STATIC_LINK_DEPS}>"
)

if(BUILD_SHARED OR BUILD_STATIC)
file(
    COPY "${Python3_ROOT_DIR}/lib" 
    DESTINATION "${EXTERNAL_RESOURCES_DIR}"
)
endif()

if(BUILD_SHARED)
ADD_CUSTOM_COMMAND(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND strip -x "${EXTERNAL_RESOURCES_DIR}/lib/libpython3.*.dylib"
)   
endif()

if(BUILD_STATIC)
ADD_CUSTOM_COMMAND(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND rm -f "${EXTERNAL_RESOURCES_DIR}/lib/libpython3.*.a"
    COMMAND strip -x "${EXTERNAL_DIR}/Contents/MacOS/${PROJECT_NAME}"
)
endif()

if(BUILD_FRAMEWORK)
file(
    COPY "${Python3_ROOT_DIR}"
    DESTINATION "${EXTERNAL_RESOURCES_DIR}"
)
ADD_CUSTOM_COMMAND(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND strip -x "${EXTERNAL_RESOURCES_DIR}/Python.framework/Versions/Current/Python"
)
endif()

if(BUILD_FRAMEWORK_PKG)
ADD_CUSTOM_COMMAND(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND strip -x "${SUPPORT_DIR}/Python.framework/Versions/Current/Python"
)
endif()

# cleanup to prevent cache distorting conditionals in subsequent runs
# unset(BUILD_SHARED CACHE)
# unset(BUILD_STATIC CACHE)
# unset(BUILD_FRAMEWORK CACHE)
# unset(BUILD_FRAMEWORK_PKG CACHE)

endfunction()
