
if(pymx IN_LIST BUILD_TARGETS)
add_subdirectory(pybind11)
endif()

if(pktpy2 IN_LIST BUILD_TARGETS OR pktpy11 IN_LIST BUILD_TARGETS)
add_subdirectory(pocketpy)
endif()
