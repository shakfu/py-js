# This doesn't work unless dependencies are also fat binaries

rm -rf ./build ./externals

pushd .

mkdir build
cd build
cmake -G Xcode ..
cmake --build .

popd

