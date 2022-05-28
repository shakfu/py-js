STANDALONE=$1
ARCH=`uname -m`

fix_pyjs_standalone() {
    cp -rf javascript $1/Contents/Resources/C74
    cp -rf jsextensions $1/Contents/Resources/C74
    cp -af externals/pyjs.mxo $1/Contents/Resources/C74/externals
}

echo "fixing pyjs standalone"
fix_pyjs_standalone ${STANDALONE}
