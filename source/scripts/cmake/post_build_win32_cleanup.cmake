message(STATUS "post-build cleanup")
file(GLOB lib_files "${CMAKE_SOURCE_DIR}/support/*.lib")

foreach(lib IN_LIST ${lib_files}) 
    file(REMOVE ${lib})
endforeach()

file(REMOVE_RECURSE "${CMAKE_SOURCE_DIR}/support/include")
