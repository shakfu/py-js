cmake_minimum_required(VERSION 3.19)

string(REGEX REPLACE "(.*)/" "" THIS_FOLDER_NAME "${CMAKE_CURRENT_SOURCE_DIR}")
project(${THIS_FOLDER_NAME})

option(C74_BUILD_FAT "Build Universal Externals") # not supported (you're on your own! :-)

# use ccache if available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Found ccache in ${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

if (APPLE)
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
endif()

set(local_prefix "/usr/local")
if(CMAKE_HOST_APPLE) # i.e if is_macos check if homebrew is used aand if so get prefix
    execute_process(
        COMMAND brew --prefix
        OUTPUT_VARIABLE homebrew_prefix
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(DEFINED homebrew_prefix)
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

# dev only
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/cmx)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/cobra)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/demo)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/jmx)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/krait)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/mamba)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/mpy)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/mxpy)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/pktpy) 
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/py)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/pyjs)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/shell)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/zedit)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/zpy)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/zthread)

