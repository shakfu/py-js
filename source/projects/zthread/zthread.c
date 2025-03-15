#include "ext.h"
#include "ext_obex.h"
#include "ext_systhread.h"

#include <string.h>
#include <unistd.h>
#include <zmq.h>

#define RESPONSE_BUFFER_SIZE 64

// derived from max-sdk/sources/advanced/simplethread

typedef struct _zt {
    t_object            x_ob;                   // standard max object
    t_systhread         x_systhread;            // thread reference
    t_systhread_mutex   x_mutex;                // mutual exclusion lock for threadsafety
    int                 x_systhread_cancel;     // thread cancel flag
    void                *x_qelem;               // for message passing between threads
    void                *x_outlet;              // our outlet
    int                 x_sleeptime;            // how many milliseconds to sleep
    t_symbol*           x_request;              // code to be evaluated remotely
    t_symbol*           x_response;             // result of remote evaluation
    int                 x_is_new;               // 1 means there's a new request
} t_zt;

void zt_bang(t_zt *x);
void zt_sleeptime(t_zt *x, long sleeptime);
void zt_stop(t_zt *x);
void zt_cancel(t_zt *x);
void *zt_threadproc(t_zt *x);
void zt_qfn(t_zt *x);
void zt_assist(t_zt *x, void *b, long m, long a, char *s);
void zt_free(t_zt *x);
void *zt_new(void);
t_max_err zt_py(t_zt* x, t_symbol* s);

t_class *zt_class;

void ext_main(void *r)
{
    t_class *c;

    c = class_new("zthread", (method)zt_new, (method)zt_free, sizeof(t_zt), 0L, 0);

    class_addmethod(c, (method)zt_bang,        "bang",         0);
    class_addmethod(c, (method)zt_py,          "py",           A_DEFSYM, 0);
    class_addmethod(c, (method)zt_sleeptime,   "sleeptime",    A_DEFLONG, 0);
    class_addmethod(c, (method)zt_cancel,      "cancel",       0);
    class_addmethod(c, (method)zt_assist,      "assist",       A_CANT, 0);

    class_register(CLASS_BOX,c);
    zt_class = c;
}

void zt_bang(t_zt *x)
{
    zt_stop(x);        // kill thread if, any

    // create new thread + begin execution
    if (x->x_systhread == NULL) {
        post("starting a new thread");
        systhread_create((method) zt_threadproc, x, 0, 0, 0, &x->x_systhread);
    }
}

t_max_err zt_py(t_zt* x, t_symbol* s)
{
    systhread_mutex_lock(x->x_mutex);
    x->x_request = s;    // override our current value
    x->x_is_new = 1;
    systhread_mutex_unlock(x->x_mutex);
}

void zt_sleeptime(t_zt *x, long sleeptime)
{
    if (sleeptime<10)
        sleeptime = 10;
    x->x_sleeptime = (int)sleeptime;    // no need to lock since we are readonly in worker thread
}


void zt_stop(t_zt *x)
{
    unsigned int ret;

    if (x->x_systhread) {
        post("stopping our thread");
        x->x_systhread_cancel = true;               // tell the thread to stop
        systhread_join(x->x_systhread, &ret);       // wait for the thread to stop
        x->x_systhread = NULL;
    }
}


void zt_cancel(t_zt *x)
{
    zt_py(x, gensym("ZTHREAD_EXIT"));

    zt_stop(x);                                    // kill thread if, any
    outlet_anything(x->x_outlet, gensym("cancelled"), 0, NULL);
}


void* zt_threadproc(t_zt* x)
{
    post("Connecting to server…\n");
    void* context = zmq_ctx_new();
    void* requester = zmq_socket(context, ZMQ_REQ);
    zmq_connect(requester, "tcp://localhost:5555");

    char buffer[RESPONSE_BUFFER_SIZE];
    t_symbol* req;
    int is_new = 0;
    while (1) {
        if (x->x_systhread_cancel) {
            post("cancelling thread process");
            break;
        }
        systhread_mutex_lock(x->x_mutex);
        req = x->x_request;
        is_new = x->x_is_new;
        systhread_mutex_unlock(x->x_mutex);
        if (is_new) {
            post("sent: %s", req->s_name);
            zmq_send(requester, req->s_name, strlen(req->s_name), 0);
            zmq_recv(requester, buffer, RESPONSE_BUFFER_SIZE, 0);
            post("received: %s", buffer);
            systhread_mutex_lock(x->x_mutex);
            x->x_response = gensym(buffer);
            x->x_is_new = 0;
            systhread_mutex_unlock(x->x_mutex);
            qelem_set(x->x_qelem);
        }
    }
    zmq_close(requester);
    zmq_ctx_destroy(context);

    systhread_sleep(x->x_sleeptime); // sleep a bit

    x->x_systhread_cancel = false; // reset cancel flag for next time, in case
    // the thread is created again

    systhread_exit(0); // this can return a value to systhread_join();
    return NULL;
}


// triggered by the helper thread
void zt_qfn(t_zt *x)
{
    t_symbol* response;

    systhread_mutex_lock(x->x_mutex);
    response = x->x_response;                                                           // access shared data
    systhread_mutex_unlock(x->x_mutex);

    // *never* wrap outlet calls with systhread_mutex_lock()
    outlet_anything(x->x_outlet, response, 0, NULL);
}

void zt_assist(t_zt *x, void *b, long m, long a, char *s)
{
    if (m==1)
        sprintf(s,"bang starts a new thread");
    else if (m==2)
        sprintf(s,"report when done/cancelled");
}

void zt_free(t_zt *x)
{
    // stop our thread if it is still running
    zt_stop(x);

    // free our qelem
    if (x->x_qelem)
        qelem_free(x->x_qelem);

    // free out mutex
    if (x->x_mutex)
        systhread_mutex_free(x->x_mutex);
}

void *zt_new(void)
{
    t_zt *x;

    x = (t_zt *)object_alloc(zt_class);
    x->x_outlet = outlet_new(x, NULL);
    x->x_qelem = qelem_new(x,(method)zt_qfn);
    x->x_systhread = NULL;
    systhread_mutex_new(&x->x_mutex,0);
    x->x_request = gensym("");
    x->x_response = gensym("");
    x->x_sleeptime = 1000;

    return(x);
}

