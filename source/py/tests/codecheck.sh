infer run -- gcc `python3.7-config --cflags --ldflags` $1 -o $(basename $1 .c)
