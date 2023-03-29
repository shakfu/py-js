/**
 * @defgroup   webserv external
 *
 * @brief      This external borrows the max-sdk simplethread example external
 * and embeds an http webserver using the mongoose webserver library. ref:
 * https://github.com/cesanta/mongoose
 *
 * @author     sa
 * @date       2023
 */

#include "ext.h"
#include "ext_obex.h"
#include "ext_systhread.h"

#include <libgen.h>

#include "mongoose.h"

#define PY_MAX_ELEMS 1024

// const
static const   int s_debug_level = 2;
static const char* s_listening_address = "http://localhost:8000";
static const char* s_enable_hexdump = "no";
static const char* s_ssi_pattern = "#.shtml";

// mutable constants
static const char* s_root_dir = NULL;


typedef struct _webserv {
    t_object x_ob;             // standard max object
    t_systhread x_systhread;   // thread reference
    t_systhread_mutex x_mutex; // mutual exclusion lock for threadsafety
    int x_systhread_cancel;    // thread cancel flag
    void* x_qelem;             // for message passing between threads
    // int x_port;                // server listening port
    void* x_outlet;            // our outlet
    int x_foo;                 // simple data to pass between threads
    int x_sleeptime;           // how many milliseconds to sleep
    int x_is_running;          // status of webserver
    t_string* x_root_dir;      // root path to statically serve from
} t_webserv;

void webserv_bang(t_webserv* x);
void webserv_start(t_webserv* x);
void webserv_foo(t_webserv* x, long foo);
void webserv_sleeptime(t_webserv* x, long sleeptime);
void webserv_stop(t_webserv* x);
void webserv_cancel(t_webserv* x);
void* webserv_threadproc(t_webserv* x);
void webserv_qfn(t_webserv* x);
void webserv_assist(t_webserv* x, void* b, long m, long a, char* s);
void webserv_free(t_webserv* x);
void* webserv_new(void);


void do_build_objects(t_webserv* x, t_symbol *s, short argc, t_atom *argv) {

    t_object *patcher;

    if (object_obex_lookup(x, gensym("#P"), &patcher) == MAX_ERR_NONE) {
        if (1) {
            t_object *toggle, *metro;
            newobject_sprintf(patcher, 
                "@maxclass toggle @patching_position %.2f %.2f", 50.0, 300.0); 
            newobject_sprintf(patcher, 
                "@maxclass newobj @text \"metro 400\" @patching_position %.2f %.2f", 200.0, 400.0);        
        } else {
            newobject_fromboxtext(patcher, "cycle~ 440");
        } 
    }
}




// webserver main function
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    switch(ev) {
    
    case MG_EV_ERROR: {
        char *error_message = strdup((char*)ev_data);
        error("MG_EV_ERROR: %s", error_message);
    }   break;

    case MG_EV_OPEN:
        post("MG_EV_OPEN");
        break;

    case MG_EV_POLL: {
        // uint64_t *milliseconds = (uint64_t*)ev_data;
        // post("MG_EV_POLL: %d", milliseconds);
    }   break;

    case MG_EV_RESOLVE:
        post("MG_EV_RESOLVE");
        break;

    case MG_EV_CONNECT:
        post("MG_EV_CONNECT");
        break;

    case MG_EV_ACCEPT:
        post("MG_EV_ACCEPT");
        break;

    case MG_EV_TLS_HS:
        post("MG_EV_TLS_HS");
        break;

    case MG_EV_READ:
        post("MG_EV_READ");
        break;

    case MG_EV_WRITE:
        post("MG_EV_WRITE");
        break;

    case MG_EV_CLOSE:
        post("MG_EV_CLOSE");
        break;

    case MG_EV_HTTP_MSG: {
        post("MG_EV_HTTP_MSG");
        struct mg_http_message *hm = (struct mg_http_message*)ev_data;
        struct mg_http_message tmp = {0};
        struct mg_str unknown = mg_str_n("?", 1);
        struct mg_str *cl;
        // On /api/hello requests, send dynamic JSON response
        if (mg_http_match_uri(hm, "/api/hello")) {
            // mg_http_reply(c, 200, "", "{%Q:%d}\n", "status", 1);
            object_post((t_object*)c->fn_data, "in http msg");
            defer((t_object*)c->fn_data, (method)do_build_objects, gensym(""), 0, NULL);
            mg_http_reply(c, 200, "", "{%Q:%d}\n", "status", 1);            

        } else if (mg_http_match_uri(hm, "/api/f2/*")) {
            mg_http_reply(c, 200, "", "{\"result\": \"%.*s\"}\n", (int) hm->uri.len,
                    hm->uri.ptr);
        } else {
            // static file server configuration
            struct mg_http_serve_opts opts = {
                .root_dir = s_root_dir,
                .ssi_pattern = s_ssi_pattern
            };
            mg_http_serve_dir(c, hm, &opts);
        }
        mg_http_parse((char *) c->send.buf, c->send.len, &tmp);
        cl = mg_http_get_header(&tmp, "Content-Length");
        if (cl == NULL) cl = &unknown;
        post("%.*s %.*s %.*s %.*s", (int) hm->method.len, hm->method.ptr,
                 (int) hm->uri.len, hm->uri.ptr, (int) tmp.uri.len, tmp.uri.ptr,
                 (int) cl->len, cl->ptr);
    }   break;

    case MG_EV_HTTP_CHUNK:
        post("MG_EV_HTTP_CHUNK");
        break;

    case MG_EV_WS_OPEN:
        post("MG_EV_WS_OPEN");
        break;

    case MG_EV_WS_MSG:
        post("MG_EV_WS_MSG");
        break;

    case MG_EV_WS_CTL:
        post("MG_EV_WS_MSG");
        break;

    case MG_EV_MQTT_CMD:
        post("MG_EV_MQTT_CMD");
        break;

    case MG_EV_MQTT_MSG:
        post("MG_EV_MQTT_MSG");
        break;

    case MG_EV_MQTT_OPEN:
        post("MG_EV_MQTT_OPEN");
        break;

    case MG_EV_SNTP_TIME:
        post("MG_EV_SNTP_TIME");
        break;

    case MG_EV_USER:
        post("MG_EV_USER");
        break;

    default:
        break;

    }
    (void) fn_data;
}


