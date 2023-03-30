#include "ext.h"
#include "ext_obex.h"
#include "cmx.h"

typedef struct _demo {
    t_object ob;
    void *ob_proxy_1;     // inlet proxy
    void *ob_proxy_2;     // inlet proxy
    long ob_inletnum;     // # of inlet currently in use

    // params
    float param0;
    float param1;
    float param2;
} t_demo;




// method prototypes
void* demo_new(t_symbol* s, long argc, t_atom* argv);
void demo_free(t_demo* x);
void demo_bang(t_demo* x);
void demo_float(t_demo *x, double f);

// global class pointer variable
static t_class* demo_class = NULL;


void ext_main(void* r)
{
    t_class* c = class_new("demo", (method)demo_new,
                           (method)demo_free, (long)sizeof(t_demo), 0L,
                           A_GIMME, 0);

    class_addmethod(c, (method)demo_bang,  "bang", 0);
    class_addmethod(c, (method)demo_float, "float", A_FLOAT, 0);

    class_register(CLASS_BOX, c);
    demo_class = c;
}


void* demo_new(t_symbol* s, long argc, t_atom* argv)
{
    t_demo* x = (t_demo*)object_alloc(demo_class);

    if (x) {
        outlet_new(x, NULL); // outlet

        // inlet proxy
        x->ob_inletnum = 0;
        // x->ob_proxy_2 = proxy_new(x, 2, &x->ob_inletnum);
        // x->ob_proxy_2 = proxy_new(x, 2, &x->ob_inletnum);
        // x->ob_proxy_1 = proxy_new(x, 1, &x->ob_inletnum);
        x->ob_proxy_2 = proxy_new(x, 2, NULL);
        x->ob_proxy_1 = proxy_new(x, 1, NULL);

        x->param0 = 7.0;
        x->param1 = 500.0;
        x->param2 = 250.0;

        post("x->param0: %f", x->param0);
        post("x->param1: %f", x->param1);
        post("x->param2: %f", x->param2);
    }
    return (x);
}

void demo_free(t_demo* x) {
    object_free(x->ob_proxy_2);
    object_free(x->ob_proxy_1);
}


void demo_float(t_demo *x, double f)
{
    switch (proxy_getinlet((t_object *)x))
    {
    case 0:
        post("param0 f: %f", f);
        break;

    case 1:
        post("param1 f: %f", f);
        break;

    case 2:
        post("param2 f: %f", f);
        break;

    default:
        error("demo_float switch out-of-index");
    }
}


void demo_bang(t_demo* x) {
    // example of using libcmx.a (common max lib)
    t_symbol* path = locate_path_to_external(demo_class);
    post("path: %s", path->s_name);
    outlet_bang(x->ob_proxy_1); 
}

