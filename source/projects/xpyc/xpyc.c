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
    XPYC_TYPE_RICH_ERROR
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

void xpc_dump_type(xpc_object_t obj, const char* key)
{
    xpc_type_t obj_type = xpc_get_type(obj);

    if (obj_type == XPC_TYPE_CONNECTION) {
        post("XPC_TYPE_CONNECTION");

    }
    else if (obj_type == XPC_TYPE_ENDPOINT) {
        post("XPC_TYPE_ENDPOINT");

    }
    else if (obj_type == XPC_TYPE_BOOL) {
        post("XPC_TYPE_BOOL");
        // bool result = xpc_dictionary_get_bool(obj, key);
        // post("result: %d", result);
    }
    else if (obj_type == XPC_TYPE_INT64) {
        post("XPC_TYPE_INT64");
        // int64_t result = xpc_dictionary_get_int64(obj, key);
        // post("result: %lld", result);
    }
    else if (obj_type == XPC_TYPE_UINT64) {
        post("XPC_TYPE_UINT64");
        // uint64_t result = xpc_dictionary_get_uint64(obj, key);
        // post("result: %" PRIu64, result);
    }
    else if (obj_type == XPC_TYPE_DOUBLE) {
        post("XPC_TYPE_DOUBLE");
        // double result = xpc_dictionary_get_double(obj, key);
        // post("result: %f", result);
    }
    else if (obj_type == XPC_TYPE_DATE) {
        post("XPC_TYPE_DATE");
        // char date_string[20];
        // int64_t result = xpc_dictionary_get_date(obj, key);
        // time_t time_value = (time_t)result;
        // struct tm *local_time = localtime(&time_value);
        // if (local_time == NULL) {
        //     error("Error converting to local time");
        //     return;
        // }
        // strftime(date_string, sizeof(date_string), "%Y-%m-%d %H:%M:%S", local_time);
        // post("result: %s", date_string);
    }
    else if (obj_type == XPC_TYPE_DATA) {
        post("XPC_TYPE_DATA");
        // size_t length = 0;
        // const void * result = xpc_dictionary_get_data(obj, key, &length);
    }
    else if (obj_type == XPC_TYPE_STRING) {
        post("XPC_TYPE_STRING");
        // const char * result = xpc_dictionary_get_string(obj, key);
        // post("result: %s", result);
    }
    else if (obj_type == XPC_TYPE_UUID) {
        post("XPC_TYPE_UUID");
    }
    else if (obj_type == XPC_TYPE_FD) {
        post("XPC_TYPE_FD");
    }
    else if (obj_type == XPC_TYPE_SHMEM) {
        post("XPC_TYPE_SHMEM");
    }
    else if (obj_type == XPC_TYPE_ARRAY) {
        post("XPC_TYPE_ARRAY");
    }
    else if (obj_type == XPC_TYPE_DICTIONARY) {
        post("XPC_TYPE_DICTIONARY");
        // xpc_dictionary_apply(obj, ^(const char * _key, xpc_object_t _Nonnull value) {
        //     // Do iteration.
        //     post("key: %s", _key);
        //     post("XPC_TYPE_INT64");
        //     post("result: %lld", result);
        //     return (bool)true;
        // });
    }
    else if (obj_type == XPC_TYPE_ERROR) {
        post("XPC_TYPE_ERROR");
    }
    else if (obj_type == XPC_TYPE_RICH_ERROR) {
        post("XPC_TYPE_RICH_ERROR");
    }
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

    xpc_session_cancel(session);

    // outlet_int(x->outlet, result);
}

