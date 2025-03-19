#include "ext.h"
#include "ext_obex.h"

#include <xpc/xpc.h>
#include <CoreFoundation/CoreFoundation.h>

// minimal test object

typedef struct _xpyc
{
    t_object ob;  // the object itself (must be first)
    void* outlet; // one outlet
} t_xpyc;

void* xpyc_new(t_symbol* s, long argc, t_atom* argv);
void xpyc_free(t_xpyc* x);
void xpyc_assist(t_xpyc* x, void* b, long m, long a, char* s);
void xpyc_bang(t_xpyc* x);
void xpyc_request(t_xpyc* x, t_symbol* s, int argc, t_atom* argv);

void* xpyc_class;

void ext_main(void* r)
{
    t_class* c;

    c = class_new("xpyc", (method)xpyc_new, (method)xpyc_free, (long)sizeof(t_xpyc),
                  0L /* leave NULL!! */, A_GIMME, 0);

    /* you CAN'T call this from the patcher */
    class_addmethod(c, (method)xpyc_assist,  "assist",  A_CANT,  0);
    class_addmethod(c, (method)xpyc_bang,    "bang",             0);
    class_addmethod(c, (method)xpyc_request, "request", A_GIMME, 0);

    class_register(CLASS_BOX, c); /* CLASS_NOBOX */
    xpyc_class = c;

    post("I am the xpyc object");
}

void xpyc_assist(t_xpyc* x, void* b, long m, long a, char* s)
{
    if (m == ASSIST_INLET) { // inlet
        sprintf(s, "I am inlet %ld", a);
    }
    else { // outlet
        sprintf(s, "I am outlet %ld", a);
    }
}

void xpyc_free(t_xpyc* x)
{
    ;
}

void* xpyc_new(t_symbol* s, long argc, t_atom* argv)
{
    t_xpyc* x = NULL;
    long i;

    if ((x = (t_xpyc*)object_alloc(xpyc_class))) {

        x->outlet = outlet_new(x, NULL);

    }
    return (x);
}


void xpyc_bang(t_xpyc* x)
{
    outlet_bang(x->outlet);
}


void xpyc_request(t_xpyc* x, t_symbol* s, int argc, t_atom* argv)
{
    // post("start");

    // long first_number = atom_getlong(argv);
    // long second_number = atom_getlong(argv+1);
    
    t_symbol* code = atom_getsym(argv);

    xpc_rich_error_t error;
    xpc_session_t session = xpc_session_create_xpc_service("xpyc.PythonService", NULL, 0, &error);

    // Once you have a connection to the service, create a Codable request and send it to the service.

    //     xpc_rich_error_t error;

    xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(message, "code", code->s_name);
    // xpc_dictionary_set_int64(message, "firstNumber", first_number);
    // xpc_dictionary_set_int64(message, "secondNumber", second_number);
    


    xpc_object_t reply = xpc_session_send_message_with_reply_sync(session, message, &error);
    int64_t result = xpc_dictionary_get_int64(reply, "result");

    post("Got result of calculation: %lld", result);

    xpc_session_cancel(session);

    outlet_int(x->outlet, result);
}

