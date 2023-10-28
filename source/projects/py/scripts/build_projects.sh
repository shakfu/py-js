rm -rf ./build ./externals

pushd .

mkdir build
cd build
cmake -GXcode ..
cmake --build . --config Release

popd

