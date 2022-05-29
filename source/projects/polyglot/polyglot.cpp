/**
 * @file polyglot.cpp
 * @author shakfu (shak@fu.com)
 * @brief basic experiment in mixing c, cpp, and python in a max external
 * @version 0.1
 * @date 2022-05-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "ext.h"
#include "ext_obex.h"

#include "../mamba/py.h"


class PythonInterpreter {
    private:
        t_py* self;

    public:
        PythonInterpreter()
        {
            self = py_init();
        }

        ~PythonInterpreter()
        {
            py_free(self);
            self = nullptr;
        }

        t_max_err import(t_symbol* s)
        {
            return py_import(self, s);
        }

        t_max_err eval(t_symbol* s, long argc, t_atom* argv, void* outlet)
        {
            return py_eval(self, s, argc, argv, outlet);
        }

        t_max_err exec(t_symbol* s, long argc, t_atom* argv)
        {
            return py_exec(self, s, argc, argv);
        }

        t_max_err execfile(t_symbol* s)
        {
            return py_execfile(self, s);
        }

        t_max_err call(t_symbol* s, long argc, t_atom* argv, void* outlet)
        {
            return py_call(self, s, argc, argv, outlet); 
        }

        t_max_err assign(t_symbol* s, long argc, t_atom* argv)
        {
            return py_assign(self, s, argc, argv);
        }

        t_max_err code(t_symbol* s, long argc, t_atom* argv, void* outlet)
        {
            return py_code(self, s, argc, argv, outlet);
        }

        t_max_err anything(t_symbol* s, long argc, t_atom* argv, void* outlet)
        {
            return py_anything(self, s, argc, argv, outlet);
        }

        t_max_err pipe(t_symbol* s, long argc, t_atom* argv, void* outlet)
        {
            return py_pipe(self, s, argc, argv, outlet);
        }

        void status(void)
        {
            post("py interpreter status: 0");
        }
};


typedef struct _polyglot {
    t_object ob;
    void* outlet;
    PythonInterpreter* py;
} t_polyglot;


BEGIN_USING_C_LINKAGE

void* polyglot_new(t_symbol* s, long argc, t_atom* argv);
void polyglot_free(t_polyglot* x);
void polyglot_assist(t_polyglot* x, void* b, long m, long a, char* s);

void polyglot_bang(t_polyglot*);

t_max_err polyglot_import(t_polyglot* x, t_symbol* s);
t_max_err polyglot_eval(t_polyglot* x, t_symbol* s, long argc, t_atom* argv);
t_max_err polyglot_exec(t_polyglot* x, t_symbol* s, long argc, t_atom* argv);
t_max_err polyglot_execfile(t_polyglot* x, t_symbol* s);

// extra py methods
t_max_err polyglot_call(t_polyglot* x, t_symbol* s, long argc, t_atom* argv);
t_max_err polyglot_assign(t_polyglot* x, t_symbol* s, long argc, t_atom* argv); 
t_max_err polyglot_code(t_polyglot* x, t_symbol* s, long argc, t_atom* argv);
t_max_err polyglot_anything(t_polyglot* x, t_symbol* s, long argc, t_atom* argv);
t_max_err polyglot_pipe(t_polyglot* x, t_symbol* s, long argc, t_atom* argv);


END_USING_C_LINKAGE

static t_class* polyglot_class = NULL;

void ext_main(void* r)
{
    t_class* c;

    c = class_new("polyglot", 
            (method)polyglot_new,
            (method)polyglot_free,
            (long)sizeof(t_polyglot), 
            (method)NULL, /* leave NULL!! */
            A_GIMME,
            0L);

    class_addmethod(c, (method)polyglot_assist,     "assist",   A_CANT,     0);

    class_addmethod(c, (method)polyglot_bang,       "bang",                 0);
    class_addmethod(c, (method)polyglot_import,     "import",   A_SYM,      0);
    class_addmethod(c, (method)polyglot_eval,       "eval",     A_GIMME,    0);
    class_addmethod(c, (method)polyglot_exec,       "exec",     A_GIMME,    0);
    class_addmethod(c, (method)polyglot_execfile,   "execfile", A_DEFSYM,   0);

    class_addmethod(c, (method)polyglot_assign,     "assign",   A_GIMME,    0);
    class_addmethod(c, (method)polyglot_call,       "call",     A_GIMME,    0);
    class_addmethod(c, (method)polyglot_code,       "code",     A_GIMME,    0);
    class_addmethod(c, (method)polyglot_pipe,       "pipe",     A_GIMME,    0);
    class_addmethod(c, (method)polyglot_anything,   "anything", A_GIMME,    0);


    class_register(CLASS_BOX, c); /* CLASS_NOBOX */
    polyglot_class = c;

    post("I am the polyglot object");
}

void polyglot_assist(t_polyglot* x, void* b, long m, long a, char* s)
{
    if (m == ASSIST_INLET) { // inlet
        sprintf(s, "I am inlet %ld", a);
    } else { // outlet
        sprintf(s, "I am outlet %ld", a);
    }
}

void polyglot_free(t_polyglot* x)
{
    // py_free(x->py);
    delete x->py;
}


void* polyglot_new(t_symbol* s, long argc, t_atom* argv)
{
    t_polyglot* x = NULL;
    long i;

    if ((x = (t_polyglot*)object_alloc(polyglot_class))) {
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
    }

    x->outlet = bangout((t_object*)x);
    x->py = new PythonInterpreter(); // <-- can also be a struct

    return (x);
}

void polyglot_bang(t_polyglot* x)
{
    outlet_bang(x->outlet);
}

t_max_err polyglot_import(t_polyglot* x, t_symbol* s)
{
    return x->py->import(s);
}

t_max_err polyglot_eval(t_polyglot* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->eval(s, argc, argv, x->outlet);
}


t_max_err polyglot_exec(t_polyglot* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->exec(s, argc, argv);
}


t_max_err polyglot_execfile(t_polyglot* x, t_symbol* s)
{
    return x->py->execfile(s);
}


t_max_err polyglot_call(t_polyglot* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->call(s, argc, argv, x->outlet);
}


t_max_err polyglot_assign(t_polyglot* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->assign(s, argc, argv);
}


t_max_err polyglot_code(t_polyglot* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->code(s, argc, argv, x->outlet);
}


t_max_err polyglot_anything(t_polyglot* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->anything(s, argc, argv, x->outlet);
}


t_max_err polyglot_pipe(t_polyglot* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->pipe(s, argc, argv, x->outlet);
}
