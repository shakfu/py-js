#include <string.h>
#include <unistd.h>

#include <uuid/uuid.h>
#include <time.h> /* for struct tm, localtime_r */
#include <sys/time.h> /* for gettimeofday */

#include "ext.h"
#include "ext_obex.h"
#include "ext_systhread.h"
#include "ext_dictionary.h"
#include "commonsyms.h"

#include <zmq.h>

// derived from max-sdk/sources/advanced/simplethread

typedef struct _jmx {
    t_object            x_ob;                   // standard max object
    t_systhread         x_systhread;            // thread reference
    t_systhread_mutex   x_mutex;                // mutual exclusion lock for threadsafety
    int                 x_systhread_cancel;     // thread cancel flag
    void                *x_qelem;               // for message passing between threads
    void                *x_outlet;              // our outlet
    int                 x_foo;                  // simple data to pass between threads
    int                 x_sleeptime;            // how many milliseconds to sleep
} t_jmx;


void jmx_bang(t_jmx *x);
void jmx_foo(t_jmx *x, long foo);
void jmx_sleeptime(t_jmx *x, long sleeptime);
void jmx_stop(t_jmx *x);
void jmx_cancel(t_jmx *x);
void *jmx_threadproc(t_jmx *x);
void jmx_qfn(t_jmx *x);
void jmx_assist(t_jmx *x, void *b, long m, long a, char *s);
void jmx_free(t_jmx *x);
void *jmx_new(void);

void jmx_msg(t_jmx* x);
t_symbol* get_uuid(void);
t_symbol* get_timestamp(void);
void jmx_msg_create(t_jmx* x, const char* username, const char* msg_type, const char* code);

t_class *jmx_class;

void ext_main(void *r)
{
    t_class *c;

    c = class_new("jmx", (method)jmx_new, (method)jmx_free, sizeof(t_jmx), 0L, 0);

    class_addmethod(c, (method)jmx_bang,        "bang",         0);
    class_addmethod(c, (method)jmx_msg,         "msg",          0);
    class_addmethod(c, (method)jmx_foo,         "foo",          A_DEFLONG, 0);
    class_addmethod(c, (method)jmx_sleeptime,   "sleeptime",    A_DEFLONG, 0);
    class_addmethod(c, (method)jmx_cancel,      "cancel",       0);
    class_addmethod(c, (method)jmx_assist,      "assist",       A_CANT, 0);

    class_register(CLASS_BOX,c);
    jmx_class = c;

    common_symbols_init();
}

void jmx_bang(t_jmx *x)
{
    jmx_stop(x);        // kill thread if, any

    // create new thread + begin execution
    if (x->x_systhread == NULL) {
        post("starting a new thread");
        systhread_create((method) jmx_threadproc, x, 0, 0, 0, &x->x_systhread);
    }
}

void jmx_foo(t_jmx *x, long foo)
{
    systhread_mutex_lock(x->x_mutex);
    x->x_foo = (int)foo;    // override our current value
    systhread_mutex_unlock(x->x_mutex);
}

void jmx_sleeptime(t_jmx *x, long sleeptime)
{
    if (sleeptime<10)
        sleeptime = 10;
    x->x_sleeptime = (int)sleeptime;    // no need to lock since we are readonly in worker thread
}


void jmx_stop(t_jmx *x)
{
    unsigned int ret;

    if (x->x_systhread) {
        post("stopping our thread");
        x->x_systhread_cancel = true;               // tell the thread to stop
        systhread_join(x->x_systhread, &ret);       // wait for the thread to stop
        x->x_systhread = NULL;
    }
}


void jmx_cancel(t_jmx *x)
{
    jmx_stop(x);                                    // kill thread if, any
    outlet_anything(x->x_outlet, gensym("cancelled"), 0, NULL);
}

