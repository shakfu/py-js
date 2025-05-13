include(${CMAKE_SOURCE_DIR}/source/scripts/cmake/max-pretarget.cmake)


python3_external(
    PROJECT_NAME ${PROJECT_NAME}
    BUILD_VARIANT ${BUILD_VARIANT}
    INCLUDE_DIRS
    	${CMAKE_CURRENT_SOURCE_DIR}/maxcpp
    	${CMAKE_SOURCE_DIR}/source/projects/cobra
	INCLUDE_COMMONSYMS
)


include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)
