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
void xpyc_request(t_xpyc* x, t_symbol* s);

void* xpyc_class;

void ext_main(void* r)
{
    t_class* c;

    c = class_new("xpyc", (method)xpyc_new, (method)xpyc_free, (long)sizeof(t_xpyc),
                  0L /* leave NULL!! */, A_GIMME, 0);

    /* you CAN'T call this from the patcher */
    class_addmethod(c, (method)xpyc_assist,  "assist",  A_CANT,  0);
    class_addmethod(c, (method)xpyc_bang,    "bang",             0);
    class_addmethod(c, (method)xpyc_request, "request", A_SYM,   0);

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
    xpyc_request(x, gensym("XPYC_EXIT"));
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


typedef enum {
    XPYC_TYPE_NONE,
    XPYC_TYPE_CONNECTION,
    XPYC_TYPE_ENDPOINT,
    XPYC_TYPE_BOOL,
    XPYC_TYPE_INT64,
    XPYC_TYPE_UINT64,
    XPYC_TYPE_DOUBLE,
    XPYC_TYPE_DATE,
    XPYC_TYPE_DATA,
    XPYC_TYPE_STRING,
    XPYC_TYPE_UUID,
    XPYC_TYPE_FD,
    XPYC_TYPE_SHMEM,
    XPYC_TYPE_ARRAY,
    XPYC_TYPE_DICTIONARY,
    XPYC_TYPE_ERROR,
    XPYC_TYPE_RICH_ERROR,
    XPYC_TYPE_SYSTEM_COMMAND
} ValueType;

const char* xpyc_get_type(ValueType type_id)
{
    switch (type_id) {
    case XPYC_TYPE_NONE: return "NONE";
        break;
    case XPYC_TYPE_CONNECTION: return "CONNECTION";
        break;
    case XPYC_TYPE_ENDPOINT: return "ENDPOINT";
        break;
    case XPYC_TYPE_BOOL: return "BOOL";
        break;
    case XPYC_TYPE_INT64: return "INT64";
        break;
    case XPYC_TYPE_UINT64: return "UINT64";
        break;
    case XPYC_TYPE_DOUBLE: return "DOUBLE";
        break;
    case XPYC_TYPE_DATE: return "DATE";
        break;
    case XPYC_TYPE_DATA: return "DATA";
        break;
    case XPYC_TYPE_STRING: return "STRING";
        break;
    case XPYC_TYPE_UUID: return "UUID";
        break;
    case XPYC_TYPE_FD: return "FD";
        break;
    case XPYC_TYPE_SHMEM: return "SHMEM";
        break;
    case XPYC_TYPE_ARRAY: return "ARRAY";
        break;
    case XPYC_TYPE_DICTIONARY: return "DICTIONARY";
        break;
    case XPYC_TYPE_ERROR: return "ERROR";
        break;
    case XPYC_TYPE_RICH_ERROR: return "RICH_ERROR";
        break;
    default:
        return "NONE";
    }
}

void xpyc_request(t_xpyc* x, t_symbol* code)
{
    xpc_rich_error_t error;
    xpc_session_t session = xpc_session_create_xpc_service("xpyc.PythonService", NULL, 0, &error);

    // Once you have a connection to the service, create a Codable request and send it to the service.

    //     xpc_rich_error_t error;

    xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);

    xpc_dictionary_set_string(message, "code", code->s_name);

    if (code == gensym("XPYC_EXIT")) {
        goto cleanup;
    }

    xpc_object_t reply = xpc_session_send_message_with_reply_sync(session, message, &error);
    int64_t result_type = xpc_dictionary_get_int64(reply, "result_type");

    post("result_type: %s", xpyc_get_type(result_type));

    if (result_type == XPYC_TYPE_INT64) {
        int64_t result = xpc_dictionary_get_int64(reply, "result");
        post("result: %lld", result);
        outlet_int(x->outlet, result);;        
    }

    else if (result_type == XPYC_TYPE_DOUBLE) {
        double result = xpc_dictionary_get_double(reply, "result");
        post("result: %f", result);
        outlet_float(x->outlet, result);
    }

    else if (result_type == XPYC_TYPE_STRING) {
        const char* result = xpc_dictionary_get_string(reply, "result");
        post("result: %s", result);
        outlet_anything(x->outlet, gensym(result), 0, NIL);
    }

    else if (result_type == XPYC_TYPE_NONE) {
        post("result: None");
    }


cleanup:
    if (session) {
        xpc_session_cancel(session);
        xpc_release(session);
    }
}