// // webserver main function
// static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
// {
//     if (ev == MG_EV_OPEN) {
//         // c->is_hexdumping = 1;
//     }
//     else if (ev == MG_EV_HTTP_MSG) {
//         struct mg_http_message *hm = (struct mg_http_message*)ev_data;
//         struct mg_http_message tmp = {0};
//         struct mg_str unknown = mg_str_n("?", 1);
//         struct mg_str *cl;
//         // On /api/hello requests, send dynamic JSON response
//         if (mg_http_match_uri(hm, "/api/hello")) {
//             mg_http_reply(c, 200, "", "{%Q:%d}\n", "status", 1);
//         } else {
//             // static file server configuration
//             struct mg_http_serve_opts opts = {
//                 .root_dir = s_root_dir,
//                 .ssi_pattern = s_ssi_pattern
//             };
//             mg_http_serve_dir(c, hm, &opts);
//         }
//         mg_http_parse((char *) c->send.buf, c->send.len, &tmp);
//         cl = mg_http_get_header(&tmp, "Content-Length");
//         if (cl == NULL) cl = &unknown;
//         post("%.*s %.*s %.*s %.*s", (int) hm->method.len, hm->method.ptr,
//                  (int) hm->uri.len, hm->uri.ptr, (int) tmp.uri.len, tmp.uri.ptr,
//                  (int) cl->len, cl->ptr);
//   }
//   (void) fn_data;
// }



t_string* get_path_to_webroot(t_class* klass)
{
    char external_path[MAX_PATH_CHARS];
    char external_name[MAX_PATH_CHARS];
    char conform_path[MAX_PATH_CHARS];

    short path_id = class_getpath(klass);
    snprintf_zero(external_name, PY_MAX_ELEMS, "%s.mxo", klass->c_sym->s_name);
    path_toabsolutesystempath(path_id, external_name, external_path);
    path_nameconform(external_path, conform_path, PATH_STYLE_NATIVE,
                     PATH_TYPE_TILDE);
    t_string* webroot_path = string_new(dirname(dirname(external_path)));
    string_append(webroot_path, "/source/projects/webserv/webroot");
    return webroot_path;
}


t_class* webserv_class;

void ext_main(void* r)
{
    t_class* c;

    c = class_new("webserv", (method)webserv_new, (method)webserv_free,
                  sizeof(t_webserv), 0L, 0);

    class_addmethod(c, (method)webserv_bang, "bang", 0);
    class_addmethod(c, (method)webserv_start, "start", 0);
    class_addmethod(c, (method)webserv_foo, "foo", A_DEFLONG, 0);
    class_addmethod(c, (method)webserv_sleeptime, "sleeptime", A_DEFLONG, 0);
    class_addmethod(c, (method)webserv_cancel, "cancel", 0);
    class_addmethod(c, (method)webserv_assist, "assist", A_CANT, 0);

    class_register(CLASS_BOX, c);
    webserv_class = c;
}



