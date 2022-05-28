#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int main (void)
{
    printf("Connecting to python server\n");
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect(requester, "tcp://localhost:5555");

    char request[10];
    for (int i = 0; i != 10; i++) {
        char response[10];
        printf("Sending 1+%d\n", i);
        sprintf(request, "1+%d", i);
        zmq_send(requester, request, sizeof(request), 0);
        zmq_recv(requester, response, sizeof(response), 0);
        printf("Received %s\n", response);
    }
    zmq_close(requester);
    zmq_ctx_destroy(context);
    return 0;
}
