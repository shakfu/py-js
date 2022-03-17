# Codesigning and Notarization


## How to codesign and notarize Max externals

- John Gibson forum [post](https://cycling74.com/forums/osx-mxo-notarization-headache): 

	> "Okay, this is probably solved. I was able to sign and notarize just the Mac external in my package, not the package folder, which makes sense. As part of doing this, you have to zip up the external .mxo bundle before uploading to the Apple server. Then I was able to staple the ticket to my unzipped bundle.
	I had not understood that you can (must) zip the bundle before uploading for notarization, but that you do not have to staple the ticket to the zip file itself. I didn't want that, because of course I didn't want my external to remain zipped within the package folder.
	I just need to test this on another machine running Catalina (I'm still on Mojave...) to be sure it loads in Max."

- This[post](https://cycling74.com/forums/apple-notarizing-for-mojave-10-14-and-beyond) has very useful information.


- [Max 8.1: Mac OS 10.15 Catalina Support and Notarization](https://cycling74.com/articles/max-8-1-mac-os-10-15-catalina-support-and-notarization/)



## Tips

### Re-sign after modifying bundle

```bash
codesign -s $DEV_ID --force --deep ./py.mxo
```

### List your dev identities

```bash
security find-identity -v -p codesigning
```

### Verify Code signing

```bash
codesign --verify --verbose ./pyjs.mxo
```

OR

```bash
spctl — assess — verbose ./pyjs.mxo
```

### Implications of signing

- cannot modify resources python library in static externals with having to re-sign manually

- makes the package format the most flexible...


### SOLVED


```python


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
```