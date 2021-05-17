PROJECT="zmqc"
XCODEPROJ=${PROJECT}.xcodeproj
HELPFILE=${PROJECT}.maxhelp

echo "building the external"
if [[ -d ${XCODEPROJ} ]]
then
    rm -rf ${XCODEPROJ}
fi
xcodegen
xcodebuild -project ${XCODEPROJ}

# echo "building the minimal zmq server"
# clang -o zmqc_server -lzmq zmqc_server.c

# echo "copying .maxhelp to proper package location"
# cp ${HELPFILE} ../../../../help

echo "deploy"
cd ../../
make pkg


