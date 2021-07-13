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
