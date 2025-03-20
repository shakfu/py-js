# xcodebuild

To set output of product use `BUILD_DIR` and `SYMROOT` for object dir

For example 

```sh
xcodebuild \
	-arch arm64 \
	-project PythonService.xcodeproj \
	SYMROOT=$(HOME)/projects/py-js/build/python_service \
	BUILD_DIR=$(HOME)/projects/py-js/build 

xcodebuild 
       -project <projectname> 
       -exportPath <destinationpath 
       -xcconfig PATH
       -output PATH
       -scheme <schemeName>
       -destination <destinationspecifier>
       -arch <architecture>
       -target NAME
       BUILD_DIR=~/projects/py-js/build

xcodebuild -arch arm64 -project PythonService.xcodeproj

```