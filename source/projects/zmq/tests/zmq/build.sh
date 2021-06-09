
# brew install zeromq
clang -o test_zmq_client -lzmq test_zmq_client.c
clang -o test_zmq_server -lzmq test_zmq_server.c


# brew install czmq
clang -o test_czmq_client -lczmq -lzmq test_zmq_client.c
clang -o test_czmq_server -lczmq -lzmq test_zmq_server.c


