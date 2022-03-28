# remove detritus

To remove detritus, please the following in the final script added to xcode build phases:

```bash
xattr -cr $HOME/Library/Developer/Xcode/DerivedData || echo Clear
xattr -cr "$PROJECT_DIR" || echo Clear
```

