#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ext.h"
#include "ext_obex.h"

#include <zmq.h>


#define ZPY_ADDRESS "tcp://localhost:5555"
#define ZPY_REQUEST_BUFFER_SIZE 128
#define ZPY_RESPONSE_BUFFER_SIZE 512

typedef struct _zpy
{
    t_object ob;             // the object itself (must be first)
    void* p_outlet_left;     // left outlet for msg output
    void* p_outlet_middle;   // middle outlet for error bang
    void* p_outlet_right;    // right outlet for success bang

} t_zpy;

void *zpy_new(t_symbol *s, long argc, t_atom *argv);
void zpy_free(t_zpy *x);
void zpy_assist(t_zpy *x, void *b, long m, long a, char *s);
void zpy_bang(t_zpy* x);
t_max_err zpy_test(t_zpy* x, t_symbol* s);
t_max_err zpy_eval(t_zpy* x, t_symbol* s, long argc, t_atom* argv);

void* zpy_class;


void ext_main(void *moduleRef)
{
    t_class *c;

    c = class_new("zpy", (method)zpy_new, (method)zpy_free, (long)sizeof(t_zpy),
                  0L /* leave NULL!! */, A_GIMME, 0);

    /* you CAN'T call this from the patcher */
    class_addmethod(c, (method)zpy_assist, "assist", A_CANT, 0);
    class_addmethod(c, (method)zpy_bang,   "bang", 0);
    class_addmethod(c, (method)zpy_test,   "test",  A_SYM, 0);
    class_addmethod(c, (method)zpy_eval,   "eval",  A_GIMME, 0);

    class_register(CLASS_BOX, c); /* CLASS_NOBOX */
    zpy_class = c;

    post("I am the zpy object");
}

void zpy_assist(t_zpy *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) { // inlet
        sprintf(s, "I am inlet %ld", a);
    }
    else {  // outlet
        sprintf(s, "I am outlet %ld", a);
    }
}

void zpy_free(t_zpy *x)
{
    ;
}


void *zpy_new(t_symbol *s, long argc, t_atom *argv)
{
    t_zpy *x = NULL;
    long i;

    if ((x = (t_zpy *)object_alloc(zpy_class))) {
        object_post((t_object *)x, "a new %s object was instantiated: %p", s->s_name, x);
        object_post((t_object *)x, "it has %ld arguments", argc);

        for (i = 0; i < argc; i++) {
            if ((argv + i)->a_type == A_LONG) {
                object_post((t_object *)x, "arg %ld: long (%ld)", i, atom_getlong(argv+i));
            } else if ((argv + i)->a_type == A_FLOAT) {
                object_post((t_object *)x, "arg %ld: float (%f)", i, atom_getfloat(argv+i));
            } else if ((argv + i)->a_type == A_SYM) {
                object_post((t_object *)x, "arg %ld: symbol (%s)", i, atom_getsym(argv+i)->s_name);
            } else {
                object_error((t_object *)x, "forbidden argument");
            }
        }

        // outlets
        x->p_outlet_right = bangout((t_object*)x);
        x->p_outlet_middle = bangout((t_object*)x);
        x->p_outlet_left = outlet_new(x, NULL);
    }
    return (x);
}

void zpy_bang(t_zpy *x)
{
    object_post((t_object*)x, "bang zpy");
}

t_max_err communicate(t_zpy* x, char * request)
{
    post("Connecting to server…\n");
    void* context = zmq_ctx_new();
    void* requester = zmq_socket(context, ZMQ_REQ);
    zmq_connect(requester, "tcp://localhost:5555");

    post("client sending request '%s' with length: %zu", request, strlen(request));
    char response[ZPY_RESPONSE_BUFFER_SIZE];
    post("sent: %s", request);
    zmq_send(requester, request, strlen(request), 0);
    // zmq_recv(requester, response, ZPY_RESPONSE_BUFFER_SIZE, ZMQ_DONTWAIT);
    zmq_recv(requester, response, ZPY_RESPONSE_BUFFER_SIZE, 0);

    if (strlen(response) > 0) {
        post("received: '%s'", response);
        t_atom *av = NULL;
        long ac = 0;
        t_max_err err = MAX_ERR_NONE;
        err = atom_setparse(&ac, &av, response);
        outlet_anything(x->p_outlet_left, gensym("list"), ac, av);
        outlet_bang(x->p_outlet_right);
        sysmem_freeptr(av);
        zmq_close(requester);
        zmq_ctx_destroy(context);
        return err;
    } else {
        outlet_bang(x->p_outlet_middle);
        zmq_close(requester);
        zmq_ctx_destroy(context);
        return MAX_ERR_GENERIC;
    }
}


t_max_err zpy_test(t_zpy* x, t_symbol* s)
{
    char request[ZPY_REQUEST_BUFFER_SIZE];

    object_post((t_object*)x, "client test %s", s->s_name);

    sprintf(request, "test %s", s->s_name);

    return communicate(x, request);
}


t_max_err zpy_eval(t_zpy* x, t_symbol* s, long argc, t_atom* argv)
{
    char request[ZPY_REQUEST_BUFFER_SIZE];

    strcpy(request, atom_getsym(argv)->s_name);

    return communicate(x, request);
}


