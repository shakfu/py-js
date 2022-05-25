/**
    @file mamba - an experimental attempt to modularize the python object

    The idea is that it can be included as a header and then used.
*/

#include "ext.h"
#include "ext_obex.h"

// INCLUDE THIS
#define PY_IMPLEMENTATION // <-- activate the implementation
#include "py.h"



typedef struct mamba
{
    t_object c_obj;
    t_py* py;           // <-- this is the key opaque type and instance
    void *c_outlet;

} t_mamba;


// prototypes
void *mamba_new(t_symbol *s, long argc, t_atom *argv);
void mamba_free(t_mamba *x);

// methods
void mamba_bang(t_mamba *x);
t_max_err mamba_import(t_mamba* x, t_symbol* s);
t_max_err mamba_eval(t_mamba* x, t_symbol* s, long argc, t_atom* argv);


static t_class *s_mamba_class = NULL;

void ext_main(void *r)
{
    t_class *c = class_new( "mamba", (method)mamba_new, (method)mamba_free, sizeof(t_mamba), (method)0L, A_GIMME, 0);

    class_addmethod(c, (method)mamba_bang,      "bang",              0);
    class_addmethod(c, (method)mamba_import,    "import",   A_SYM,   0);
    class_addmethod(c, (method)mamba_eval,      "eval",     A_GIMME, 0);

    class_register(CLASS_BOX, c);

    s_mamba_class = c;
}



void *mamba_new(t_symbol *s, long argc, t_atom *argv)
{
    t_mamba *x = (t_mamba *)object_alloc(s_mamba_class);

    x->c_outlet = bangout(x);

    x->py = py_init(); // This is all that is need to init the `py` obj
 
    attr_args_process(x, argc, argv);

    return x;
}


void mamba_free(t_mamba *x)
{
    py_free(x->py); // must be called in the free metho to cleanup python.
}


void mamba_bang(t_mamba *x)
{
    outlet_bang(x->c_outlet);
}


t_max_err mamba_import(t_mamba* x, t_symbol* s)
{
    return py_import(x->py, s); // returns t_max_err 
}


t_max_err mamba_eval(t_mamba* x, t_symbol* s, long argc, t_atom* argv)
{
    return py_eval(x->py, s, argc, argv, x->c_outlet);
}

