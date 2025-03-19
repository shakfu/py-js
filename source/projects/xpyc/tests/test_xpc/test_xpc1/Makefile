
.PHONY: all server clean

all: server client

server: 
	@gcc -o server xpc_server.c

client: 
	@gcc -o client xpc_client.c

clean:
	@rm -f server client
