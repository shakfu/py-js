cmake_minimum_required(VERSION 3.0)

set(C74_MAX_SDK_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base)
set(C74_BUILD_MAX_EXTENSION true)
include(${C74_MAX_SDK_BASE_DIR}/script/max-pretarget.cmake)


include_directories( 
	"${C74_INCLUDES}"
)


add_library( 
	${PROJECT_NAME} 
	MODULE
	${MAX_SDK_INCLUDES}/common/commonsyms.c
	ext_test.cpp
	ext_test.h
	oscar.c
	oscar.h
	test.assert.cpp
	test.db.cpp
	test.equals.cpp
	test.log.cpp
	test.master.c
	test.port.cpp
	test.runner.c
	test.sample~.cpp
	test.terminate.cpp
	test.unit.c
)


include(${C74_MAX_SDK_BASE_DIR}/script/max-posttarget.cmake)

