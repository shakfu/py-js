# Copyright 2018 The Min-DevKit Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.

# add_subdirectory(min-api)
# add_subdirectory(min-lib)

cmake_minimum_required(VERSION 3.0)

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/min-api)
include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)


#############################################################
# MAX EXTERNAL
#############################################################

python3_external(
    MIN_API
    PROJECT_NAME ${PROJECT_NAME}
    # BUILD_VARIANT ${BUILD_VARIANT}
)


# include_directories( 
#     "${C74_INCLUDES}"
# )


# set( SOURCE_FILES
#     ${PROJECT_NAME}.cpp
# )


# add_library( 
#     ${PROJECT_NAME} 
#     MODULE
#     ${SOURCE_FILES}
# )



include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)


#############################################################
# UNIT TEST
#############################################################

# include(${C74_MIN_API_DIR}/test/min-object-unittest.cmake)
