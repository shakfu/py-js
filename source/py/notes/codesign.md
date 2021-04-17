# Codesigning tips

## Re-sign after modifying bundle

```bash
codesign -s $DEV_ID --force --deep ./py.mxo
```

## List your dev identities

```bash
security find-identity -v -p codesigning
```

## Verify Code signing

```bash
codesign --verify --verbose ./pyjs.mxo
```

OR

```bash
spctl — assess — verbose ./pyjs.mxo
```

## Implications of signing

- cannot modify resources python library in static externals with having to re-sign manually

- makes the package format the most flexible...
