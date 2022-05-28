# attempt to build without xcode (see notes/experiments.md)
# it seems to compile ok 

PRODUCT_NAME=tst
PRODUCT_VERSION=8.0.2
ARCHS=x86_64

SRCROOT=`pwd`
C74SUPPORT=${SRCROOT}/../../max-sdk/source/c74support
HEADER_SEARCH_PATHS="-I ${C74SUPPORT}/max-includes -I ${C74SUPPORT}/msp-includes -I ${C74SUPPORT}/jit-includes"
FRAMEWORK_SEARCH_PATHS="-F ${C74SUPPORT}/max-includes -F ${C74SUPPORT}/msp-includes -F ${C74SUPPORT}/jit-includes"
# DSTROOT=${SRCROOT}/../../../externals
# (This next path is relative to DSTROOT}
# INSTALL_PATH=.

# GCC_PREFIX_HEADER=${C74SUPPORT}/max-includes/macho-prefix.pch
# INFOPLIST_FILE=${SRCROOT}/../../Info.plist


ARCHS=x86_64

# SDKROOT = macosx10.6
MACOSX_DEPLOYMENT_TARGET=10.9

# Preprocessor Defines
#GCC_PREPROCESSOR_DEFINITIONS="DENORM_WANT_FIX=1" "NO_TRANSLATION_SUPPORT=1"


# Static Configuration (dont change these}
# WRAPPER_EXTENSION=mxo;
# WARNING_CFLAGS="-Wmost -Wno-four-char-constants -Wno-unknown-pragmas"
# DEPLOYMENT_LOCATION=YES
# GENERATE_PKGINFO_FILE=YES
#SDK="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX11.1.sdk"
SDK="/Library/Developer/CommandLineTools/SDKs/MacOSX11.1.sdk"
#SDK="/Library/Developer/CommandLineTools/SDKs/MacOSX11.1.sdk/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers"

# Flags to enforce some build-time checks for the symbols used while not actually performing a hard link
C74_SYM_LINKER_FLAGS=$(<${SRCROOT}/c74-link-flags.txt)

# hide all symbols by default
# mark a function to be exported with the C74_EXPORT macro
# most likely this will only apply to the ext_main(} function which we've already prototyped for you
OTHER_CFLAGS="-fvisibility=hidden -mmacosx-version-min=10.9 -arch x86_64 -isysroot ${SDK}"

OTHER_LDFLAGS="-framework MaxAudioAPI -framework JitterAPI ${C74_SYM_LINKER_FLAGS} -mmacosx-version-min=10.9"

EXTRA_HEADERS="-I ${SDK}/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers"

echo "HELLO"

SDK="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX11.1.sdk"

VERSION="3.9"
PREFIX="/usr/local/opt/python@3.9/Frameworks/Python.framework/Versions/${VERSION}"

PY_HEADERS="-I${PREFIX}/include/python${VERSION}"
PY_LIBS="-L${PREFIX}/lib"
PY_LDFLAGS="-lpython${VERSION} -ldl"



MACOSX_DEPLOYMENT_TARGET=10.9 \
    clang \
        ${OTHER_CFLAGS} \
        ${OTHER_LDFLAGS} \
        ${PY_LIBS} \
        ${HEADER_SEARCH_PATHS} \
        ${PY_HEADERS} \
        ${EXTRA_HEADERS} \
        ${PY_LDFLAGS} \
        ${FRAMEWORK_SEARCH_PATHS} \
        -framework CoreFoundation \
        -o test_sdk api.c test_mxsdk.c

# note: -framework CoreService may or may not be required