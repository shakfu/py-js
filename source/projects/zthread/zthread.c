#include "ext.h"
#include "ext_obex.h"
#include "ext_systhread.h"

#include <string.h>
#include <unistd.h>
#include <zmq.h>

// derived from max-sdk/sources/advanced/simplethread

typedef struct _zthread {
    t_object            x_ob;                   // standard max object
    t_systhread         x_systhread;            // thread reference
    t_systhread_mutex   x_mutex;                // mutual exclusion lock for threadsafety
    int                 x_systhread_cancel;     // thread cancel flag
    void                *x_qelem;               // for message passing between threads
    void                *x_outlet;              // our outlet
    int                 x_foo;                  // simple data to pass between threads
    int                 x_sleeptime;            // how many milliseconds to sleep
} t_zthread;

void zthread_bang(t_zthread *x);
void zthread_foo(t_zthread *x, long foo);
void zthread_sleeptime(t_zthread *x, long sleeptime);
void zthread_stop(t_zthread *x);
void zthread_cancel(t_zthread *x);
void *zthread_threadproc(t_zthread *x);
void zthread_qfn(t_zthread *x);
void zthread_assist(t_zthread *x, void *b, long m, long a, char *s);
void zthread_free(t_zthread *x);
void *zthread_new(void);

t_class *zthread_class;

void ext_main(void *r)
{
    t_class *c;

    c = class_new("zthread", (method)zthread_new, (method)zthread_free, sizeof(t_zthread), 0L, 0);

    class_addmethod(c, (method)zthread_bang,        "bang",         0);
    class_addmethod(c, (method)zthread_foo,         "foo",          A_DEFLONG, 0);
    class_addmethod(c, (method)zthread_sleeptime,   "sleeptime",    A_DEFLONG, 0);
    class_addmethod(c, (method)zthread_cancel,      "cancel",       0);
    class_addmethod(c, (method)zthread_assist,      "assist",       A_CANT, 0);

    class_register(CLASS_BOX,c);
    zthread_class = c;
}

void zthread_bang(t_zthread *x)
{
    zthread_stop(x);        // kill thread if, any

    // create new thread + begin execution
    if (x->x_systhread == NULL) {
        post("starting a new thread");
        systhread_create((method) zthread_threadproc, x, 0, 0, 0, &x->x_systhread);
    }
}

void zthread_foo(t_zthread *x, long foo)
{
    systhread_mutex_lock(x->x_mutex);
    x->x_foo = (int)foo;    // override our current value
    systhread_mutex_unlock(x->x_mutex);
}

void zthread_sleeptime(t_zthread *x, long sleeptime)
{
    if (sleeptime<10)
        sleeptime = 10;
    x->x_sleeptime = (int)sleeptime;    // no need to lock since we are readonly in worker thread
}


void zthread_stop(t_zthread *x)
{
    unsigned int ret;

    if (x->x_systhread) {
        post("stopping our thread");
        x->x_systhread_cancel = true;               // tell the thread to stop
        systhread_join(x->x_systhread, &ret);       // wait for the thread to stop
        x->x_systhread = NULL;
    }
}


void zthread_cancel(t_zthread *x)
{
    zthread_stop(x);                                    // kill thread if, any
    outlet_anything(x->x_outlet, gensym("cancelled"), 0, NULL);
}

void* zthread_threadproc(t_zthread* x)
{

    post("Connecting to hello world server…\n");
    void* context = zmq_ctx_new();
    void* requester = zmq_socket(context, ZMQ_REQ);
    zmq_connect(requester, "tcp://localhost:5555");

    int request_nbr = 0;
    post("request_nbr: %i", request_nbr);
    for (request_nbr = 0; request_nbr != 10; request_nbr++) {
        // if (x->x_systhread_cancel)
        //  break;
        char buffer[10];
        post("Sending Hello %d…\n", request_nbr);
        zmq_send(requester, "Hello", 5, 0);
        zmq_recv(requester, buffer, 10, 0);
        post("Received (%d): %s\n", request_nbr, buffer);
    }
    zmq_close(requester);
    zmq_ctx_destroy(context);

    systhread_mutex_lock(x->x_mutex);
    x->x_foo++; // fiddle with shared data
    systhread_mutex_unlock(x->x_mutex);

    qelem_set(x->x_qelem); // notify main thread using qelem mechanism

    systhread_sleep(x->x_sleeptime); // sleep a bit

    x->x_systhread_cancel = false; // reset cancel flag for next time, in case
    // the thread is created again

    systhread_exit(0); // this can return a value to systhread_join();
    return NULL;
}

// triggered by the helper thread
void zthread_qfn(t_zthread *x)
{
    int myfoo;

    systhread_mutex_lock(x->x_mutex);
    myfoo = x->x_foo;                                                           // access shared data
    systhread_mutex_unlock(x->x_mutex);

    // *never* wrap outlet calls with systhread_mutex_lock()
    outlet_int(x->x_outlet, myfoo);
}

void zthread_assist(t_zthread *x, void *b, long m, long a, char *s)
{
    if (m==1)
        sprintf(s,"bang starts a new thread");
    else if (m==2)
        sprintf(s,"report when done/cancelled");
}

void zthread_free(t_zthread *x)
{
    // stop our thread if it is still running
    zthread_stop(x);

    // free our qelem
    if (x->x_qelem)
        qelem_free(x->x_qelem);

    // free out mutex
    if (x->x_mutex)
        systhread_mutex_free(x->x_mutex);
}

void *zthread_new(void)
{
    t_zthread *x;

    x = (t_zthread *)object_alloc(zthread_class);
    x->x_outlet = outlet_new(x,NULL);
    x->x_qelem = qelem_new(x,(method)zthread_qfn);
    x->x_systhread = NULL;
    systhread_mutex_new(&x->x_mutex,0);
    x->x_foo = 0;
    x->x_sleeptime = 1000;

    return(x);
}

