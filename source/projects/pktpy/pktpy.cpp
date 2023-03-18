/**
 * @file pktpy.cpp
 * @author shakfu (https://github.com/shakfu)
 * @brief another experiment in mixing c, cpp, and python in a max external
 * @version 0.1
 * @date 2022-05-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "ext.h"
#include "ext_obex.h"

#include "pocketpy.h"

#define PY_MAX_ELEMS 1024

typedef struct _pktpy {
    t_object ob;
    t_symbol* name;
    void* outlet;
    pkpy::VM* py; // PocketPy VM object is named `py` for familiarity
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
// t_max_err pktpy_import(t_pktpy* x, t_symbol* s);
t_max_err pktpy_eval(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
t_max_err pktpy_exec(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
// t_max_err pktpy_execfile(t_pktpy* x, t_symbol* s);

// extra py methods
// t_max_err pktpy_call(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
// t_max_err pktpy_assign(t_pktpy* x, t_symbol* s, long argc, t_atom* argv); 
// t_max_err pktpy_code(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
// t_max_err pktpy_anything(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
// t_max_err pktpy_pipe(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);

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
    // class_addmethod(c, (method)pktpy_import,     "import",   A_SYM,      0);
    class_addmethod(c, (method)pktpy_eval,       "eval",     A_GIMME,    0);
    class_addmethod(c, (method)pktpy_exec,       "exec",     A_GIMME,    0);
    // class_addmethod(c, (method)pktpy_execfile,   "execfile", A_DEFSYM,   0);

    // class_addmethod(c, (method)pktpy_assign,     "assign",   A_GIMME,    0);
    // class_addmethod(c, (method)pktpy_call,       "call",     A_GIMME,    0);
    // class_addmethod(c, (method)pktpy_code,       "code",     A_GIMME,    0);
    // class_addmethod(c, (method)pktpy_pipe,       "pipe",     A_GIMME,    0);
    // class_addmethod(c, (method)pktpy_anything,   "anything", A_GIMME,    0);

    CLASS_ATTR_LABEL(c, "name", 0,  "unique object id");
    CLASS_ATTR_SYM(c, "name", 0,   t_pktpy, name);
    CLASS_ATTR_BASIC(c, "name", 0);
    // CLASS_ATTR_ACCESSORS(c, "name", NULL, pktpy_name_set);


    class_register(CLASS_BOX, c); /* CLASS_NOBOX */
    pktpy_class = c;

    post("I am the pktpy object");
}

void pktpy_assist(t_pktpy* x, void* b, long m, long a, char* s)
{
    if (m == ASSIST_INLET) { // inlet
        snprintf(s, PY_MAX_ELEMS, "I am inlet %ld", a);
    } else { // outlet
        snprintf(s, PY_MAX_ELEMS, "I am outlet %ld", a);
    }
}

void pktpy_free(t_pktpy* x)
{
    pkpy_delete(x->py);
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
        x->py = pkpy_new_vm(false); // instanciate the PocketPy VM

        attr_args_process(x, argc, argv);
    }
    return (x);
}

void pktpy_bang(t_pktpy* x)
{
    outlet_bang(x->outlet);
}


// t_max_err pktpy_import(t_pktpy* x, t_symbol* s)
// {
//     return x->py->import(s);
// }

/**
 * @brief Execute a max symbol as a line of python code
 *
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return t_max_err error code
 */
t_max_err pktpy_exec(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
{
    char* pcode = atom_getsym(argv)->s_name;
    if (pcode == NULL) {
        return MAX_ERR_GENERIC;
    }
    // x->py->exec(pcode, "main.py", EXEC_MODE);
    pkpy_vm_exec(x->py, pcode);
    return MAX_ERR_NONE;
}


t_max_err pktpy_eval(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
{
    char* pcode = atom_getsym(argv)->s_name;

    if (pcode == NULL) {
        return MAX_ERR_GENERIC;
    }

    char* result = pkpy_vm_eval(x->py, pcode);

    outlet_anything(x->outlet, gensym(result), 0, (t_atom*)NIL);

    pkpy_delete(result);

    return MAX_ERR_GENERIC;
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


// t_max_err pktpy_code(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
// {
//     return x->py->code(s, argc, argv, x->outlet);
// }


// t_max_err pktpy_anything(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
// {
//     return x->py->anything(s, argc, argv, x->outlet);
// }


// t_max_err pktpy_pipe(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
// {
//     return x->py->pipe(s, argc, argv, x->outlet);
// }

