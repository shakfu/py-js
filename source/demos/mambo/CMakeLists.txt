include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/common.cmake)

option(BUILD_DYNAMIC "Build a shared external variant" OFF)
option(BUILD_STATIC "Build a static external variant" OFF)
option(BUILD_FRAMEWORK "Build a shared framework external variant" OFF)

set(DEPS_DIR "${CMAKE_SOURCE_DIR}/build/install")
set(EXTERNAL_DIR "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.mxo")
set(EXTERNAL_RESOURCES_DIR "${EXTERNAL_DIR}/Contents/Resources")

MESSAGE(STATUS "BUILD_STATIC: ${BUILD_STATIC}")
MESSAGE(STATUS "BUILD_FRAMEWORK: ${BUILD_FRAMEWORK}")
MESSAGE(STATUS "BUILD_DYNAMIC: ${BUILD_DYNAMIC}")

if(BUILD_STATIC)
    set(Python3_ROOT_DIR "${DEPS_DIR}/python-static")
endif()

# if(BUILD_FRAMEWORK)
#     set(Python3_ROOT_DIR "${DEPS_DIR}/Python.framework")
# endif()

if(BUILD_DYNAMIC)
    set(Python3_ROOT_DIR "${DEPS_DIR}/python-shared")
endif()

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
    $<$<BOOL:${BUILD_STATIC}>:${DEPS_DIR}/bzip2/include>
    $<$<BOOL:${BUILD_STATIC}>:${DEPS_DIR}/openssl/include>
    $<$<BOOL:${BUILD_STATIC}>:${DEPS_DIR}/xz/include>
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
    $<$<BOOL:${BUILD_STATIC}>:${STATIC_LINK_DEPS}>
)

if(BUILD_DYNAMIC OR BUILD_STATIC)
file(
    COPY "${Python3_ROOT_DIR}/lib" 
    DESTINATION "${EXTERNAL_RESOURCES_DIR}"
)
endif()

if(BUILD_DYNAMIC)
ADD_CUSTOM_COMMAND(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND strip -x "${EXTERNAL_RESOURCES_DIR}/lib/libpython3.*.dylib"
)   
endif()

if(BUILD_STATIC)
ADD_CUSTOM_COMMAND(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND rm -f "${EXTERNAL_RESOURCES_DIR}/lib/libpython3.*.a"
    COMMAND strip -x "${EXTERNAL_DIR}/Contents/MacOS/mambo"
)
endif()

# if(BUILD_FRAMEWORK)
# ADD_CUSTOM_COMMAND(
#     TARGET ${PROJECT_NAME} POST_BUILD
#     COMMAND cp -a "${DEPS_DIR}/Python.framework" "${EXTERNAL_RESOURCES_DIR}/Python.framework"
#     # COMMAND strip -x "${EXTERNAL_RESOURCES_DIR}/Python.framework/Versions/Current/Python"
# )
# endif()

# cleanup to prevent cache distorting conditiols in subsequent runs
unset(BUILD_DYNAMIC CACHE)
unset(BUILD_STATIC CACHE)
unset(BUILD_FRAMEWORK CACHE)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)

