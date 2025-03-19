/**
 * @file cobra.cpp
 * @author shakfu (https://github.com/shakfu)
 * @brief another experiment in mixing c, cpp, and python in a max external
 * @version 0.1.2
 * @date 2022-05-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "ext.h"
#include "ext_obex.h"

#define PY_INTERPRETER_IMPLEMENTATION
#include "py_interpreter.h"


typedef struct _cobra {
    t_object ob;
    t_symbol* name;
    void* outlet;
    pyjs::PythonInterpreter* py;
} t_cobra;


BEGIN_USING_C_LINKAGE

void* cobra_new(t_symbol* s, long argc, t_atom* argv);
void cobra_free(t_cobra* x);
void cobra_assist(t_cobra* x, void* b, long m, long a, char* s);

// attr getters / setters
t_max_err cobra_name_get(t_cobra* x, t_object* attr, long* argc, t_atom** argv);
t_max_err cobra_name_set(t_cobra* x, t_object* attr, long argc, t_atom* argv);


// basic methods
void cobra_bang(t_cobra*);

// core methods
t_max_err cobra_import(t_cobra* x, t_symbol* s);
t_max_err cobra_eval(t_cobra* x, t_symbol* s, long argc, t_atom* argv);
t_max_err cobra_exec(t_cobra* x, t_symbol* s, long argc, t_atom* argv);
t_max_err cobra_execfile(t_cobra* x, t_symbol* s);

// extra py methods
t_max_err cobra_call(t_cobra* x, t_symbol* s, long argc, t_atom* argv);
t_max_err cobra_assign(t_cobra* x, t_symbol* s, long argc, t_atom* argv); 
t_max_err cobra_code(t_cobra* x, t_symbol* s, long argc, t_atom* argv);
t_max_err cobra_anything(t_cobra* x, t_symbol* s, long argc, t_atom* argv);
t_max_err cobra_pipe(t_cobra* x, t_symbol* s, long argc, t_atom* argv);

END_USING_C_LINKAGE


static t_class* cobra_class = NULL;

void ext_main(void* r)
{
    t_class* c;

    c = class_new("cobra", 
            (method)cobra_new,
            (method)cobra_free,
            (long)sizeof(t_cobra), 
            (method)NULL, /* leave NULL!! */
            A_GIMME,
            0L);

    class_addmethod(c, (method)cobra_assist,     "assist",   A_CANT,     0);

    class_addmethod(c, (method)cobra_bang,       "bang",                 0);
    class_addmethod(c, (method)cobra_import,     "import",   A_SYM,      0);
    class_addmethod(c, (method)cobra_eval,       "eval",     A_GIMME,    0);
    class_addmethod(c, (method)cobra_exec,       "exec",     A_GIMME,    0);
    class_addmethod(c, (method)cobra_execfile,   "execfile", A_DEFSYM,   0);

    class_addmethod(c, (method)cobra_assign,     "assign",   A_GIMME,    0);
    class_addmethod(c, (method)cobra_call,       "call",     A_GIMME,    0);
    class_addmethod(c, (method)cobra_code,       "code",     A_GIMME,    0);
    class_addmethod(c, (method)cobra_pipe,       "pipe",     A_GIMME,    0);
    class_addmethod(c, (method)cobra_anything,   "anything", A_GIMME,    0);

    CLASS_ATTR_LABEL(c, "name", 0,  "unique object id");
    CLASS_ATTR_SYM(c, "name", 0,   t_cobra, name);
    CLASS_ATTR_BASIC(c, "name", 0);


    class_register(CLASS_BOX, c); /* CLASS_NOBOX */
    cobra_class = c;

    post("I am the cobra object");
}

void cobra_assist(t_cobra* x, void* b, long m, long a, char* s)
{
    if (m == ASSIST_INLET) { // inlet
        snprintf(s, PY_MAX_ELEMS, "I am inlet %ld", a);
    } else { // outlet
        snprintf(s, PY_MAX_ELEMS, "I am outlet %ld", a);
    }
}

void cobra_free(t_cobra* x)
{
    delete x->py;
}


void* cobra_new(t_symbol* s, long argc, t_atom* argv)
{
    t_cobra* x = NULL;
    long i;

    if ((x = (t_cobra*)object_alloc(cobra_class))) {
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
        x->py = new pyjs::PythonInterpreter(cobra_class); // <-- can also be a struct

        attr_args_process(x, argc, argv);
    }
    return (x);
}

void cobra_bang(t_cobra* x)
{
    outlet_bang(x->outlet);
}


t_max_err cobra_import(t_cobra* x, t_symbol* s)
{
    return x->py->import(s);
}


t_max_err cobra_eval(t_cobra* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->eval(s, argc, argv, x->outlet);
}


t_max_err cobra_exec(t_cobra* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->exec(s, argc, argv);
}


t_max_err cobra_execfile(t_cobra* x, t_symbol* s)
{
    return x->py->execfile(s);
}


t_max_err cobra_call(t_cobra* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->call(s, argc, argv, x->outlet);
}


t_max_err cobra_assign(t_cobra* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->assign(s, argc, argv);
}


t_max_err cobra_code(t_cobra* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->code(s, argc, argv, x->outlet);
}


t_max_err cobra_anything(t_cobra* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->anything(s, argc, argv, x->outlet);
}


t_max_err cobra_pipe(t_cobra* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->pipe(s, argc, argv, x->outlet);
}


