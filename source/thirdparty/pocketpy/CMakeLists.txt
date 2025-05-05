cmake_minimum_required(VERSION 3.10)

project(pocketpy)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(PK_BUILD_WITH_UNITY FALSE)

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8 /jumptablerdata /GS-")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /experimental:c11atomics")
    add_compile_options(/wd4267 /wd4244)

    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Ox")
        add_definitions(-DNDEBUG)
    endif()
else()
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
        add_definitions(-DNDEBUG)
    endif()

    # disable -Wshorten-64-to-32 for apple
    if(APPLE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-shorten-64-to-32")
    endif()
endif()


include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(POCKETPY_SRC 
    pocketpy.c
)

set(PK_ENABLE_OS ON)
if(PK_ENABLE_OS)
    add_definitions(-DPK_ENABLE_OS=1)
endif()

set(PK_IS_MAIN FALSE)
option(PK_BUILD_SHARED_LIB "Build shared library" OFF)
option(PK_BUILD_STATIC_LIB "Build static library" ON)
option(PK_BUILD_STATIC_MAIN "Build static main" OFF)

if(PK_BUILD_SHARED_LIB)
    message(">> Building shared library")
    add_library(${PROJECT_NAME} SHARED ${POCKETPY_SRC})
else(PK_BUILD_STATIC_LIB)
    message(">> Building static library")
    add_library(${PROJECT_NAME} STATIC ${POCKETPY_SRC})
endif()


