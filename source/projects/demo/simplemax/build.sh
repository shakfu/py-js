PROJECT="simplemax"
XCODEPROJ=${PROJECT}.xcodeproj
HELPFILE=${PROJECT}.maxhelp

if [[ -d ${XCODEPROJ} ]]
then
    rm -rf ${XCODEPROJ}
fi
xcodegen
xcodebuild -project ${XCODEPROJ}
#cp ${HELPFILE} ../../../../help
#cd ../../
#make pkg

