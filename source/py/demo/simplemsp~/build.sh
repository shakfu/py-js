PROJECT="simplemsp~"
XCODEPROJ=${PROJECT}.xcodeproj
HELPFILE=${PROJECT}.maxhelp

if [[ -d ${XCODEPROJ} ]]
then
    echo "Regenerating xcode project"
    rm -rf ${XCODEPROJ}
fi
xcodegen
xcodebuild -project ${XCODEPROJ}
cp ${HELPFILE} ../../../../help


