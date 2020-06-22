#install_name_tool -change /usr/local/Cellar/python/3.7.7/Frameworks/Python.framework/Versions/3.7/Python @executable_path/libpython3.7.dylib python3.7

# works if the executable (like the one that launches the python innteractive interp.
# are in the same directly as libpython3.7.dylib
#install_name_tool -id @executable_path/libpython3.7.dylib libpython3.7.dylib

# this works if all the contents of support/python3.7 are in external itself
# in the same directory as py
#install_name_tool -id @loader_path/libpython3.7.dylib libpython3.7.dylib

install_name_tool -id @loader_path/../../../../support/python3.7/libpython3.7.dylib libpython3.7.dylib
