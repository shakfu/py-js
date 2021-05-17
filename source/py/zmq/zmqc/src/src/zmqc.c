/**
	@file
	zmqc - a max object shell
	jeremy bernstein - jeremy@bootsquad.com

	@ingroup	examples
*/


#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object

// #include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <zmq.h>


typedef struct _zmqc
{
	t_object ob;	// the object itself (must be first)
} t_zmqc;

void *zmqc_new(t_symbol *s, long argc, t_atom *argv);
void zmqc_free(t_zmqc *x);
void zmqc_assist(t_zmqc *x, void *b, long m, long a, char *s);
void zmqc_bang(t_zmqc* x);
int call_server(void);


void* zmqc_class;


void ext_main(void *moduleRef)
{
	t_class *c;

	c = class_new("zmqc", (method)zmqc_new, (method)zmqc_free, (long)sizeof(t_zmqc),
				  0L /* leave NULL!! */, A_GIMME, 0);

	/* you CAN'T call this from the patcher */
	class_addmethod(c, (method)zmqc_assist,	"assist", A_CANT, 0);
    class_addmethod(c, (method)zmqc_bang,   "bang", 0);

    class_register(CLASS_BOX, c); /* CLASS_NOBOX */
	zmqc_class = c;

	post("I am the zmqc object");
}

void zmqc_assist(t_zmqc *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { // inlet
		sprintf(s, "I am inlet %ld", a);
	}
	else {	// outlet
		sprintf(s, "I am outlet %ld", a);
	}
}

void zmqc_free(t_zmqc *x)
{
	;
}


void *zmqc_new(t_symbol *s, long argc, t_atom *argv)
{
	t_zmqc *x = NULL;
	long i;

	if ((x = (t_zmqc *)object_alloc(zmqc_class))) {
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
	}
	return (x);
}

void zmqc_bang(t_zmqc *x)
{
    call_server();
	object_post((t_object*)x, "hello max");
}


int call_server(void)
{
	post("Connecting to hello world server...\n");
	void* context = zmq_ctx_new();
	void* requester = zmq_socket(context, ZMQ_REQ);
	zmq_connect(requester, "tcp://localhost:5555");

	int request_nbr;
	for (request_nbr = 0; request_nbr != 10; request_nbr++) {
		char buffer[10];
		printf("Sending Hello %d...\n", request_nbr);
		zmq_send(requester, "Hello", 5, 0);
		zmq_recv(requester, buffer, 10, 0);
        post("Received (%d): '%s'\n", request_nbr, buffer);
    }
	zmq_close(requester);
	zmq_ctx_destroy(context);
	return 0;
}
