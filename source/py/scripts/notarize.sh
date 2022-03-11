
# REQUIRES setting environmental variables to change
# DEV_ID="Sam Dow" \
# APP_PASS="cdds-fdsa-ggas-fgwa" \
# APPLE_ID="sam.dow@icloud.com" \
# notarize.sh sign_notarize
#
# OR
# ...(same as above)
# notarize.sh staple

TARGETS="py pyjs"
DEV_ID="${DEV_ID:-Jane Dow}"
APP_PASS="${APP_PASS:-xxxx-xxxx-xxxx-xxxx}"
APPLE_ID="${APPLE_ID:-jane.dow@icloud.com}"
REQUEST_UUID="xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxx"


# NO NEED TO CHANGE ANYTHING BELOW HERE
AUTHORITY="Developer ID Application: ${DEV_ID}"
ENTITLEMENTS="entitlements.plist"
BUNDLE_ID="org.me.py"


codesign_only() {
	for TARGET in $TARGETS
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
	done
}

notarize_only() {
	for TARGET in $TARGETS
	do
		# set local vars
		EXTERNAL="${TARGET}.mxo"
		ARCHIVE=${EXTERNAL}.zip

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

codesign_notarize_all() {
	for TARGET in $TARGETS
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



staple_zip() {
	for TARGET in $TARGETS
	do
		# set local vars
		EXTERNAL="${TARGET}.mxo"
		ARCHIVE=${EXTERNAL}.zip

		echo "removing archive: ${ARCHIVE}"
		rm ${ARCHIVE}

		echo "stapling signed external: ${EXTERNAL}"
		xcrun stapler staple -v ${EXTERNAL}

		ditto -c -k --keepParent ${EXTERNAL} ${ARCHIVE}
	done
}

check_status() {
	xcrun altool --notarization-info ${REQUEST_UUID} \
		--username ${APPLE_ID} \
		--password ${APP_PASS}

}

check_notarize() {
    codesign -vvvv -R="notarized" --check-notarization $1
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
