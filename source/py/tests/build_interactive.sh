

# see: https://stackoverflow.com/questions/27672572/embedding-python-in-c-linking-fails-with-undefined-reference-to-py-initialize

LDFLAGS="-L/usr/local/Cellar/readline/8.0.4/lib"
CFLAGS1="-I/usr/local/Cellar/readline/8.0.4/include"
CFLAGS2="-I/usr/local/Cellar/readline/8.0.4/include/readline"

echo "compiling $fname"
gcc -g $CFLAGS1 $CFLAGS2 `python3.7-config --cflags --ldflags` $LDFLAGS -lreadline $fname.c -o $fname
echo "cleaning up..."
rm -rf *.dSYM



