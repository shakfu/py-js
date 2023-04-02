/**
 * @defgroup   wbsrv external
 *
 * @brief      This external borrows from the implementation max-sdk simplethread
 *             example external and embeds an http webserver using the mongoose
 *             webserver library. ref: https://github.com/cesanta/mongoose
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
static const char* s_subpath = "/source/projects/wbsrv/webroot";

// mutable constants
static const char* s_root_dir = NULL;


typedef struct _wbsrv {
    t_object x_ob;             // standard max object
    t_systhread x_systhread;   // thread reference
    t_systhread_mutex x_mutex; // mutual exclusion lock for threadsafety
    int x_systhread_cancel;    // thread cancel flag
    void* x_qelem;             // for message passing between threads
    // int x_port;                // server listening port
    void* x_outlet;            // our outlet
    int x_foo;                 // simple data to pass between threads
    int x_sleeptime;           // how many milliseconds to sleep
    int x_is_running;          // status of wbsrver
    t_string* x_root_dir;      // root path to statically serve from
} t_wbsrv;

void wbsrv_bang(t_wbsrv* x);
void wbsrv_start(t_wbsrv* x);
void wbsrv_foo(t_wbsrv* x, long foo);
void wbsrv_sleeptime(t_wbsrv* x, long sleeptime);
void wbsrv_stop(t_wbsrv* x);
void wbsrv_cancel(t_wbsrv* x);
void* wbsrv_threadproc(t_wbsrv* x);
void wbsrv_qfn(t_wbsrv* x);
void wbsrv_assist(t_wbsrv* x, void* b, long m, long a, char* s);
void wbsrv_free(t_wbsrv* x);
void* wbsrv_new(void);


void do_build_objects(t_wbsrv* x, t_symbol *s, short argc, t_atom *argv) {

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

// http message handler
void handle_event_http_message(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    post("MG_EV_HTTP_MSG");
    struct mg_http_message *hm = (struct mg_http_message*)ev_data;
    struct mg_http_message tmp = {0};
    struct mg_str unknown = mg_str_n("?", 1);
    struct mg_str *cl;
    // On /api/hello requests, send dynamic JSON response
    if (mg_http_match_uri(hm, "/api/hello")) {
        // mg_http_reply(c, 200, "", "{%Q:%d}\n", "status", 1);
        object_post((t_object*)c->fn_data, "in /api/hello msg");
        mg_http_reply(c, 200, "", "{%Q:%d}\n", "status", 1);

    } else if (mg_http_match_uri(hm, "/api/create")) {
        object_post((t_object*)c->fn_data, "in /api/create msg");
        defer((t_object*)c->fn_data, (method)do_build_objects, gensym(""),
              0, NULL);
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
}


// wbsrv main function
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    switch(ev) {
    
    case MG_EV_ERROR: { // Error -> char *error_message
        char *error_message = strdup((char*)ev_data);
        error("MG_EV_ERROR: %s", error_message);
    }   break;

    case MG_EV_OPEN:    // Connection created -> NULL
        post("MG_EV_OPEN");
        // c->is_hexdumping = 1;
        break;

    case MG_EV_POLL: {  // mg_mgr_poll iteration -> uint64_t *milliseconds
        // uint64_t *milliseconds = (uint64_t*)ev_data;
        // post("MG_EV_POLL: %d", milliseconds);
    }   break;

    case MG_EV_RESOLVE: // host name is resolved
        post("MG_EV_RESOLVE");
        break;

    case MG_EV_CONNECT: // connection established
        post("MG_EV_CONNECT");
        break;

    case MG_EV_ACCEPT:  // connection accepted
        post("MG_EV_ACCEPT");
        break;

    case MG_EV_TLS_HS:  // TLS handshake succeeded
        post("MG_EV_TLS_HS");
        break;

    case MG_EV_READ:    // data received from socket -> long *bytes_read
        post("MG_EV_READ");
        break;

    case MG_EV_WRITE:   // data written to socket -> long *bytes_written
        post("MG_EV_WRITE");
        break;

    case MG_EV_CLOSE:   // connection closed
        post("MG_EV_CLOSE");
        break;

    case MG_EV_HTTP_MSG: // http request/respose -> struct mg_http_message *
        handle_event_http_message(c, ev, ev_data, fn_data);
        break;

    case MG_EV_HTTP_CHUNK: // HTTP chunk (partial msg) -> struct mg_http_message *
        post("MG_EV_HTTP_CHUNK");
        break;

    case MG_EV_WS_OPEN: // Websocket handshake done -> struct mg_http_message *
        post("MG_EV_WS_OPEN");
        break;

    case MG_EV_WS_MSG: // Websocket msg, text or bin ->sstruct mg_ws_message *
        post("MG_EV_WS_MSG");
        break;

    case MG_EV_WS_CTL: // Websocket control msg -> struct mg_ws_message *
        post("MG_EV_WS_MSG");
        break;

    case MG_EV_MQTT_CMD: // /MQTT low-level command -> struct mg_mqtt_message *
        post("MG_EV_MQTT_CMD");
        break;

    case MG_EV_MQTT_MSG: // MQTT PUBLISH received -> struct mg_mqtt_message *
        post("MG_EV_MQTT_MSG");
        break;

    case MG_EV_MQTT_OPEN: // MQTT CONNACK received -> int *connack_status_code
        post("MG_EV_MQTT_OPEN");
        break;

    case MG_EV_SNTP_TIME: // SNTP time received -> uint64_t *milliseconds
        post("MG_EV_SNTP_TIME");
        break;

    case MG_EV_USER: // Starting ID for user events
        post("MG_EV_USER");
        break;

    default:
        break;

    }
    (void) fn_data;
}


// // wbsrver main function
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
    string_append(webroot_path, s_subpath);
    return webroot_path;
}


t_class* wbsrv_class;

void ext_main(void* r)
{
    t_class* c;

    c = class_new("wbsrv", (method)wbsrv_new, (method)wbsrv_free,
                  sizeof(t_wbsrv), 0L, 0);

    class_addmethod(c, (method)wbsrv_bang, "bang", 0);
    class_addmethod(c, (method)wbsrv_start, "start", 0);
    class_addmethod(c, (method)wbsrv_foo, "foo", A_DEFLONG, 0);
    class_addmethod(c, (method)wbsrv_sleeptime, "sleeptime", A_DEFLONG, 0);
    class_addmethod(c, (method)wbsrv_cancel, "cancel", 0);
    class_addmethod(c, (method)wbsrv_assist, "assist", A_CANT, 0);

    class_register(CLASS_BOX, c);
    wbsrv_class = c;
}



void wbsrv_start(t_wbsrv* x)
{
    if (x->x_is_running)
        wbsrv_stop(x); // kill thread if, any

    // create new thread + begin execution
    if (x->x_systhread == NULL && !x->x_is_running) {
        // post("starting a wbsrver on port: %d", x->x_port);

        post("Mongoose version : v%s", MG_VERSION);
        post("Listening on     : %s", s_listening_address);
        post("Web root         : [%s]", s_root_dir);
        systhread_create((method)wbsrv_threadproc, x, 0, 0, 0,
                         &x->x_systhread);
        x->x_is_running = true;
    }
}

void wbsrv_foo(t_wbsrv* x, long foo)
{
    systhread_mutex_lock(x->x_mutex);
    x->x_foo = foo; // override our current value
    systhread_mutex_unlock(x->x_mutex);
}

void wbsrv_sleeptime(t_wbsrv* x, long sleeptime)
{
    if (sleeptime < 10)
        sleeptime = 10;
    x->x_sleeptime = sleeptime; // no need to lock since we are readonly in
                                // worker thread
}


void wbsrv_stop(t_wbsrv* x)
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


void wbsrv_cancel(t_wbsrv* x)
{
    wbsrv_stop(x); // kill thread if, any
    post("Exiting on message cancel");
    outlet_anything(x->x_outlet, gensym("cancelled"), 0, NULL);
}

void* wbsrv_threadproc(t_wbsrv* x)
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
void wbsrv_qfn(t_wbsrv* x)
{
    int myfoo;

    systhread_mutex_lock(x->x_mutex);
    myfoo = x->x_foo; // access shared data
    systhread_mutex_unlock(x->x_mutex);

    // *never* wrap outlet calls with systhread_mutex_lock()
    outlet_int(x->x_outlet, myfoo);
}

void wbsrv_assist(t_wbsrv* x, void* b, long m, long a, char* s)
{
    if (m == 1)
        sprintf(s, "start starts a new thread");
    else if (m == 2)
        sprintf(s, "report when done/cancelled");
}

void wbsrv_free(t_wbsrv* x)
{
    // stop our thread if it is still running
    wbsrv_stop(x);

    // free our qelem
    if (x->x_qelem)
        qelem_free(x->x_qelem);

    // free out mutex
    if (x->x_mutex)
        systhread_mutex_free(x->x_mutex);
}

void* wbsrv_new(void)
{
    t_wbsrv* x;

    x = (t_wbsrv*)object_alloc(wbsrv_class);
    x->x_outlet = outlet_new(x, NULL);
    x->x_qelem = qelem_new(x, (method)wbsrv_qfn);
    x->x_systhread = NULL;
    systhread_mutex_new(&x->x_mutex, 0);
    x->x_foo = 0;
    x->x_sleeptime = 1000;
    // x->x_port = 8000;
    x->x_is_running = false;
    x->x_root_dir = get_path_to_webroot(wbsrv_class);

    // set global
    s_root_dir = string_getptr(x->x_root_dir);

    return (x);
}

void wbsrv_bang(t_wbsrv* x) {
    outlet_bang(x->x_outlet); 
}