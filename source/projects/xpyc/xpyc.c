#include "ext.h"
#include "ext_obex.h"

#include <xpc/xpc.h>
#include <CoreFoundation/CoreFoundation.h>

// minimal test object

#define XPYC_MAX_ATOMS 1024

typedef struct _xpyc
{
    t_object ob;  // the object itself (must be first)
    void* outlet; // one outlet
} t_xpyc;

void* xpyc_new(t_symbol* s, long argc, t_atom* argv);
void xpyc_free(t_xpyc* x);
void xpyc_assist(t_xpyc* x, void* b, long m, long a, char* s);
void xpyc_bang(t_xpyc* x);
t_max_err xpyc_anything(t_xpyc* x, t_symbol* s, long argc, t_atom* argv);

void* xpyc_class;

void ext_main(void* r)
{
    t_class* c;

    c = class_new("xpyc", (method)xpyc_new, (method)xpyc_free, (long)sizeof(t_xpyc),
                  0L /* leave NULL!! */, A_GIMME, 0);

    /* you CAN'T call this from the patcher */
    class_addmethod(c, (method)xpyc_assist,     "assist",   A_CANT,  0);
    class_addmethod(c, (method)xpyc_bang,       "bang",              0);
    class_addmethod(c, (method)xpyc_anything,   "anything", A_GIMME, 0);

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
    xpyc_anything(x, gensym("XPYC_EXIT"), 0, NULL);
}

void* xpyc_new(t_symbol* s, long argc, t_atom* argv)
{
    t_xpyc* x = NULL;

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


char* str_replace(const char* s, const char* old, const char* new)
{
    char* result;
    int i, cnt = 0;
    size_t new_len = strlen(new);
    size_t old_len = strlen(old);

    // Counting the number of times old word occurs in the string
    for (i = 0; s[i] != '\0'; i++) {
        if (strstr(&s[i], old) == &s[i]) {
            cnt++;

            // Jumping to index after the old word.
            i += old_len - 1;
        }
    }

    // Making new string of enough length
    size_t maxlen = i + cnt * (new_len - old_len) + 1;
    result = (char*)sysmem_newptr(maxlen);

    i = 0;
    while (*s) {
        // compare the substring with the result
        if (strstr(s, old) == s) {
            strncpy_zero(&result[i], new, maxlen);
            i += new_len;
            s += old_len;
        } else
            result[i++] = *s++;
    }

    result[i] = '\0';
    return result;
}


t_max_err xpyc_anything(t_xpyc* x, t_symbol* s, long argc, t_atom* argv)
{
    t_atom atoms[XPYC_MAX_ATOMS];
    long textsize = 0;
    char* text = NULL;

    if (s == gensym("")) {
        return MAX_ERR_GENERIC;
    }

    // set symbol as first atom in new atoms array
    atom_setsym(atoms, s);

    for (int i = 0; i < argc; i++) {
        switch ((argv + i)->a_type) {
        case A_FLOAT: {
            atom_setfloat((atoms + (i + 1)), atom_getfloat(argv + i));
            break;
        }
        case A_LONG: {
            atom_setlong((atoms + (i + 1)), atom_getlong(argv + i));
            break;
        }
        case A_SYM: {
            atom_setsym((atoms + (i + 1)), atom_getsym(argv + i));
            break;
        }
        default:
            object_error((t_object*)x, "cannot process unknown type");
            break;
        }
    }

    t_max_err err = atom_gettext(argc+1, atoms, &textsize, &text,
                                 OBEX_UTIL_ATOM_GETTEXT_DEFAULT);
    if (err != MAX_ERR_NONE && !textsize && !text) {
        goto error;
    }

    // post("text: %s", text);

    char* code = str_replace(text, "\\", "");

    xpc_rich_error_t error;
    xpc_session_t session = xpc_session_create_xpc_service("xpyc.PythonService", NULL, 0, &error);
    if (session == NULL) {
        goto cleanup;
    }

    xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);

    // post("code: %s", code);

    xpc_dictionary_set_string(message, "code", code);

    if (gensym(code) == gensym("XPYC_EXIT")) {
        goto cleanup;
    }

    xpc_object_t reply = xpc_session_send_message_with_reply_sync(session, message, &error);
    if (reply == NULL) {
        goto cleanup;
    }

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
    sysmem_freeptr(text);
    sysmem_freeptr(code);

    if (session) {
        xpc_session_cancel(session);
        xpc_release(session);
    }

    return MAX_ERR_NONE;

error:

    return MAX_ERR_GENERIC;
}
