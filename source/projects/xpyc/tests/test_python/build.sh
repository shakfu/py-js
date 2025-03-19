
gcc -o test test_python.c `python3-config --cflags` `python3-config --ldflags` -lpython3.13

rm -rf *.dSYM
