cmake_minimum_required(VERSION 3.19)

string(REGEX REPLACE "(.*)/" "" THIS_FOLDER_NAME "${CMAKE_CURRENT_SOURCE_DIR}")
project(${THIS_FOLDER_NAME})

option(C74_BUILD_FAT "Build Universal Externals" OFF) # not supported (you're on your own! :-)
option(ENABLE_LTO "enable link-time / interprocedural optimization" OFF)

# grouped build options
option(BUILD_PYTHON3_CORE_EXTERNALS "Enable building python3 core externals" OFF)
option(BUILD_PYTHON3_EXPERIMENTAL_EXTERNALS "Enable building python3 experimental externals" OFF)
option(BUILD_POCKETPY_EXTERNALS "Enable building pocketpy-based externals" OFF)
option(BUILD_NET_EXTERNALS "Enable building networking externals" OFF)
option(BUILD_DEMO_EXTERNALS "Enable demo (scratch) externals" OFF)

# micropython external build options
option(FETCH_MICROPYTHON "Download latest micropython and use that to build external" OFF)
option(BUILD_MICROPYTHON_EXTERNAL "Build the micropython external" OFF)

# campatible with 3.5
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# common string variables
set(THIRDPARTY_DEPS ${CMAKE_SOURCE_DIR}/source/thirdparty)

# string options
set(BUILD_VARIANT "" CACHE STRING "specify build variant")
set(BUILD_TARGETS "" CACHE STRING "specify build targets")

# use ccache if available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Found ccache in ${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

if (ENABLE_LTO_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_lto_supported OUTPUT check_lto_ipo_error)
    if(ipo_lto_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
        message(STATUS "IPO / LTO enabled")
    else()
        message(STATUS "IPO / LTO not supported: <${check_lto_ipo_error}>")
    endif()
endif()

message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if (${CMAKE_GENERATOR} MATCHES "Xcode")
            if (${XCODE_VERSION} VERSION_LESS 10)
                message(STATUS "Xcode 10 or higher is required. Please install from the Mac App Store.")
                return ()
            # elseif(${XCODE_VERSION} VERSION_GREATER_EQUAL 12)
            #     set(C74_BUILD_FAT YES)
            endif ()
    endif ()

    if (NOT CMAKE_OSX_ARCHITECTURES)
        if(C74_BUILD_FAT)
            set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "macOS architecture" FORCE)
        else()
            set(CMAKE_OSX_ARCHITECTURES ${CMAKE_SYSTEM_PROCESSOR} CACHE STRING "macOS architecture" FORCE)
        endif()
        message("CMAKE_OSX_ARCHITECTURES set to ${CMAKE_OSX_ARCHITECTURES}")
    endif()

    # i.e check if homebrew is used, set local prefix accordingly
    set(local_prefix "/usr/local")
    execute_process(
        COMMAND brew --prefix
        OUTPUT_VARIABLE homebrew_prefix
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(DEFINED homebrew_prefix)
        set(HAS_HOMEBREW 1)
        set(local_prefix ${homebrew_prefix})
    endif()
endif()

# include functions to use later
include(${CMAKE_SOURCE_DIR}/source/scripts/cmake/fn_python3_external.cmake)


if(BUILD_PYTHON3_CORE_EXTERNALS)
list(APPEND BUILD_TARGETS py pyjs)
endif()

if(BUILD_PYTHON3_EXPERIMENTAL_EXTERNALS)
list(APPEND BUILD_TARGETS krait cobra mamba mxpy pyx mpyx pymx)
endif()

if(BUILD_POCKETPY_EXTERNALS)
list(APPEND BUILD_TARGETS pktpy pktpy2)
endif()

if(BUILD_NETWORKING_EXTERNALS)
list(APPEND BUILD_TARGETS zedit zpy ztp jmx)
endif()

set(DEMO_EXTERNALS cmx demo mx)
if(BUILD_DEMO_EXTERNALS)
list(APPEND BUILD_TARGETS ${DEMO_EXTERNALS})
endif()

if(BUILD_MICROPYTHON_EXTERNAL)
list(APPEND BUILD_TARGETS mpy)
endif()


# build shared thirdparty dependencies
add_subdirectory(${THIRDPARTY_DEPS})


foreach(proj ${BUILD_TARGETS})
    message(STATUS "to build: ${proj}")
    if(proj IN_LIST DEMO_EXTERNALS)
        add_subdirectory(${CMAKE_SOURCE_DIR}/source/demos/${proj})
    else()
        add_subdirectory(${CMAKE_SOURCE_DIR}/source/projects/${proj})
    endif()
endforeach()

# cache cleanup at end of build
unset(C74_BUILD_FAT CACHE)
unset(ENABLE_LTO CACHE)
unset(BUILD_TARGETS CACHE)
unset(BUILD_VARIANT CACHE)
unset(BUILD_PYTHON3_CORE_EXTERNALS CACHE)
unset(BUILD_PYTHON3_EXPERIMENTAL_EXTERNALS CACHE)
unset(BUILD_POCKETPY_EXTERNALS CACHE)
unset(BUILD_NET_EXTERNALS CACHE)
unset(BUILD_DEMO_EXTERNALS CACHE)
unset(FETCH_MICROPYTHON CACHE)
unset(BUILD_MICROPYTHON_EXTERNAL CACHE)
