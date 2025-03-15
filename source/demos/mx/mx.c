#include "ext.h"
#include "ext_obex.h"

// minimal test object

typedef struct _mx
{
    t_object ob;  // the object itself (must be first)
    void* outlet; // one outlet
} t_mx;

void* mx_new(t_symbol* s, long argc, t_atom* argv);
void mx_free(t_mx* x);
void mx_assist(t_mx* x, void* b, long m, long a, char* s);
void mx_bang(t_mx* x);

void* mx_class;

void ext_main(void* r)
{
    t_class* c;

    c = class_new("mx", (method)mx_new, (method)mx_free, (long)sizeof(t_mx),
                  0L /* leave NULL!! */, A_GIMME, 0);

    /* you CAN'T call this from the patcher */
    class_addmethod(c, (method)mx_assist, "assist", A_CANT, 0);
    class_addmethod(c, (method)mx_bang,   "bang",   0);

    class_register(CLASS_BOX, c); /* CLASS_NOBOX */
    mx_class = c;

    post("I am the mx object");
}

void mx_assist(t_mx* x, void* b, long m, long a, char* s)
{
    if (m == ASSIST_INLET) { // inlet
        sprintf(s, "I am inlet %ld", a);
    }
    else { // outlet
        sprintf(s, "I am outlet %ld", a);
    }
}

void mx_free(t_mx* x)
{
    ;
}

void* mx_new(t_symbol* s, long argc, t_atom* argv)
{
    t_mx* x = NULL;
    long i;

    if ((x = (t_mx*)object_alloc(mx_class))) {

        x->outlet = outlet_new(x, NULL);

    }
    return (x);
}


void mx_bang(t_mx* x)
{
    outlet_bang(x->outlet);
}