void* jmx_threadproc(t_jmx* x)
{

    post("Connecting to hello world server…\n");
    void* context = zmq_ctx_new();
    void* requester = zmq_socket(context, ZMQ_REQ);
    zmq_connect(requester, "tcp://localhost:5555");

    int request_nbr = 0;
    post("request_nbr: %i", request_nbr);
    for (request_nbr = 0; request_nbr != 10; request_nbr++) {
        if (x->x_systhread_cancel) {
            post("cancelling thread process");
            break;
        }
        char buffer[10];
        post("Sending Hello %d…\n", request_nbr);
        zmq_send(requester, "Hello", 5, 0);
        zmq_recv(requester, buffer, 10, 0);
        post("Received World %d\n", request_nbr);
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
void jmx_qfn(t_jmx *x)
{
    int myfoo;

    systhread_mutex_lock(x->x_mutex);
    myfoo = x->x_foo;                                                           // access shared data
    systhread_mutex_unlock(x->x_mutex);

    // *never* wrap outlet calls with systhread_mutex_lock()
    outlet_int(x->x_outlet, myfoo);
}

void jmx_assist(t_jmx *x, void *b, long m, long a, char *s)
{
    if (m==1)
        sprintf(s,"bang starts a new thread");
    else if (m==2)
        sprintf(s,"report when done/cancelled");
}

void jmx_free(t_jmx *x)
{
    // stop our thread if it is still running
    jmx_stop(x);

    // free our qelem
    if (x->x_qelem)
        qelem_free(x->x_qelem);

    // free out mutex
    if (x->x_mutex)
        systhread_mutex_free(x->x_mutex);
}

void *jmx_new(void)
{
    t_jmx *x;

    x = (t_jmx *)object_alloc(jmx_class);
    x->x_outlet = outlet_new(x,NULL);
    x->x_qelem = qelem_new(x,(method)jmx_qfn);
    x->x_systhread = NULL;
    systhread_mutex_new(&x->x_mutex,0);
    x->x_foo = 0;
    x->x_sleeptime = 1000;

    return(x);
}


void jmx_msg(t_jmx* x)
{
    jmx_msg_create(x, "bob", "execute_request", "1 + 1");
}


t_symbol* get_uuid(void)
{
    char uuid_str[37];
    uuid_t uuid;
    t_symbol* res = NULL;

    uuid_generate_time(uuid);
    uuid_unparse_lower(uuid, uuid_str);
    // post("generate uuid=%s\n", uuid_str);
    res = gensym(uuid_str);
    uuid_clear(uuid);
    return res;
}

t_symbol* get_timestamp(void)
{
    struct timeval tv;
    struct tm tm;
    char timestamp[] = "YYYY-MM-ddTHH:mm:ss.SSS+0000";

    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S.000%z", &tm);
    sprintf(timestamp + 20, "%03ld%s", (long)tv.tv_usec / 1000, timestamp + 23);
    // post("ISO 8601 timestamp: %s\n", timestamp);
    return gensym(timestamp);
}


void jmx_msg_create(t_jmx* x, const char* username, const char* msg_type, const char* code)
{
    // header dict
    t_dictionary* header = dictionary_new();
    dictionary_appendsym(header, gensym("msg_id"), get_uuid());
    dictionary_appendsym(header, gensym("session"), get_uuid());
    dictionary_appendstring(header, gensym("username"), username);
    dictionary_appendsym(header, gensym("date"), get_timestamp());
    dictionary_appendstring(header, gensym("msg_type"), msg_type);
    // dictionary_appendsym(header, gensym("subshell_id"), gensym("shell_id")); // optional

    // code dict
    t_dictionary* content = dictionary_new();
    t_dictionary* user_expressions = dictionary_new();
    dictionary_appendstring(content, gensym("code"), code);
    dictionary_appendlong(content, gensym("silent"), 1);
    dictionary_appendlong(content, gensym("store_history"), 0);
    dictionary_appenddictionary(content, gensym("user_expressions"), (t_object*)user_expressions);
    dictionary_appendlong(content, gensym("allow_stdin"), 0);
    dictionary_appendlong(content, gensym("stop_on_error"), 1);

    // other empty dicts
    t_dictionary* parent_header = dictionary_new();
    t_dictionary* metadata = dictionary_new();

    // create a msg dictionary
    t_dictionary* d = dictionary_new();
    dictionary_appenddictionary(d, gensym("header"), (t_object *)header);
    dictionary_appenddictionary(d, gensym("parent_header"), (t_object *)parent_header);
    dictionary_appenddictionary(d, gensym("metadata"), (t_object *)metadata);
    dictionary_appenddictionary(d, gensym("content"), (t_object *)content);

    // add empty buffers list
    dictionary_appendatoms(d, gensym("buffers"), 0, NULL);

    // // convert it to json
    t_object* jsonwriter = (t_object*)object_new(_sym_nobox, _sym_jsonwriter);
    t_handle json;
    const char* str;
    object_method(jsonwriter, _sym_writedictionary, d);
    object_method(jsonwriter, _sym_getoutput, &json);
    str = *json;
    // now str contains our JSON serialization of the t_dictionary d

    post("json: %s", str);

    // free d and jsonwriter
    object_free(d);
    object_free(jsonwriter);
}




