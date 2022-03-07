//  Hello World client
#include <czmq.h>

int main(void)
{
    printf("Connecting to python server…\n");
    zsock_t *requester = zsock_new(ZMQ_REQ);
    zsock_connect (requester, "tcp://localhost:5555");

    char result[100];
    for (int i = 0; i != 10; i++) {
        printf("Sending 1+%d\n", i);
        sprintf(result, "1+%d", i);
        zstr_send(requester, result);
        char *str = zstr_recv(requester);
        printf ("Received: %s\n", str);
        zstr_free (&str);
    }
    zsock_destroy (&requester);
    return 0;
}

