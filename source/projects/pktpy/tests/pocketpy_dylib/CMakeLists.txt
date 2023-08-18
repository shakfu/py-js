
set(CMAKE_BUILD_TYPE "Release")

add_library( 
	dsp
	SHARED
	dsp.c
	pocketpy_c.c
)

target_include_directories(
	dsp
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)
