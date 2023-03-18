CFLAGS="--std=c++17"
LIBNAME="pocketpy"
HEADER="${LIBNAME}.hpp"
PRECOMPILED_HEADER="${HEADER}.gch"
TARGET="demo"

echo "linking header"
if [ ! -f "${HEADER}" ]; then
	ln -s ../${LIBNAME}.h ./${HEADER}
fi


if [ -f "${PRECOMPILED_HEADER}" ]; then
	echo "rm ${PRECOMPILED_HEADER}"
	rm ${PRECOMPILED_HEADER}
fi

echo "compiling ${TARGET} without pre-compiled header"
time clang++ "${CFLAGS}" -o "${TARGET}" "${TARGET}.cpp"


if [ ! -f "${PRECOMPILED_HEADER}" ]; then
	echo "pre-compile ${HEADER}"
	clang++ -c "${HEADER}" "${CFLAGS}"
fi

echo "compiling ${TARGET} with pre-compiled header ${PRECOMPILED_HEADER}"
time clang++ "${CFLAGS}" -o "${TARGET}" "${TARGET}.cpp"
