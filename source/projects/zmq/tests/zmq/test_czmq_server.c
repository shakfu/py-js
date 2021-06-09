//  Hello World server
#include <czmq.h>

int main (void)
{
    //  Socket to talk to clients
    zsock_t *responder = zsock_new(ZMQ_REP);
    int rc = zsock_bind(responder, "tcp://*:5555");
    assert (rc == 0);

    while (1) {
        char *str = zstr_recv(responder);
        printf("Received Hello\n");
        sleep(1);          //  Do some 'work'
        zstr_send(responder, "World");
        zstr_free(&str);
    }
    return 0;
}

