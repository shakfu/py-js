include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/cmake/common.cmake)

cmake_minimum_required(VERSION 3.10)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Release")

# use IPO
option(PK_BUILD_WITH_IPO "" TRUE)
include(CheckIPOSupported)
check_ipo_supported(RESULT result)

if(result AND PK_BUILD_WITH_IPO)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(WARNING ">> IPO disabled. You will not get the best performance.")
endif()

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8 /jumptablerdata /GS-")
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

option(PK_BUILD_MODULE_LZ4 "" OFF)
option(PK_BUILD_MODULE_LIBHV "" OFF)



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

option(PK_ENABLE_OS "" OFF)
if(PK_ENABLE_OS)
    add_definitions(-DPK_ENABLE_OS=1)
endif()


add_library( 
	${PROJECT_NAME} 
	MODULE
	${PROJECT_SRC}
)

target_include_directories(
	${PROJECT_NAME}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

if(UNIX)
    target_link_libraries(
    	${PROJECT_NAME}
    	PUBLIC
    	m
    	dl
    )
    # if(PK_ENABLE_OS)
    #     target_link_libraries(${PROJECT_NAME} dl)
    # endif()
endif()



# target_compile_options(
# 	${PROJECT_NAME}
# 	PRIVATE
# 	$<$<PLATFORM_ID:Darwin>:-Wno-unused-variable>
# 	$<$<PLATFORM_ID:Darwin>:-Wno-sign-compare>
# 	$<$<PLATFORM_ID:Darwin>:-Wfatal-errors>
# 	$<$<PLATFORM_ID:Darwin>:-Wall>	
# 	$<$<PLATFORM_ID:Darwin>:-O2>
# 	$<$<PLATFORM_ID:Darwin>:-frtti>
# 	$<$<PLATFORM_ID:Darwin>:-stdlib=libc++>
# 	$<$<PLATFORM_ID:Darwin>:--std=c++17>
# 	$<$<PLATFORM_ID:Windows>:/std:c++17>
# 	$<$<PLATFORM_ID:Windows>:/EHsc>
# 	$<$<PLATFORM_ID:Windows>:/utf-8>
# 	$<$<PLATFORM_ID:Windows>:/O2>
# )


include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)
