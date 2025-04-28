# Copyright 2018 The Min-API Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.

if (${CMAKE_GENERATOR} MATCHES "Xcode")
        if (${XCODE_VERSION} VERSION_LESS 10)
              message(STATUS "Xcode 10 or 11 is required. Please install from the Mac App Store")
            return ()
        endif ()
endif ()


set(C74_MAX_SDK_DIR ${CMAKE_SOURCE_DIR}/source/max-sdk-base)

if (APPLE)
    if (CMAKE_OSX_ARCHITECTURES STREQUAL "")
        set(CMAKE_OSX_ARCHITECTURES x86_64)
    endif()
	set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "Minimum OS X deployment version" FORCE)
endif ()

include(${C74_MAX_SDK_DIR}/script/max-pretarget.cmake)

set(C74_INCLUDES "${MAX_SDK_INCLUDES}" "${MAX_SDK_MSP_INCLUDES}" "${MAX_SDK_JIT_INCLUDES}" "${MAX_SDK_INCLUDES}/.." "${CMAKE_CURRENT_LIST_DIR}/../include")
file(GLOB_RECURSE C74_MIN_HEADERS ${CMAKE_CURRENT_LIST_DIR}/../include/*.h)

add_definitions(-DC74_MIN_API)

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../min-lib")
    message(STATUS "Min-Lib found")
    add_definitions(
        -DC74_USE_MIN_LIB
    )
endif()
