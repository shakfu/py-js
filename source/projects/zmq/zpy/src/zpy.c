#include "ext.h"
#include "ext_obex.h"

// #include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <zmq.h>

#define ZPY_ADDRESS "tcp://localhost:5555"

typedef struct _zpy
{
	t_object ob;			 // the object itself (must be first)
	void* p_outlet_left;   	 // left outlet for msg output
	void* p_outlet_middle;   // middle outlet for error bang
	void* p_outlet_right;    // right outlet for bang indicating process end without errors

} t_zpy;

void *zpy_new(t_symbol *s, long argc, t_atom *argv);
void zpy_free(t_zpy *x);
void zpy_assist(t_zpy *x, void *b, long m, long a, char *s);
void zpy_bang(t_zpy* x);
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
	else {	// outlet
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



t_max_err zpy_eval(t_zpy* x, t_symbol* s, long argc, t_atom* argv)
{
	t_max_err err;
	char buffer[100];

    char* text = atom_getsym(argv)->s_name;
    object_post((t_object*)x, "client %s %s", s->s_name, text);

    post("Connecting to python server...");
    void* context = zmq_ctx_new();
    void* requester = zmq_socket(context, ZMQ_REQ);
    zmq_connect(requester, ZPY_ADDRESS);

    post("client sending request '%s'", text);
    zmq_send(requester, text, strlen(text), 0);

    zmq_recv(requester, buffer, 10, 0);
    post("client received response: '%s'", buffer);

    zmq_close(requester);
    zmq_ctx_destroy(context);

    if (strlen(buffer) > 0) {
    	post("client outputing response: '%s'", buffer);
        // py_handle_output(x, buffer);
        outlet_anything(x->p_outlet_left, gensym(buffer), 0, NIL);
        outlet_bang(x->p_outlet_right);
        return MAX_ERR_NONE;
    } else {
        // py_handle_error(x, "eval %s", py_argv);
        outlet_bang(x->p_outlet_middle);
        return MAX_ERR_GENERIC;
    }
}


