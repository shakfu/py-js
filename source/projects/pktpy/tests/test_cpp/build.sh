TARGET="demo"

echo "compiling ${TARGET}"
time clang++ --std=c++17 -I../.. -o "${TARGET}" "${TARGET}.cpp"

