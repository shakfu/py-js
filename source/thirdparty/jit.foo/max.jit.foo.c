/*
        Copyright 2001 - Cycling '74
        Joshua Kit Clayton jkc@cycling74.com
*/

#include "jit.common.h"

typedef struct _max_jit_foo
{
    t_object ob;
    void* obex;
} t_max_jit_foo;

t_class* max_jit_foo_class;

void max_jit_foo_assist(t_max_jit_foo* x, void* b, long m, long a, char* s);
void* max_jit_foo_new(t_symbol* s, long argc, t_atom* argv);
void max_jit_foo_free(t_max_jit_foo* x);

// from jit.foo.c
t_jit_err jit_foo_init(void);

C74_EXPORT void ext_main(void* r)
{
    void *q;
    t_class* c = NULL;

    jit_foo_init();

    c = class_new("jit.foo",
        (method)max_jit_foo_new,
        (method)max_jit_foo_free,
        (long)sizeof(t_max_jit_foo), 0L, A_GIMME, 0);

    max_jit_class_obex_setup(c, calcoffset(t_max_jit_foo, obex));
    q = jit_class_findbyname(gensym("jit_foo"));
    max_jit_class_wrap_standard(c, q, 0);

    class_addmethod(c, (method)max_jit_foo_assist, "assist", A_CANT, 0);

    class_register(CLASS_BOX, c);

    max_jit_foo_class = c;
}

void max_jit_foo_assist(t_max_jit_foo* x, void* b, long m, long a, char* s)
{
    // nada for now
}

void max_jit_foo_free(t_max_jit_foo* x)
{
    jit_object_free(max_jit_obex_jitob_get(x));
    max_jit_object_free(x);
}

void* max_jit_foo_new(t_symbol* s, long argc, t_atom* argv)
{
    t_max_jit_foo* x;
    long attrstart;
    t_symbol* text = gensym("Hello World!");
    void* o;

    x = (t_max_jit_foo*)max_jit_object_alloc(max_jit_foo_class, gensym("jit_foo"));

    if (x) {
        max_jit_obex_dumpout_set(x, outlet_new(x, 0L)); // general purpose outlet(rightmost)

        // get normal args
        attrstart = max_jit_attr_args_offset(argc, argv);
        if (attrstart && argv) {
            jit_atom_arg_getsym(&text, 0, attrstart, argv);
        }
        o = jit_object_new(gensym("jit_foo"), text);
        if (o) {
            max_jit_obex_jitob_set(x, o);
        }
        else {
            freeobject((void*)x);
            x = NULL;
            jit_object_error((t_object*)x, "jit.foo: out of memory");
            goto out;
        }
    }

out:
    return (x);
}
