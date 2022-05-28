/**
 * @file polyglot.cpp
 * @author shakfu (shak@fu.com)
 * @brief basic experiment in mixing cpp and c in a max external
 * @version 0.1
 * @date 2022-05-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "ext.h"
#include "ext_obex.h"

typedef struct person {
    int age;
    t_symbol* name;

    person(int n, char* a_name)
    {
        age = n;
        name = gensym(a_name);
    }
    int get_age() { return age; }
    t_symbol* get_name() { return name; }
} person;


typedef struct _polyglot {
    t_object ob;
    void* outlet;
    person sam;
} t_polyglot;


BEGIN_USING_C_LINKAGE

void* polyglot_new(t_symbol* s, long argc, t_atom* argv);
void polyglot_free(t_polyglot* x);
void polyglot_assist(t_polyglot* x, void* b, long m, long a, char* s);
void polyglot_bang(t_polyglot*);

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

    class_addmethod(c, (method)polyglot_assist, "assist", A_CANT, 0);
    class_addmethod(c, (method)polyglot_bang, "bang", 0);


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

void polyglot_free(t_polyglot* x) { ; }


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
    x->sam = person(100, (char*)"sammy");

    return (x);
}

void polyglot_bang(t_polyglot* x)
{
    post("age: %d", x->sam.get_age());
    post("name: %s", x->sam.get_name()->s_name);
    outlet_bang(x->outlet);
}