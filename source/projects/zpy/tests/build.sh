PROJECT="zpy"
XCODEPROJ=${PROJECT}.xcodeproj
HELPFILE=${PROJECT}.maxhelp

echo "building the minimal zmq python server"
gcc `python3-config --cflags` -I/usr/local/include -L/usr/local/lib `python3-config --ldflags` -lpython3.9 -lzmq -o server server.c
rm -rf *.dSYM

