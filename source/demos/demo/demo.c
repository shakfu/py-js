#include "ext.h"
#include "ext_obex.h"
#include "cmx.h"
#include "ext_dictionary.h"
#include "ext_dictobj.h"


typedef struct _demo {
    t_object ob;

    void* outlet;         // outlet

    void *ob_proxy_1;     // inlet proxy
    void *ob_proxy_2;     // inlet proxy
    long ob_inletnum;     // # of inlet currently in use

    t_symbol* name;

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
t_max_err demo_name_get(t_demo *x, t_object *attr, long *argc, t_atom **argv);
t_max_err demo_name_set(t_demo *x, t_object *attr, long argc, t_atom *argv);
void demo_dict(t_demo* x, t_symbol* s);

// global class pointer variable
static t_class* demo_class = NULL;


void ext_main(void* r)
{
    t_class* c = class_new("demo", (method)demo_new,
                           (method)demo_free, (long)sizeof(t_demo), 0L,
                           A_GIMME, 0);

    class_addmethod(c, (method)demo_bang,  "bang", 0);
    class_addmethod(c, (method)demo_float, "float", A_FLOAT, 0);
    class_addmethod(c, (method)demo_dict,  "dict",  A_SYM,   0);

    CLASS_ATTR_LABEL(c,     "name", 0,  "patch-wide name");
    CLASS_ATTR_SYM(c,       "name", 0,  t_demo, name);
    CLASS_ATTR_BASIC(c,     "name", 0);
    CLASS_ATTR_SAVE(c,      "name", 0);
    CLASS_ATTR_ACCESSORS(c, "name", demo_name_get, demo_name_set);

    class_register(CLASS_BOX, c);
    demo_class = c;
}


void* demo_new(t_symbol* s, long argc, t_atom* argv)
{
    t_demo* x = (t_demo*)object_alloc(demo_class);
    post("DEMO");

    if (x) {
        x->outlet = outlet_new(x, NULL); // outlet

        // inlet proxy
        x->ob_inletnum = 0;
        // x->ob_proxy_2 = proxy_new(x, 2, &x->ob_inletnum);
        // x->ob_proxy_2 = proxy_new(x, 2, &x->ob_inletnum);
        // x->ob_proxy_1 = proxy_new(x, 1, &x->ob_inletnum);
        x->ob_proxy_2 = proxy_new(x, 2, NULL);
        x->ob_proxy_1 = proxy_new(x, 1, NULL);

        x->name = gensym("");

        x->param0 = 7.0;
        x->param1 = 500.0;
        x->param2 = 250.0;

        attr_args_process(x, argc, argv);

        post("x->param0: %f", x->param0);
        post("x->param1: %f", x->param1);
        post("x->param2: %f", x->param2);

        post("x->name: %s", x->name->s_name);
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
    t_string* path_to_ext = get_path_to_external(demo_class, NULL);
    const char* ext_path = string_getptr(path_to_ext);
    post("path to external: %s", ext_path);

    t_string* path_to_pkg = get_path_to_package(demo_class, "");
    const char* pkg_path = string_getptr(path_to_pkg);
    post("path to package: %s", pkg_path);

    char filename[MAX_PATH_CHARS];
    char directory[MAX_PATH_CHARS];

    path_dirname(pkg_path, directory);
    path_basename(pkg_path, filename);

    post("directory: %s", directory);
    post("filename: %s", filename);

    outlet_bang(x->outlet); 
}


t_max_err demo_name_get(t_demo *x, t_object *attr, long *argc, t_atom **argv)
{
    char alloc;

    if (argc && argv) {
        if (atom_alloc(argc, argv, &alloc)) {
                return MAX_ERR_OUT_OF_MEM;
            }
            if (alloc) {
                atom_setsym(*argv, x->name);
                post("demo_name_get: %s", x->name->s_name);
        }
    }
    return MAX_ERR_NONE;
}


t_max_err demo_name_set(t_demo *x, t_object *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        x->name = atom_getsym(argv);
        post("demo_name_set: %s", x->name->s_name);
    }
    return MAX_ERR_NONE;
}


void demo_dict(t_demo* x, t_symbol* s)
{
    long        argc = 0;
    t_atom      *argv = NULL;
    t_max_err   err;

    t_dictionary* d = dictionary_new();
    err = dictionary_appendstring(d, s, "def");
    err = dictobj_dictionarytoatoms(d, &argc, &argv);

    if (argc && argv) {
        // handles singles, lists, symbols, atomarrays, dictionaries, etc.
        dictobj_outlet_atoms(x->outlet, argc, argv);
    }

    if (argv)
        sysmem_freeptr(argv);
}


