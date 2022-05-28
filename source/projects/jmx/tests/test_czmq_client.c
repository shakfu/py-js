//  Hello World client
#include <czmq.h>

int main (void)
{
    printf ("Connecting to hello world server…\n");
    zsock_t *requester = zsock_new (ZMQ_REQ);
    zsock_connect (requester, "tcp://localhost:5555");

    int request_nbr;
    for (request_nbr = 0; request_nbr != 10; request_nbr++) {
        printf ("Sending Hello %d…\n", request_nbr);
        zstr_send (requester, "Hello");
        char *str = zstr_recv (requester);
        printf ("Received World %d\n", request_nbr);
        zstr_free (&str);
    }
    zsock_destroy (&requester);
    return 0;
}

