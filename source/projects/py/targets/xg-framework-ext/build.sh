PROJECT="py-js"
XCODEPROJ=${PROJECT}.xcodeproj
HELPFILE=${PROJECT}.maxhelp
TARGETS="py pyjs"

if [[ -d ${XCODEPROJ} ]]
then
    rm -rf ${XCODEPROJ}
fi
xcodegen

for t in ${TARGETS}
do
    xcodebuild -project ${XCODEPROJ} -target $t
done

