cmake_minimum_required(VERSION 3.19)

string(REGEX REPLACE "(.*)/" "" THIS_FOLDER_NAME "${CMAKE_CURRENT_SOURCE_DIR}")
project(${THIS_FOLDER_NAME})

option(C74_BUILD_FAT "Build Universal Externals" OFF) # not supported (you're on your own! :-)
option(ENABLE_LTO "enable link-time / interprocedural optimization" OFF)

# grouped build options
option(BUILD_PYTHON3_CORE_EXTERNALS "Enable building python3 core externals" OFF)
option(BUILD_PYTHON3_EXPERIMENTAL_EXTERNALS "Enable building python3 experimental externals" OFF)
option(BUILD_POCKETPY_EXTERNALS "Enable building pocketpy-based externals" OFF)
option(BUILD_THIRDPARTY_EXTERNALS "Enable building third-party externals" OFF)
option(BUILD_NET_EXTERNALS "Enable building networking externals" OFF)
option(BUILD_DEMO_EXTERNALS "Enable demo (scratch) externals" OFF)

# individual build options
option(BUILD_PY_EXTERNAL "Build the py external" OFF)
option(BUILD_PYJS_EXTERNAL "Build the pyjs external" OFF)
option(BUILD_COBRA_EXTERNAL "Build the cobra external" OFF)
option(BUILD_KRAIT_EXTERNAL "Build the krait external" OFF)
option(BUILD_MAMBA_EXTERNAL "Build the mamba external" OFF)
option(BUILD_MXPY_EXTERNAL "Build the mxpy external" OFF)
option(BUILD_PKTPY_EXTERNAL "Build the pktpy external" OFF)
option(BUILD_PKTPY2_EXTERNAL "Build the pktpy2 external" OFF)
option(BUILD_ZEDIT_EXTERNAL "Build the zedit external" OFF)
option(BUILD_ZPY_EXTERNAL "Build the zpy external" OFF)
option(BUILD_ZTHREAD_EXTERNAL "Build the zthread external" OFF)
option(BUILD_JMX_EXTERNAL "Build the jmx external" OFF)
option(BUILD_MAMBO_EXTERNAL "Build the mambo external" OFF)

# micropython external build options
option(FETCH_MICROPYTHON "Download latest micropython and use that to build external" OFF)
option(BUILD_MICROPYTHON_EXTERNAL "Build the micropython external" OFF)


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


# python3 externals

if(BUILD_PYTHON3_CORE_EXTERNALS)
set(BUILD_PY_EXTERNAL ON)
set(BUILD_PYJS_EXTERNAL ON)
endif()

if(BUILD_PYTHON3_EXPERIMENTAL_EXTERNALS)
set(BUILD_COBRA_EXTERNAL ON)
set(BUILD_KRAIT_EXTERNAL ON)
set(BUILD_MAMBA_EXTERNAL ON)
set(BUILD_MAMBO_EXTERNAL ON)
set(BUILD_MXPY_EXTERNAL ON)
endif()

# python variants
if(BUILD_POCKETPY_EXTERNALS)
set(BUILD_PKTPY_EXTERNAL ON)
set(BUILD_PKTPY2_EXTERNAL ON)
endif()


# networking externals
if(BUILD_NET_EXTERNALS)
set(BUILD_ZEDIT_EXTERNAL ON)
set(BUILD_ZPY_EXTERNAL ON)
set(BUILD_ZTHREAD_EXTERNAL ON)
set(BUILD_JMX_EXTERNAL ON)
endif()


# demo externals
if(BUILD_DEMO_EXTERNALS)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/demos/cmx)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/demos/demo)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/demos/mx)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/demos/jw)
endif()

# thirdparty externals
if(BUILD_THIRDPARTY_EXTERNALS)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/thirdparty/jit.foo)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/thirdparty/jit.fill2)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/thirdparty/shell)
endif()

# single-build-options

if(BUILD_PY_EXTERNAL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/py)
endif()

if(BUILD_PYJS_EXTERNAL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/pyjs)
endif()

if(BUILD_COBRA_EXTERNAL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/cobra)
endif()

if(BUILD_KRAIT_EXTERNAL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/krait)
endif()

if(BUILD_MAMBA_EXTERNAL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/mamba)
endif()

if(BUILD_MAMBO_EXTERNAL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/mambo)
endif()

if(BUILD_MXPY_EXTERNAL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/mxpy)
endif()

if(BUILD_PKTPY_EXTERNAL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/pktpy)
endif()

if(BUILD_PKTPY2_EXTERNAL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/pktpy2)
endif()

if(BUILD_MICROPYTHON_EXTERNAL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/mpy)
endif()

if(BUILD_ZEDIT_EXTERNAL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/zedit)
endif()

if(BUILD_ZPY_EXTERNAL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/zpy)
endif()

if(BUILD_ZTHREAD_EXTERNAL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/zthread)
endif()

if(BUILD_JMX_EXTERNAL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/jmx)
endif()

