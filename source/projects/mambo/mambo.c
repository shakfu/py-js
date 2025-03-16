/**
    @file mambo - an experimental attempt to modularize the python object

    The idea is that it can be included as a header and then used in any
   external context.
*/

#include "ext.h"
#include "ext_obex.h"

#define PY_IMPLEMENTATION // <-- activate the implementation
#include "py.h"           // <-- include this


typedef struct mambo {
    t_object c_obj;
    t_py* py; // <-- this is the key opaque type and instance
    void* c_outlet;
} t_mambo;


// prototypes
void* mambo_new(t_symbol* s, long argc, t_atom* argv);
void mambo_free(t_mambo* x);

// generic methods
void mambo_bang(t_mambo* x);

// core py methods
t_max_err mambo_import(t_mambo* x, t_symbol* s);
t_max_err mambo_eval(t_mambo* x, t_symbol* s, long argc, t_atom* argv);
t_max_err mambo_exec(t_mambo* x, t_symbol* s, long argc, t_atom* argv);
t_max_err mambo_execfile(t_mambo* x, t_symbol* s);

// extra py methods
t_max_err mambo_call(t_mambo* x, t_symbol* s, long argc, t_atom* argv, void* outlet);
t_max_err mambo_assign(t_mambo* x, t_symbol* s, long argc, t_atom* argv);
t_max_err mambo_code(t_mambo* x, t_symbol* s, long argc, t_atom* argv, void* outlet);
t_max_err mambo_anything(t_mambo* x, t_symbol* s, long argc, t_atom* argv, void* outlet);
t_max_err mambo_pipe(t_mambo* x, t_symbol* s, long argc, t_atom* argv, void* outlet);


static t_class* mambo_class = NULL;

void ext_main(void* r)
{
    t_class* c = class_new("mambo", (method)mambo_new, (method)mambo_free,
                           sizeof(t_mambo), (method)0L, A_GIMME, 0);

    class_addmethod(c, (method)mambo_bang,      "bang", 0);

    class_addmethod(c, (method)mambo_import,    "import",   A_SYM, 0);
    class_addmethod(c, (method)mambo_eval,      "eval",     A_GIMME, 0);
    class_addmethod(c, (method)mambo_exec,      "exec",     A_GIMME, 0);
    class_addmethod(c, (method)mambo_execfile,  "execfile", A_DEFSYM, 0);

    class_addmethod(c, (method)mambo_assign,    "assign",   A_GIMME, 0);
    class_addmethod(c, (method)mambo_call,      "call",     A_GIMME, 0);
    class_addmethod(c, (method)mambo_code,      "code",     A_GIMME, 0);
    class_addmethod(c, (method)mambo_pipe,      "pipe",     A_GIMME, 0);
    class_addmethod(c, (method)mambo_anything,  "anything", A_GIMME, 0);

    class_register(CLASS_BOX, c);

    mambo_class = c;
}

/**
 * @brief Mambo new method
 *
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 *
 * @return pointer to mambo object
 */
void* mambo_new(t_symbol* s, long argc, t_atom* argv)
{
    t_mambo* x = (t_mambo*)object_alloc(mambo_class);
    x->c_outlet = bangout(x);
    x->py = py_init(mambo_class); // pass class for possible relocatable builds,
                                  // or NULL for strictly local builds.
    return x;
}

/**
 * @brief Mambo free method
 *
 * @param x pointer to mambo object
 */
void mambo_free(t_mambo* x)
{
    py_free(x->py); // must be called in the free method to cleanup python.
}

/**
 * @brief Mambo bang method
 *
 * @param x pointer to mambo object
 */
void mambo_bang(t_mambo* x) { outlet_bang(x->c_outlet); }


/**
 * @brief Mambo import method
 *
 * @param x pointer to mambo object
 * @param s symbol
 *
 * @return t_max_err
 */
t_max_err mambo_import(t_mambo* x, t_symbol* s)
{
    return py_import(x->py, s); // returns t_max_err
}

/**
 * @brief Mambo eval method
 *
 * @param x pointer to mambo object
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 *
 * @return t_max_err
 */
t_max_err mambo_eval(t_mambo* x, t_symbol* s, long argc, t_atom* argv)
{
    return py_eval(x->py, s, argc, argv, x->c_outlet);
}

/**
 * @brief Mambo exec method
 *
 * @param x pointer to mambo object
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 *
 * @return t_max_err
 */
t_max_err mambo_exec(t_mambo* x, t_symbol* s, long argc, t_atom* argv)
{
    return py_exec(x->py, s, argc, argv);
}

/**
 * @brief Mambo execfile method
 *
 * @param x pointer to mambo object
 * @param s symbol
 *
 * @return t_max_err
 */
t_max_err mambo_execfile(t_mambo* x, t_symbol* s)
{
    return py_execfile(x->py, s);
}

/**
 * @brief Mambo call method
 *
 * @param x pointer to mambo object
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param outlet object outlet
 *
 * @return t_max_err
 */
t_max_err mambo_call(t_mambo* x, t_symbol* s, long argc, t_atom* argv, void* outlet)
{
    return py_call(x->py, s, argc, argv, x->c_outlet);
}

/**
 * @brief Mambo assign method
 *
 * @param x pointer to mambo object
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 *
 * @return t_max_err
 */
t_max_err mambo_assign(t_mambo* x, t_symbol* s, long argc, t_atom* argv)
{
    return py_assign(x->py, s, argc, argv);
}


/**
 * @brief Mambo code method
 *
 * @param x pointer to mambo object
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param outlet object outlet
 *
 * @return t_max_err
 */
t_max_err mambo_code(t_mambo* x, t_symbol* s, long argc, t_atom* argv, void* outlet)
{
    return py_code(x->py, s, argc, argv, x->c_outlet);
}

/**
 * @brief Mambo anything method
 *
 * @param x pointer to mambo object
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param outlet object outlet
 *
 * @return t_max_err
 */
t_max_err mambo_anything(t_mambo* x, t_symbol* s, long argc, t_atom* argv, void* outlet)
{
    return py_anything(x->py, s, argc, argv, x->c_outlet);
}

/**
 * @brief Mambo pipe method
 *
 * @param x pointer to mambo object
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param outlet object outlet
 *
 * @return t_max_err
 */
t_max_err mambo_pipe(t_mambo* x, t_symbol* s, long argc, t_atom* argv, void* outlet)
{
    return py_pipe(x->py, s, argc, argv, x->c_outlet);
}


