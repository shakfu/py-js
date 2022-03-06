
# brew install zeromq
clang -o test_zmq_client -lzmq test_zmq_client.c
clang -o test_zmq_server -lzmq test_zmq_server.c


# # brew install czmq
clang -o test_czmq_client -lczmq -lzmq test_zmq_client.c
clang -o test_czmq_server -lczmq -lzmq test_zmq_server.c


clang -I/usr/local/include -L/usr/local/lib `python3-config --cflags --ldflags` -lpython3.9 -lczmq -lzmq -o server test_pzmq_server.c
clang -I/usr/local/include -L/usr/local/lib `python3-config --cflags --ldflags` -lpython3.9 -lczmq -lzmq -o client test_pzmq_client.c