void webserv_start(t_webserv* x)
{
    if (x->x_is_running)
        webserv_stop(x); // kill thread if, any

    // create new thread + begin execution
    if (x->x_systhread == NULL && !x->x_is_running) {
        // post("starting a webserver on port: %d", x->x_port);

        post("Mongoose version : v%s", MG_VERSION);
        post("Listening on     : %s", s_listening_address);
        post("Web root         : [%s]", s_root_dir);
        systhread_create((method)webserv_threadproc, x, 0, 0, 0,
                         &x->x_systhread);
        x->x_is_running = true;
    }
}

void webserv_foo(t_webserv* x, long foo)
{
    systhread_mutex_lock(x->x_mutex);
    x->x_foo = foo; // override our current value
    systhread_mutex_unlock(x->x_mutex);
}

void webserv_sleeptime(t_webserv* x, long sleeptime)
{
    if (sleeptime < 10)
        sleeptime = 10;
    x->x_sleeptime = sleeptime; // no need to lock since we are readonly in
                                // worker thread
}


void webserv_stop(t_webserv* x)
{
    unsigned int ret;

    if (x->x_systhread) {
        post("stopping webserver thread");
        x->x_systhread_cancel = true;         // tell the thread to stop
        systhread_join(x->x_systhread, &ret); // wait for the thread to stop
        x->x_systhread = NULL;
        x->x_is_running = false;
    }
}


void webserv_cancel(t_webserv* x)
{
    webserv_stop(x); // kill thread if, any
    post("Exiting on message cancel");
    outlet_anything(x->x_outlet, gensym("cancelled"), 0, NULL);
}

void* webserv_threadproc(t_webserv* x)
{
    // char listening_address[100];
    // int max_len = sizeof listening_address;
    // snprintf_zero(listening_address, max_len, s_listening_address, x->x_port);

    // loop until told to stop
    while (1) {

        struct mg_mgr mgr;
        mg_log_set(s_debug_level);
        mg_mgr_init(&mgr); // Init manager
        // mg_http_listen(&mgr, s_listening_address, fn, &mgr); // Setup listener
        mg_http_listen(&mgr, s_listening_address, fn, x); // Setup listener

        for (;;) {
            mg_mgr_poll(&mgr, 1000); // Event loop
            if (x->x_systhread_cancel)
                break;
        }
        mg_mgr_free(&mgr); // Cleanup
        break;

        systhread_mutex_lock(x->x_mutex);
        x->x_foo++; // fiddle with shared data
        systhread_mutex_unlock(x->x_mutex);

        qelem_set(x->x_qelem); // notify main thread using qelem mechanism

        systhread_sleep(x->x_sleeptime); // sleep a bit
    }

    // reset cancel flag for next time, in case the thread is created again
    x->x_systhread_cancel = false;

    systhread_exit(0); // this can return a value to systhread_join();
    return NULL;
}

// triggered by the helper thread
void webserv_qfn(t_webserv* x)
{
    int myfoo;

    systhread_mutex_lock(x->x_mutex);
    myfoo = x->x_foo; // access shared data
    systhread_mutex_unlock(x->x_mutex);

    // *never* wrap outlet calls with systhread_mutex_lock()
    outlet_int(x->x_outlet, myfoo);
}

void webserv_assist(t_webserv* x, void* b, long m, long a, char* s)
{
    if (m == 1)
        sprintf(s, "start starts a new thread");
    else if (m == 2)
        sprintf(s, "report when done/cancelled");
}

void webserv_free(t_webserv* x)
{
    // stop our thread if it is still running
    webserv_stop(x);

    // free our qelem
    if (x->x_qelem)
        qelem_free(x->x_qelem);

    // free out mutex
    if (x->x_mutex)
        systhread_mutex_free(x->x_mutex);
}

void* webserv_new(void)
{
    t_webserv* x;

    x = (t_webserv*)object_alloc(webserv_class);
    x->x_outlet = outlet_new(x, NULL);
    x->x_qelem = qelem_new(x, (method)webserv_qfn);
    x->x_systhread = NULL;
    systhread_mutex_new(&x->x_mutex, 0);
    x->x_foo = 0;
    x->x_sleeptime = 1000;
    // x->x_port = 8000;
    x->x_is_running = false;
    x->x_root_dir = get_path_to_webroot(webserv_class);

    // set global
    s_root_dir = string_getptr(x->x_root_dir);

    return (x);
}

void webserv_bang(t_webserv* x) {
    outlet_bang(x->x_outlet); 
}