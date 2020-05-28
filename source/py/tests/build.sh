# see: https://stackoverflow.com/questions/27672572/embedding-python-in-c-linking-fails-with-undefined-reference-to-py-initialize


echo "compiling..."

for fname in test_goto
do
    echo "compiling $fname"
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    clang -g -I /usr/include/python3.8 -lpython3.8 $fname.c -o $fname
	elif [[ "$OSTYPE" == "darwin"* ]]; then
    gcc -g `python3.7-config --cflags --ldflags` $fname.c -o $fname
	else
		echo "not implemented"
	fi
done

echo "cleaning up..."
rm -rf *.dSYM

