/**
 * @file krait.cpp
 * @author shakfu (shak@fu.com)
 * @brief another experiment in mixing c, cpp, and python in a max external
 * @version 0.1
 * @date 2022-05-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "ext.h"
#include "ext_obex.h"

#define PY_INTERPRETER_IMPLEMENTATION
#include "py_interpreter.h"


typedef struct _krait {
    t_object ob;
    void* outlet;
    pyjs::PythonInterpreter* py;
} t_krait;


BEGIN_USING_C_LINKAGE

void* krait_new(t_symbol* s, long argc, t_atom* argv);
void krait_free(t_krait* x);
void krait_assist(t_krait* x, void* b, long m, long a, char* s);

void krait_bang(t_krait*);

t_max_err krait_import(t_krait* x, t_symbol* s);
t_max_err krait_eval(t_krait* x, t_symbol* s, long argc, t_atom* argv);
t_max_err krait_exec(t_krait* x, t_symbol* s, long argc, t_atom* argv);
t_max_err krait_execfile(t_krait* x, t_symbol* s);

// extra py methods
t_max_err krait_call(t_krait* x, t_symbol* s, long argc, t_atom* argv);
t_max_err krait_assign(t_krait* x, t_symbol* s, long argc, t_atom* argv); 
t_max_err krait_code(t_krait* x, t_symbol* s, long argc, t_atom* argv);
t_max_err krait_anything(t_krait* x, t_symbol* s, long argc, t_atom* argv);
t_max_err krait_pipe(t_krait* x, t_symbol* s, long argc, t_atom* argv);

END_USING_C_LINKAGE


static t_class* krait_class = NULL;

void ext_main(void* r)
{
    t_class* c;

    c = class_new("krait", 
            (method)krait_new,
            (method)krait_free,
            (long)sizeof(t_krait), 
            (method)NULL, /* leave NULL!! */
            A_GIMME,
            0L);

    class_addmethod(c, (method)krait_assist,     "assist",   A_CANT,     0);

    class_addmethod(c, (method)krait_bang,       "bang",                 0);
    class_addmethod(c, (method)krait_import,     "import",   A_SYM,      0);
    class_addmethod(c, (method)krait_eval,       "eval",     A_GIMME,    0);
    class_addmethod(c, (method)krait_exec,       "exec",     A_GIMME,    0);
    class_addmethod(c, (method)krait_execfile,   "execfile", A_DEFSYM,   0);

    class_addmethod(c, (method)krait_assign,     "assign",   A_GIMME,    0);
    class_addmethod(c, (method)krait_call,       "call",     A_GIMME,    0);
    class_addmethod(c, (method)krait_code,       "code",     A_GIMME,    0);
    class_addmethod(c, (method)krait_pipe,       "pipe",     A_GIMME,    0);
    class_addmethod(c, (method)krait_anything,   "anything", A_GIMME,    0);


    class_register(CLASS_BOX, c); /* CLASS_NOBOX */
    krait_class = c;

    post("I am the krait object");
}

void krait_assist(t_krait* x, void* b, long m, long a, char* s)
{
    if (m == ASSIST_INLET) { // inlet
        sprintf(s, "I am inlet %ld", a);
    } else { // outlet
        sprintf(s, "I am outlet %ld", a);
    }
}

void krait_free(t_krait* x)
{
    delete x->py;
}


void* krait_new(t_symbol* s, long argc, t_atom* argv)
{
    t_krait* x = NULL;
    long i;

    if ((x = (t_krait*)object_alloc(krait_class))) {
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
        x->outlet = bangout((t_object*)x);
        x->py = new pyjs::PythonInterpreter(); // <-- can also be a struct
    }
    return (x);
}

void krait_bang(t_krait* x)
{
    int argc = 0;
    t_atom* argv = NULL;

    PyObject* pval = x->py->eval_pcode_to_pval((char*)"[1,2,3]");

    assert(pval != NULL);

    x->py->plist_to_atoms(pval, &argc, &argv);

    post("argc: %d", argc);

    outlet_list(x->outlet, NULL, argc, argv);

    // outlet_bang(x->outlet);
}


t_max_err krait_import(t_krait* x, t_symbol* s)
{
    return x->py->import(s);
}


t_max_err krait_eval(t_krait* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->eval(s, argc, argv, x->outlet);
}


t_max_err krait_exec(t_krait* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->exec(s, argc, argv);
}


t_max_err krait_execfile(t_krait* x, t_symbol* s)
{
    return x->py->execfile(s);
}


t_max_err krait_call(t_krait* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->call(s, argc, argv, x->outlet);
}


t_max_err krait_assign(t_krait* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->assign(s, argc, argv);
}


t_max_err krait_code(t_krait* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->code(s, argc, argv, x->outlet);
}


t_max_err krait_anything(t_krait* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->anything(s, argc, argv, x->outlet);
}


t_max_err krait_pipe(t_krait* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->pipe(s, argc, argv, x->outlet);
}
