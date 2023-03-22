#include "pktpy.h"


typedef struct _pktpy {
    t_object ob;
    t_symbol* name;
    void* outlet;
    PktpyInterpreter* py;
} t_pktpy;


BEGIN_USING_C_LINKAGE

void* pktpy_new(t_symbol* s, long argc, t_atom* argv);
void pktpy_free(t_pktpy* x);
void pktpy_assist(t_pktpy* x, void* b, long m, long a, char* s);

// attr getters / setters
// t_max_err pktpy_name_get(t_pktpy* x, t_object* attr, long* argc, t_atom** argv);
// t_max_err pktpy_name_set(t_pktpy* x, t_object* attr, long argc, t_atom* argv);


// basic methods
void pktpy_bang(t_pktpy*);

// core methods
t_max_err pktpy_eval(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
t_max_err pktpy_exec(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
// t_max_err pktpy_execfile(t_pktpy* x, t_symbol* s);

// extra py methods
// t_max_err pktpy_call(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
// t_max_err pktpy_assign(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
t_max_err pktpy_anything(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
// t_max_err pktpy_pipe(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);

// utilities
t_symbol* pktpy_get_path_to_external(t_pktpy* x);
// int add100(int a);


END_USING_C_LINKAGE


static t_class* pktpy_class = NULL;

void ext_main(void* r)
{
    t_class* c;

    c = class_new("pktpy", 
            (method)pktpy_new,
            (method)pktpy_free,
            (long)sizeof(t_pktpy), 
            (method)NULL, /* leave NULL!! */
            A_GIMME,
            0L);

    class_addmethod(c, (method)pktpy_assist,     "assist",   A_CANT,     0);

    class_addmethod(c, (method)pktpy_bang,       "bang",                 0);

    class_addmethod(c, (method)pktpy_eval,       "eval",     A_GIMME,    0);
    class_addmethod(c, (method)pktpy_exec,       "exec",     A_GIMME,    0);
    // class_addmethod(c, (method)pktpy_execfile,   "execfile", A_DEFSYM,   0);

    // class_addmethod(c, (method)pktpy_assign,     "assign",   A_GIMME,    0);
    // class_addmethod(c, (method)pktpy_call,       "call",     A_GIMME,    0);
    // class_addmethod(c, (method)pktpy_pipe,       "pipe",     A_GIMME,    0);
    class_addmethod(c, (method)pktpy_anything,   "anything", A_GIMME,    0);

    CLASS_ATTR_LABEL(c, "name", 0,  "unique object id");
    CLASS_ATTR_SYM(c, "name", 0,   t_pktpy, name);
    CLASS_ATTR_BASIC(c, "name", 0);
    // CLASS_ATTR_ACCESSORS(c, "name", NULL, pktpy_name_set);


    class_register(CLASS_BOX, c); /* CLASS_NOBOX */
    pktpy_class = c;

    post("I am the pktpy object");
}


t_symbol* pktpy_get_path_to_external(t_pktpy* x)
{
    char external_path[MAX_PATH_CHARS];
    char external_name[MAX_PATH_CHARS];
    char conform_path[MAX_PATH_CHARS];

    short path_id = class_getpath(pktpy_class);
    snprintf_zero(external_name, PY_MAX_ELEMS, "%s.mxo", pktpy_class->c_sym->s_name);
    path_toabsolutesystempath(path_id, external_name, external_path);
    path_nameconform(external_path, conform_path, PATH_STYLE_NATIVE, PATH_TYPE_TILDE);
    // post("path_id: %d, external_name: %s, external_path: %s conform_path: %s", 
    //     path_id, external_name, external_path, conform_path);
    return gensym(external_path);
}




void pktpy_assist(t_pktpy* x, void* b, long m, long a, char* s)
{
    if (m == ASSIST_INLET) { // inlet
        snprintf(s, PY_MAX_ELEMS, "I am inlet %ld", a);
    } else { // outlet
        snprintf(s, PY_MAX_ELEMS, "I am outlet %ld", a);
    }
}



// example function to be wrapped by NativeProxyFunc
int add100(int a){
    return a + 100;
}

void add_custom_builtins(t_pktpy* x)
{
        // builtins
        x->py->p_vm->bind_builtin_func<1>("add10", [](VM* vm, Args& args) {
            i64 a = CAST(i64, args[0]);
            return VAR(a + 10);
        });

        // example of wrapping a max api function
        // bind: void *outlet_int(t_outlet *x, t_atom_long n);
        // >>> out_int(10) -> sends 10 out of outlet
        x->py->p_vm->bind_builtin_func<1>("out_int", [x](VM* vm, Args& args) {
            i64 a = CAST(i64, args[0]);
            outlet_int(x->outlet, a);
            return vm->None;
        });

        // wrap exist function pktpy_get_path_to_external
        x->py->p_vm->bind_builtin_func<0>("location", [x](VM* vm, Args& args) {
            t_symbol* sym = pktpy_get_path_to_external(x);
            outlet_anything(x->outlet, sym, 0, (t_atom*)NIL);
            return vm->None;
        });

        // example of wrapping function using NativeProxyFunc
        // It can be constructed from a function pointer,
        // a lambda function, or a std::function.
        x->py->p_vm->bind_builtin_func<1>("add100", NativeProxyFunc(&add100));
}



void pktpy_free(t_pktpy* x)
{
    // pkpy_delete(x->py);
}

void* pktpy_new(t_symbol* s, long argc, t_atom* argv)
{
    t_pktpy* x = NULL;
    long i;

    if ((x = (t_pktpy*)object_alloc(pktpy_class))) {
        object_post((t_object*)x, "a new %s object was instantiated: %p",
                    s->s_name, x);
        object_post((t_object*)x, "it has %ld arguments", argc);

        for (i = 0; i < argc; i++) {
            if ((argv + i)->a_type == A_LONG) {
                object_post((t_object*)x, "arg %ld: long (%ld)", i,
                            atom_getlong(argv + i));
            } else if ((argv + i)->a_type == A_FLOAT) {
                object_post((t_object*)x, "arg %ld: float (%f)", i,
                            atom_getfloat(argv + i));
            } else if ((argv + i)->a_type == A_SYM) {
                object_post((t_object*)x, "arg %ld: symbol (%s)", i,
                            atom_getsym(argv + i)->s_name);
            } else {
                object_error((t_object*)x, "forbidden argument");
            }
        }

        x->name = gensym("");
        x->outlet = bangout((t_object*)x);
        // x->py = new VM(true);    // instanciate the PocketPy VM
        x->py = new PktpyInterpreter(); // <-- can also be a struct

        // custom builtins
        add_custom_builtins(x);

        attr_args_process(x, argc, argv);
    }
    return (x);
}



void pktpy_bang(t_pktpy* x)
{
    outlet_bang(x->outlet);
}


/**
 * @brief Execute a max symbol as a line of python code
 *
 * @param x instance of t_pktpy struct
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * 
 * @return t_max_err error code
 */
t_max_err pktpy_exec(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->exec(s, argc, argv);
}

/**
 * @brief Evaluate a max symbol as a python expression
 *
 * @param x instance of t_pktpy struct
 * @param s symbol of object to be evaluated
 * @param argc atom argument count
 * @param argv atom argument vector
 *
 * @return t_max_err error code
 */
t_max_err pktpy_eval(t_pktpy* x, t_symbol* s, long argc, t_atom* argv) {
    return x->py->eval(s, argc, argv, x->outlet);
}


t_max_err pktpy_anything(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->anything(s, argc, argv, x->outlet);
}


// t_max_err pktpy_execfile(t_pktpy* x, t_symbol* s)
// {
//     return x->py->execfile(s);
// }

// t_max_err pktpy_assign(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
// {
//     return x->py->assign(s, argc, argv);
// }

// t_max_err pktpy_call(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
// {
//     return x->py->call(s, argc, argv, x->outlet);
// }

// t_max_err pktpy_pipe(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
// {
//     return x->py->pipe(s, argc, argv, x->outlet);
// }
