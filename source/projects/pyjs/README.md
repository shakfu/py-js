# pyjs project

Note that the source files in this projects are soft-linked from the `py` project. The reason for this is that the `py` and `pyjs` were originally developed together and there is still extensive non-cmake driven build infrastructure which assumes this.

Creating a separate folder for pyjs with its own CMakeLists.txt file means that the pyjs external will be built when `make projects` is called.

Ultimately all externals should have their own folders and documentation, but it will take some iterations to gether there.
