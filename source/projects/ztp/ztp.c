#include "ext.h"
#include "ext_obex.h"
#include "ext_systhread.h"

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <spawn.h>
#include <sys/wait.h>

#include <zmq.h>

#define RESPONSE_BUFFER_SIZE 64

typedef struct _ztp {
    t_object            x_ob;                   // standard max object
    t_systhread         x_systhread;            // thread reference
    t_systhread_mutex   x_mutex;                // mutual exclusion lock for threadsafety
    int                 x_systhread_cancel;     // thread cancel flag
    void                *x_qelem;               // for message passing between threads
    void                *x_outlet;              // our outlet
    int                 x_sleeptime;            // how many milliseconds to sleep
    t_symbol*           x_request;              // code to be evaluated remotely
    t_symbol*           x_response;             // result of remote evaluation
    int                 x_is_new;               // 1 means there's a new request
    t_symbol*           x_python;               // full path to the python3 executable
    t_symbol*           x_server;               // path to the python3 server file
} t_ztp;

void ztp_bang(t_ztp *x);
void ztp_sleeptime(t_ztp *x, long sleeptime);
void ztp_stop(t_ztp *x);
void ztp_cancel(t_ztp *x);
void *ztp_threadproc(t_ztp *x);
void ztp_qfn(t_ztp *x);
void ztp_assist(t_ztp *x, void *b, long m, long a, char *s);
void ztp_free(t_ztp *x);
void *ztp_new(t_symbol* s, long argc, t_atom* argv);
t_max_err ztp_py(t_ztp* x, t_symbol* s);
t_symbol* ztp_locate_path_from_symbol(t_ztp* x, t_symbol* s);
void ztp_run_server(t_ztp* x);
void ztp_server_do(t_ztp *x, t_symbol *s, short argc, t_atom *argv);
void ztp_serve(t_ztp *x);
t_max_err ztp_server_attr_get(t_ztp* x, t_object* attr, long* argc,t_atom** argv);
t_max_err ztp_server_attr_set(t_ztp* x, t_object* attr, long argc,t_atom* argv);



t_class *ztp_class;

extern char **environ;


void ext_main(void *r)
{
    t_class *c;

    c = class_new("ztp", (method)ztp_new, (method)ztp_free, sizeof(t_ztp),
        0L, A_GIMME, 0);

    class_addmethod(c, (method)ztp_bang,        "bang",         0);
    class_addmethod(c, (method)ztp_py,          "py",           A_DEFSYM, 0);
    class_addmethod(c, (method)ztp_sleeptime,   "sleeptime",    A_DEFLONG, 0);
    class_addmethod(c, (method)ztp_cancel,      "cancel",       0);
    class_addmethod(c, (method)ztp_serve,       "serve",        0);
    class_addmethod(c, (method)ztp_assist,      "assist",       A_CANT, 0);

    CLASS_ATTR_SYM(c,   "python", 0,  t_ztp, x_python);
    CLASS_ATTR_BASIC(c, "python", 0);
    CLASS_ATTR_SYM(c,   "server", 0,  t_ztp, x_server);
    CLASS_ATTR_BASIC(c, "server", 0);
    CLASS_ATTR_ACCESSORS(c, "server", ztp_server_attr_get, ztp_server_attr_set);

    class_register(CLASS_BOX,c);
    ztp_class = c;
}

void ztp_bang(t_ztp *x)
{
    ztp_stop(x);        // kill thread if, any

    // create new thread + begin execution
    if (x->x_systhread == NULL) {
        post("starting a new thread");
        systhread_create((method) ztp_threadproc, x, 0, 0, 0, &x->x_systhread);
    }
}

t_max_err ztp_py(t_ztp* x, t_symbol* s)
{
    systhread_mutex_lock(x->x_mutex);
    x->x_request = s;    // override our current value
    x->x_is_new = 1;
    systhread_mutex_unlock(x->x_mutex);
}

void ztp_sleeptime(t_ztp *x, long sleeptime)
{
    if (sleeptime<10)
        sleeptime = 10;
    x->x_sleeptime = (int)sleeptime;    // no need to lock since we are readonly in worker thread
}


void ztp_stop(t_ztp *x)
{
    unsigned int ret;

    if (x->x_systhread) {
        post("stopping our thread");
        x->x_systhread_cancel = true;               // tell the thread to stop
        systhread_join(x->x_systhread, &ret);       // wait for the thread to stop
        x->x_systhread = NULL;
    }
}


void ztp_cancel(t_ztp *x)
{
    ztp_py(x, gensym("ZTP_EXIT"));

    ztp_stop(x);                                    // kill thread if, any
    outlet_anything(x->x_outlet, gensym("cancelled"), 0, NULL);
}


void ztp_serve(t_ztp *x)
{
    defer_low((t_object *)x, (method)ztp_server_do,  NULL, 0, NULL);

}


