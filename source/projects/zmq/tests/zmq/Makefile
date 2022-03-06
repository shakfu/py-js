
CFLAGS  = -I/usr/local/include -L/usr/local/lib
CFLAGS  += $(shell python3-config --cflags)
LDFLAGS = $(shell python3-config --ldflags)
LDFLAGS += -lpython3.9 -lczmq -lzmq


all: zmq czmq pzmq


zmq:
	@clang -o test_zmq_client -lzmq test_zmq_client.c
	@clang -o test_zmq_server -lzmq test_zmq_server.c


czmq:
	@clang -o test_czmq_client -lczmq -lzmq test_zmq_client.c
	@clang -o test_czmq_server -lczmq -lzmq test_zmq_server.c

pzmq:
	@clang $(CFLAGS) $(LDFLAGS) -o server test_pzmq_server.c
	@clang $(CFLAGS) $(LDFLAGS) -o client test_pzmq_client.c


clean:
	@rm -f test_zmq_client test_zmq_server
	@rm -f test_czmq_client test_czmq_server
	@rm -f client server
	@rm -rf *.dSYM
	@rm -rf infer-out