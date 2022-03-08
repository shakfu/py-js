PROJECT="zpy"
XCODEPROJ=${PROJECT}.xcodeproj
HELPFILE=${PROJECT}.maxhelp

echo "building the external"
if [[ -d ${XCODEPROJ} ]]
then
    rm -rf ${XCODEPROJ}
fi
xcodegen
xcodebuild -project ${XCODEPROJ}

echo "building the minimal zmq python server"
gcc `python3-config --cflags` -I/usr/local/include -L/usr/local/lib `python3-config --ldflags` -lpython3.9 -lzmq -o server server.c
rm -rf *.dSYM

# echo "copying .maxhelp to proper package location"
# cp ${HELPFILE} ../../../../help

#echo "deploy"
#cd ../../
#make pkg


