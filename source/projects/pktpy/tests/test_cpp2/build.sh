TARGET="demo"

echo "compiling ${TARGET} with exception support"
time clang++ --std=c++17 -fexceptions -I../.. -o "${TARGET}" "${TARGET}.cpp"

