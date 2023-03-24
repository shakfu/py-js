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

// core methods
void pktpy_bang(t_pktpy*);
t_max_err pktpy_eval(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
t_max_err pktpy_exec(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
t_max_err pktpy_anything(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
// t_max_err pktpy_execfile(t_pktpy* x, t_symbol* s);


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
    class_addmethod(c, (method)pktpy_anything,   "anything", A_GIMME,    0);
    // class_addmethod(c, (method)pktpy_execfile,   "execfile", A_DEFSYM,   0);

    CLASS_ATTR_LABEL(c, "name", 0,  "unique object id");
    CLASS_ATTR_SYM(c, "name", 0,   t_pktpy, name);
    CLASS_ATTR_BASIC(c, "name", 0);

    class_register(CLASS_BOX, c); /* CLASS_NOBOX */
    pktpy_class = c;
}


t_symbol* pktpy_get_path_to_external(t_pktpy* x)
{
    return x->py->get_path_to_external(pktpy_class);
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
    // not required
}

void* pktpy_new(t_symbol* s, long argc, t_atom* argv)
{
    t_pktpy* x = NULL;
    long i;

    if ((x = (t_pktpy*)object_alloc(pktpy_class))) {
        object_post((t_object*)x, "a new %s object was created: %p",
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


t_max_err pktpy_exec(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->exec(s, argc, argv);
}


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