void* ztp_threadproc(t_ztp* x)
{
    post("Connecting to server…\n");
    void* context = zmq_ctx_new();
    void* requester = zmq_socket(context, ZMQ_REQ);
    zmq_connect(requester, "tcp://localhost:5555");

    char buffer[RESPONSE_BUFFER_SIZE];
    t_symbol* req;
    int is_new = 0;
    while (1) {
        if (x->x_systhread_cancel) {
            post("cancelling thread process");
            break;
        }
        systhread_mutex_lock(x->x_mutex);
        req = x->x_request;
        is_new = x->x_is_new;
        systhread_mutex_unlock(x->x_mutex);
        if (is_new) {
            post("sent: %s", req->s_name);
            zmq_send(requester, req->s_name, strlen(req->s_name), 0);
            zmq_recv(requester, buffer, RESPONSE_BUFFER_SIZE, 0);
            post("received: %s", buffer);
            systhread_mutex_lock(x->x_mutex);
            x->x_response = gensym(buffer);
            x->x_is_new = 0;
            systhread_mutex_unlock(x->x_mutex);
            qelem_set(x->x_qelem);
        }
    }
    zmq_close(requester);
    zmq_ctx_destroy(context);

    systhread_sleep(x->x_sleeptime); // sleep a bit

    x->x_systhread_cancel = false; // reset cancel flag for next time, in case
    // the thread is created again

    systhread_exit(0); // this can return a value to systhread_join();
    return NULL;
}


// triggered by the helper thread
void ztp_qfn(t_ztp *x)
{
    t_symbol* response;

    systhread_mutex_lock(x->x_mutex);
    response = x->x_response;                                                           // access shared data
    systhread_mutex_unlock(x->x_mutex);

    // *never* wrap outlet calls with systhread_mutex_lock()
    outlet_anything(x->x_outlet, response, 0, NULL);
}

void ztp_assist(t_ztp *x, void *b, long m, long a, char *s)
{
    if (m==1)
        sprintf(s,"bang starts a new thread");
    else if (m==2)
        sprintf(s,"report when done/cancelled");
}

void ztp_free(t_ztp *x)
{
    // stop our thread if it is still running
    ztp_stop(x);

    // free our qelem
    if (x->x_qelem)
        qelem_free(x->x_qelem);

    // free out mutex
    if (x->x_mutex)
        systhread_mutex_free(x->x_mutex);
}

void *ztp_new(t_symbol* s, long argc, t_atom* argv)
{
    t_ztp *x;

    x = (t_ztp *)object_alloc(ztp_class);
    x->x_outlet = outlet_new(x, NULL);
    x->x_qelem = qelem_new(x,(method)ztp_qfn);
    x->x_systhread = NULL;
    systhread_mutex_new(&x->x_mutex,0);
    x->x_request = gensym("");
    x->x_response = gensym("");
    x->x_sleeptime = 1000;
    x->x_is_new = 0;
    x->x_python = gensym("");
    x->x_server = gensym("");

    attr_args_process(x, argc, argv);

    post("x_python: %s", x->x_python->s_name);
    post("x_server: %s", x->x_server->s_name);

    return(x);
}

void ztp_server_do(t_ztp *x, t_symbol *s, short argc, t_atom *argv)
{
    ztp_run_server(x);
}


void ztp_run_server(t_ztp *x)
{
    pid_t pid;
    char *argv[] = {x->x_python->s_name, x->x_server->s_name, NULL};
    if(posix_spawn(&pid, argv[0], NULL, NULL, argv, environ) != 0) {
        error("run_server failed");
        return;
    } else {
        post("process %d started", pid);
    }
    waitpid(pid, NULL, WNOHANG);
}


t_symbol* ztp_locate_path_from_symbol(t_ztp* x, t_symbol* s)
{
    char filename[MAX_PATH_CHARS];
    char pathname[MAX_PATH_CHARS];
    short path;
    t_fourcc type = FOUR_CHAR_CODE('TEXT');
    t_max_err err;

    strncpy_zero(filename, s->s_name, MAX_PATH_CHARS);
    if (locatefile_extended(filename, &path, &type, &type, 1)) {
        // nozero: not found
        error("can't find file %s", s->s_name);
        return gensym("");
    }

    pathname[0] = 0;
    err = path_toabsolutesystempath(path, filename, pathname);
    if (err != MAX_ERR_NONE) {
        error("can't convert %s to absolutepath", s->s_name);
        return gensym("");
    }
    // post("full path is: %s", pathname);
    return gensym(pathname);
}


t_max_err ztp_server_attr_get(t_ztp* x, t_object* attr, long* argc,t_atom** argv)
{
    char alloc;

    if (argc && argv) {
        if (atom_alloc(argc, argv, &alloc)) {
            return MAX_ERR_OUT_OF_MEM;
        }
        if (alloc) {
            atom_setsym(*argv, x->x_server);
        }
    }
    return MAX_ERR_NONE;
}


t_max_err ztp_server_attr_set(t_ztp* x, t_object* attr, long argc,t_atom* argv)
{
    if (argc && argv) {

        t_symbol* entry = atom_getsym(argv);
        t_symbol* fullpath = ztp_locate_path_from_symbol(x, entry);

        if (fullpath != gensym("")) {
            x->x_server = fullpath;
        } else {
            x->x_server = entry;
        }
    }
    return MAX_ERR_NONE;
}
