STANDALONE=$1
ARCH=`uname -m`



maybe_rm() {
    read -p "Remove ${1}? " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        rm -rf ${1}
    fi   
}


maybe_rm_fn() {
    read -p "Remove ${1}? " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        ${2} ${1}
    fi   
}


rm_framework() {
    echo "removing ${1} framework"
    rm -rf ${STANDALONE}/Contents/Frameworks/${1}.framework
}

maybe_rm_framework() {
    maybe_rm_fn ${1} rm_framework
}


rm_external() {
    echo "removing ${1} external"
    rm -rf ${STANDALONE}
}

maybe_rm_external() {
    maybe_rm_fn ${1} rm_external
}


rm_max_extension() {
    echo "removing ${1} max extension?"
    rm -rf ${STANDALONE}/Contents/Resources/C74/extensions/max/${1}.mxo
}

maybe_rm_max_extension() {
    maybe_rm_fn ${1} rm_max_extension
}


maybe_rm_jitter() {
    read -p "Remove Jitter? " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        rm -rf ${STANDALONE}/Contents/Resources/C74/extensions/jitter
        rm_framework JitterAPI
        rm_framework JitterAPIImpl
    fi
}


shrink() {
    echo "shrinking ${1} to $ARCH"
    ditto --arch $ARCH ${1} ${1}-tmp
    rm -rf $1
    mv $1-tmp $1
}

maybe_shrink() {
    read -p "Shrink Standalone ${1} to $ARCH? " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        shrink ${1}
    fi
}



# echo "removing extensions"
# rm_max_extension maxclang
# rm_max_extension maxxslt
# # maybe_rm_max_extension maxclang
# # maybe_rm_max_extension maxxslt

# echo "removing MaxPlugInScanner"
# rm -rf $1/Contents/Resources/MaxPlugInScanner


# echo "thinning standalone to architecture $ARCH"
# shrink ${STANDALONE}

# echo "remove jitter"
# rm -rf $1/Contents/Resources/C74/extensions/jitter
# rm_framework JitterAPI
# rm_framework JitterAPIImpl

# echo "remove lua"
# rm_framework MaxLua
# rm_framework MaxLuaImpl

maybe_shrink ${STANDALONE}
maybe_rm_jitter
maybe_rm_max_extension maxclang
maybe_rm_max_extension maxxslt
maybe_rm ${STANDALONE}/Contents/Resources/MaxPlugInScanner
maybe_rm_framework MaxLua
maybe_rm_framework MaxLuaImpl




