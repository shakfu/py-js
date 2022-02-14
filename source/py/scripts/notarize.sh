
# REQUIRES setting environmental variables to change
# DEV_ID="Sam Dow" \
# APP_PASS="cdds-fdsa-ggas-fgwa" \
# APPLE_ID="sam.dow@icloud.com" \
# notarize.sh sign_notarize
#
# OR
# ...(same as above)
# notarize.sh staple

DEV_ID="${DEV_ID:-Jane Dow}"
APP_PASS="${APP_PASS:-xxxx-xxxx-xxxx-xxxx}"
APPLE_ID="${APPLE_ID:-jane.dow@icloud.com}"



# NO NEED TO CHANGE ANYTHING BELOW HERE
AUTHORITY="Developer ID Application: ${DEV_ID}"
ENTITLEMENTS="entitlements.plist"
BUNDLE_ID="org.me.py"



codesign_notarize_all() {
	for TARGET in py pyjs
	do
		# set local vars
		EXTERNAL="${TARGET}.mxo"
		ARCHIVE=${EXTERNAL}.zip

		echo "codesigning ${EXTERNAL}"
		codesign -s "${AUTHORITY}" \
				 --timestamp \
				 --deep \
				 -f \
				 --options runtime \
				 --entitlements=${ENTITLEMENTS} \
				 ${EXTERNAL}

		echo "creating archive: ${ARCHIVE}"
		ditto -c -k --keepParent ${EXTERNAL} ${ARCHIVE}

		echo "notarizing ${ARCHIVE}"
		xcrun altool --notarize-app \
			--file ${ARCHIVE} \
			-t osx \
			-u "${APPLE_ID}" \
			-p "${APP_PASS}" \
			-primary-bundle-id "${BUNDLE_ID}"
	done
}


cleanup_staple_all() {
	for TARGET in py pyjs
	do
		# set local vars
		EXTERNAL="${TARGET}.mxo"
		ARCHIVE=${EXTERNAL}.zip

		echo "removing archive: ${ARCHIVE}"
		rm ${ARCHIVE}

		echo "stapling signed external: ${EXTERNAL}"
		xcrun stapler staple -v ${EXTERNAL}

		# ditto -c -k --keepParent ${EXTERNAL} ${ARCHIVE}
	done
}


if [ "$1" == "sign_notarize" ]; then
	echo "codesigning and notarizing externals."
	codesign_notarize_all

elif [ "$1" == "staple" ]; then
	echo "stapling signed externals."
	cleanup_staple_all
else
	exit
fi
