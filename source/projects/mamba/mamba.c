/**
    @file mamba - an experimental attempt to modularize the python object

    The idea is that it can be included as a header and then used in any
   external context.
*/

#include "ext.h"
#include "ext_obex.h"

#define PY_IMPLEMENTATION // <-- activate the implementation
#include "py.h"           // <-- include this


typedef struct mamba {
    t_object c_obj;
    t_py* py; // <-- this is the key opaque type and instance
    void* c_outlet;
} t_mamba;


// prototypes
void* mamba_new(t_symbol* s, long argc, t_atom* argv);
void mamba_free(t_mamba* x);

// generic methods
void mamba_bang(t_mamba* x);

// core py methods
t_max_err mamba_import(t_mamba* x, t_symbol* s);
t_max_err mamba_eval(t_mamba* x, t_symbol* s, long argc, t_atom* argv);
t_max_err mamba_exec(t_mamba* x, t_symbol* s, long argc, t_atom* argv);
t_max_err mamba_execfile(t_mamba* x, t_symbol* s);

// extra py methods
t_max_err mamba_call(t_mamba* x, t_symbol* s, long argc, t_atom* argv, void* outlet);
t_max_err mamba_assign(t_mamba* x, t_symbol* s, long argc, t_atom* argv);
t_max_err mamba_code(t_mamba* x, t_symbol* s, long argc, t_atom* argv, void* outlet);
t_max_err mamba_anything(t_mamba* x, t_symbol* s, long argc, t_atom* argv, void* outlet);
t_max_err mamba_pipe(t_mamba* x, t_symbol* s, long argc, t_atom* argv, void* outlet);


static t_class* s_mamba_class = NULL;

void ext_main(void* r)
{
    t_class* c = class_new("mamba", (method)mamba_new, (method)mamba_free,
                           sizeof(t_mamba), (method)0L, A_GIMME, 0);

    class_addmethod(c, (method)mamba_bang,      "bang", 0);

    class_addmethod(c, (method)mamba_import,    "import",   A_SYM, 0);
    class_addmethod(c, (method)mamba_eval,      "eval",     A_GIMME, 0);
    class_addmethod(c, (method)mamba_exec,      "exec",     A_GIMME, 0);
    class_addmethod(c, (method)mamba_execfile,  "execfile", A_DEFSYM, 0);

    class_addmethod(c, (method)mamba_assign,    "assign",   A_GIMME, 0);
    class_addmethod(c, (method)mamba_call,      "call",     A_GIMME, 0);
    class_addmethod(c, (method)mamba_code,      "code",     A_GIMME, 0);
    class_addmethod(c, (method)mamba_pipe,      "pipe",     A_GIMME, 0);
    class_addmethod(c, (method)mamba_anything,  "anything", A_GIMME, 0);

    class_register(CLASS_BOX, c);

    s_mamba_class = c;
}

/**
 * @brief Mamba new method
 *
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 *
 * @return pointer to mamba object
 */
void* mamba_new(t_symbol* s, long argc, t_atom* argv)
{
    t_mamba* x = (t_mamba*)object_alloc(s_mamba_class);
    x->c_outlet = bangout(x);
    x->py = py_init(); // This is all that is need to init the `py` obj
    return x;
}

/**
 * @brief Mamba free method
 *
 * @param x pointer to mamba object
 */
void mamba_free(t_mamba* x)
{
    py_free(x->py); // must be called in the free metho to cleanup python.
}

/**
 * @brief Mamba bang method
 *
 * @param x pointer to mamba object
 */
void mamba_bang(t_mamba* x) { outlet_bang(x->c_outlet); }


/**
 * @brief Mamba import method
 *
 * @param x pointer to mamba object
 * @param s symbol
 *
 * @return t_max_err
 */
t_max_err mamba_import(t_mamba* x, t_symbol* s)
{
    return py_import(x->py, s); // returns t_max_err
}

/**
 * @brief Mamba eval method
 *
 * @param x pointer to mamba object
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 *
 * @return t_max_err
 */
t_max_err mamba_eval(t_mamba* x, t_symbol* s, long argc, t_atom* argv)
{
    return py_eval(x->py, s, argc, argv, x->c_outlet);
}

/**
 * @brief Mamba exec method
 *
 * @param x pointer to mamba object
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 *
 * @return t_max_err
 */
t_max_err mamba_exec(t_mamba* x, t_symbol* s, long argc, t_atom* argv)
{
    return py_exec(x->py, s, argc, argv);
}

/**
 * @brief Mamba execfile method
 *
 * @param x pointer to mamba object
 * @param s symbol
 *
 * @return t_max_err
 */
t_max_err mamba_execfile(t_mamba* x, t_symbol* s)
{
    return py_execfile(x->py, s);
}

/**
 * @brief Mamba call method
 *
 * @param x pointer to mamba object
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param outlet object outlet
 *
 * @return t_max_err
 */
t_max_err mamba_call(t_mamba* x, t_symbol* s, long argc, t_atom* argv, void* outlet)
{
    return py_call(x->py, s, argc, argv, x->c_outlet);
}

/**
 * @brief Mamba assign method
 *
 * @param x pointer to mamba object
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 *
 * @return t_max_err
 */
t_max_err mamba_assign(t_mamba* x, t_symbol* s, long argc, t_atom* argv)
{
    return py_assign(x->py, s, argc, argv);
}


/**
 * @brief Mamba code method
 *
 * @param x pointer to mamba object
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param outlet object outlet
 *
 * @return t_max_err
 */
t_max_err mamba_code(t_mamba* x, t_symbol* s, long argc, t_atom* argv, void* outlet)
{
    return py_code(x->py, s, argc, argv, x->c_outlet);
}

/**
 * @brief Mamba anything method
 *
 * @param x pointer to mamba object
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param outlet object outlet
 *
 * @return t_max_err
 */
t_max_err mamba_anything(t_mamba* x, t_symbol* s, long argc, t_atom* argv, void* outlet)
{
    return py_anything(x->py, s, argc, argv, x->c_outlet);
}

/**
 * @brief Mamba pipe method
 *
 * @param x pointer to mamba object
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param outlet object outlet
 *
 * @return t_max_err
 */
t_max_err mamba_pipe(t_mamba* x, t_symbol* s, long argc, t_atom* argv, void* outlet)
{
    return py_pipe(x->py, s, argc, argv, x->c_outlet);
}
