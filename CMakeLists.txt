cmake_minimum_required(VERSION 3.19)

string(REGEX REPLACE "(.*)/" "" THIS_FOLDER_NAME "${CMAKE_CURRENT_SOURCE_DIR}")
project(${THIS_FOLDER_NAME})

option(C74_BUILD_FAT "Build Universal Externals" OFF) # not supported (you're on your own! :-)
option(ENABLE_LTO "enable link-time / interprocedural optimization" OFF)

option(BUILD_PYTHON3_CORE_EXTERNALS "Enable building python3 core externals" OFF)
option(BUILD_PYTHON3_EXPERIMENTAL_EXTERNALS "Enable building python3 experimental externals" OFF)
option(BUILD_POCKETPY_EXTERNALS "Enable building pocketpy-based externals" OFF)
option(BUILD_THIRDPARTY_EXTERNALS "Enable building third-party externals" OFF)
option(BUILD_ZMQ_EXTERNALS "Enable building zeromq externals" OFF)
option(BUILD_DEMO_EXTERNALS "Enable demo (scratch) externals" OFF)
option(BUILD_MICROPYTHON_EXTERNAL "Enable building of micropython external" OFF)
option(FETCH_MICROPYTHON "Download latest micropython and use that to build external" OFF)


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

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
        LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

# Generate a project for every folder in the "source/category" folder
# SUBDIRLIST(CATEGORY_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source)
# foreach (cat_dir ${CATEGORY_DIRS})
#   SUBDIRLIST(PROJECT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source/${cat_dir})
#   foreach (project_dir ${PROJECT_DIRS})
#     set(project_path ${CMAKE_CURRENT_SOURCE_DIR}/source/${cat_dir}/${project_dir})
#     #message("checking in: ${project_path}")
#     if (EXISTS "${project_path}/CMakeLists.txt")
#       message("Generating: ${project_dir}")
#       add_subdirectory(${project_path})
#     endif ()
#   endforeach ()
# endforeach ()

# python3 externals

if(BUILD_PYTHON3_CORE_EXTERNALS)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/py)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/pyjs)
endif()

if(BUILD_PYTHON3_EXPERIMENTAL_EXTERNALS)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/cobra)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/krait)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/mamba)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/mxpy)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/zedit)
endif()

# python variants
if(BUILD_POCKETPY_EXTERNALS)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/pktpy)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/pktpy2) # WIP
endif()

if(BUILD_MICROPYTHON_EXTERNAL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/mpy)
endif()

# zmq externals
if(BUILD_ZMQ_EXTERNALS)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/jmx)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/zpy)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/zthread)
endif()

# demo externals
if(BUILD_DEMO_EXTERNALS)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/demos/cmx)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/demos/demo)
endif()

# thirdparty externals
if(BUILD_THIRDPARTY_EXTERNALS)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/thirdparty/jit.foo)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/thirdparty/jit.fill2)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/thirdparty/shell)
endif()
