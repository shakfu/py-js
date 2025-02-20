rm -rf ./build ./externals

pushd .

mkdir build
cd build
cmake .. \
	-DBUILD_PYTHON3_CORE_EXTERNALS=ON \
	-DBUILD_PYTHON3_EXPERIMENTAL_EXTERNALS=ON \
	-DBUILD_POCKETPY_EXTERNALS=ON \
	-DBUILD_DEMO_EXTERNALS=ON
make
cd ..
make sign

popd

