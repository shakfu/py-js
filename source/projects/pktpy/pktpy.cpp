#include "pktpy.h"

typedef struct _pktpy {
    t_object ob;            /*!< object header */
    t_symbol* name;         /*!< unique object name */
    t_object* code_editor;  /*!< code editor object */
    char** code_buffer;     /*!< handle to code buffer for code editor */
    long code_size;         /*!< length of code buffer */
    long run_on_save;       /*!< evaluate/run code in editor on save */
    long run_on_close;      /*!< evaluate/run code in editor on close */
    void* outlet;           /*!< object outlet */
    PktpyInterpreter* py;   /*!< pktpy interpreter instance */
    PyObject* mod_dsp;      /*!< pktpy dsp native module */
} t_pktpy;

// clang-format off
BEGIN_USING_C_LINKAGE

void* pktpy_new(t_symbol* s, long argc, t_atom* argv);
void pktpy_free(t_pktpy* x);
void pktpy_assist(t_pktpy* x, void* b, long m, long a, char* s);

// core methods
void pktpy_bang(t_pktpy*);
t_max_err pktpy_eval(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
t_max_err pktpy_exec(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
t_max_err pktpy_anything(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
t_max_err pktpy_execfile(t_pktpy* x, t_symbol* s);

// code-editor methods
void pktpy_dblclick(t_pktpy* x);
void pktpy_read(t_pktpy* x, t_symbol* s);
void pktpy_load(t_pktpy* x, t_symbol* s); // read(f) -> execfile(f)
void pktpy_doread(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
t_max_err pktpy_run(t_pktpy* x);
void pktpy_edclose(t_pktpy* x, char** text, long size);
t_max_err pktpy_edsave(t_pktpy* x, char** text, long size);
void pktpy_okclose(t_pktpy* x, char *s, short *result);

// utilities
t_symbol* pktpy_get_path_to_external(t_pktpy* x);
void add_custom_builtins(t_pktpy* x);

END_USING_C_LINKAGE


/* -------------------------------------------------------------------------
 * EXTERNAL IMPLEMENTATION
 */


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

    class_addmethod(c, (method)pktpy_assist,     "assist",     A_CANT,     0);

    // message methods
    class_addmethod(c, (method)pktpy_bang,       "bang",                   0);
    class_addmethod(c, (method)pktpy_eval,       "eval",       A_GIMME,    0);
    class_addmethod(c, (method)pktpy_exec,       "exec",       A_GIMME,    0);
    class_addmethod(c, (method)pktpy_anything,   "anything",   A_GIMME,    0);
    class_addmethod(c, (method)pktpy_execfile,   "execfile",   A_DEFSYM,   0);

    // code editor
    class_addmethod(c, (method)pktpy_read,       "read",       A_DEFSYM,  0);
    class_addmethod(c, (method)pktpy_dblclick,   "dblclick",   A_CANT,    0);
    class_addmethod(c, (method)pktpy_edclose,    "edclose",    A_CANT,    0);
    class_addmethod(c, (method)pktpy_edsave,     "edsave",     A_CANT,    0);
    class_addmethod(c, (method)pktpy_load,       "load",       A_DEFSYM,  0);
    class_addmethod(c, (method)pktpy_run,        "run",        A_NOTHING, 0);
    class_addmethod(c, (method)pktpy_okclose,    "okclose",    A_CANT,    0);

    CLASS_ATTR_LABEL(c, "name", 0,  "unique object id");
    CLASS_ATTR_SYM(c,   "name", 0,   t_pktpy, name);
    CLASS_ATTR_BASIC(c, "name", 0);

    class_register(CLASS_BOX, c); /* CLASS_NOBOX */
    pktpy_class = c;
}
// clang-format on

/**
 * @brief      returns path to external
 *
 * @param      x     object instance
 *
 * @return     symbol path to external
 */
t_symbol* pktpy_get_path_to_external(t_pktpy* x)
{
    return x->py->get_path_to_external(pktpy_class);
}


void pktpy_assist(t_pktpy* x, void* b, long m, long a, char* s)
{
    if (m == ASSIST_INLET) { // inlet
        snprintf(s, PY_MAX_ELEMS, "I am inlet %ld", a);
    } else {                 // outlet
        snprintf(s, PY_MAX_ELEMS, "I am outlet %ld", a);
    }
}


/* -------------------------------------------------------------------------
 * constructor / destructor
 */

/**
 * @brief      external cleanup (called on object deletion)
 *
 * @param      x     object instance
 */
void pktpy_free(t_pktpy* x)
{
    // code editor cleanup
    object_free(x->code_editor);
    if (x->code_buffer)
        sysmem_freehandle(x->code_buffer);
}

/**
 * @brief      external initializer (called on creation)
 *
 * @param      s     symbol
 * @param[in]  argc  number of atoms
 * @param      argv  The arguments array
 *
 * @return     pointer to external instance
 */
void* pktpy_new(t_symbol* s, long argc, t_atom* argv)
{
    t_pktpy* x = NULL;
    long i;

    if ((x = (t_pktpy*)object_alloc(pktpy_class))) {
        object_post((t_object*)x, "a new %s object was created: %p", s->s_name,
                    x);
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
        x->py = new PktpyInterpreter(); // <-- can also be a struct

        // text editor
        x->code_buffer = sysmem_newhandle(0);
        x->code_size = 0;
        x->code_editor = NULL;
        x->run_on_save = 0;
        x->run_on_close = 1;

        // custom builtins
        add_custom_builtins(x);

        // set scripts directory as pythonpath
        post("setting pythonpath to scripts directory");
        x->py->set_scripts_path(pktpy_class);

        // process args to external object
        attr_args_process(x, argc, argv);
    }
    return (x);
}

/* -------------------------------------------------------------------------
 * set import path
 */



/* -------------------------------------------------------------------------
 * core message methods
 */

/**
 * @brief      bang method
 *
 * @param      x     object instance
 */
void pktpy_bang(t_pktpy* x) { outlet_bang(x->outlet); }

/**
 * @brief      { function_description }
 *
 * @param      x     object instance
 * @param      s     symbol
 * @param[in]  argc  no of atoms
 * @param      argv  atom array
 *
 * @return     The t maximum error.
 */
t_max_err pktpy_exec(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->exec2(s, argc, argv);
}

/**
 * @brief      { function_description }
 *
 * @param      x     object instance
 * @param      s     symbol
 * @param[in]  argc  no of atoms
 * @param      argv  atom array
 *
 * @return     The t maximum error.
 */
t_max_err pktpy_eval(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->eval(s, argc, argv, x->outlet);
}

/**
 * @brief      { function_description }
 *
 * @param      x     object instance
 * @param      s     symbol
 * @param[in]  argc  no of atoms
 * @param      argv  atom array
 *
 * @return     The t maximum error.
 */
t_max_err pktpy_anything(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->anything(s, argc, argv, x->outlet);
}

/**
 * @brief      { function_description }
 *
 * @param      x     object instance
 * @param      s     symbol
 *
 * @return     The t maximum error.
 */
t_max_err pktpy_execfile(t_pktpy* x, t_symbol* s)
{
    return x->py->execfile(s);
}


/* -------------------------------------------------------------------------
 * code-editor methods
 */

/**
 * @brief Event of double-clicking on external object launches code-editor UI
 *
 * @param x pointer to object structure
 *
 */
void pktpy_dblclick(t_pktpy* x)
{
    if (x->code_editor)
        object_attr_setchar(x->code_editor, gensym("visible"), 1);
    else {
        x->code_editor = (t_object*)object_new(CLASS_NOBOX, gensym("jed"), x, 0);
        object_method(x->code_editor, gensym("settext"), *x->code_buffer,
                      gensym("utf-8"));
        object_attr_setchar(x->code_editor, gensym("scratch"), 1);
        object_attr_setsym(x->code_editor, gensym("title"),
                           gensym("py-editor"));
    }
}


/**
 * @brief Read text file into code-editor.
 *
 * @param x pointer to object structure
 * @param s path to text file
 */
void pktpy_read(t_pktpy* x, t_symbol* s)
{
    defer((t_object*)x, (method)pktpy_doread, s, 0, NULL);
}

/**
 * @brief Read function callback
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 */
void pktpy_doread(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
{
    t_max_err err;
    t_filehandle fh;

    x->py->locate_path_from_symbol(s);
    err = path_opensysfile((const char*)x->py->source_name,
                           (const short)x->py->path_code, &fh, READ_PERM);
    if (!err) {
        sysfile_readtextfile(
            fh, x->code_buffer, 0,
            (t_sysfile_text_flags)(TEXT_LB_UNIX | TEXT_NULL_TERMINATE));
        sysfile_close(fh);
        x->code_size = sysmem_handlesize(x->code_buffer);
    }
}


/**
 * @brief Run python code stored in editor buffer
 *
 * @param x pointer to object structure
 */
t_max_err pktpy_run(t_pktpy* x)
{
    if ((*(x->code_buffer) != NULL) && (*(x->code_buffer)[0] == '\0'))
        // is empty string
        return MAX_ERR_GENERIC;

    return x->py->exec_pcode(*(x->code_buffer));
}


/**
 * @brief Event function to preserve text in buffer after editor is closed
 *
 * @param x pointer to object structure
 * @param text text to be saved to buffer
 * @param size size of text to be saved to buffer
 */
void pktpy_edclose(t_pktpy* x, char** text, long size)
{
    if (x->code_buffer)
        sysmem_freehandle(x->code_buffer);

    x->code_buffer = sysmem_newhandleclear(size + 1);
    sysmem_copyptr((char*)*text, *x->code_buffer, size);
    x->code_size = size + 1;
    x->code_editor = NULL;
    if (x->run_on_close) {
        pktpy_run(x);
    }
}


/**
 * @brief      { function_description }
 *
 * @param      x       pointer to object structure
 * @param      s       custom save text (optional)
 * @param      result  set values [0-4] to adjust what happens
 *                     how the system responds when the editor
 *                     window is closed.
 */
void pktpy_okclose(t_pktpy* x, char* s, short* result)
{
    // see: https://cycling74.com/forums/text-editor-without-dirty-bit
    x->py->log_debug((char*)"okclose: called -- run-on-close: %d",
                     x->run_on_close);
    *result = 3; // don't put up a dialog
    // const char *string = "custom save text";
    // memcpy(s, string, strlen(string)+1);
}

/**
 * @brief Provides run-code-on-save functionality to code-editor
 *
 * @param x pointer to object structure
 * @param text text to be run and saved
 * @param size size of text to be run and saved
 * @return t_max_err error code
 */
t_max_err pktpy_edsave(t_pktpy* x, char** text, long size)
{
    if (x->run_on_save) {

        x->py->log_debug((char*)"run-on-save activated");

        return x->py->exec_pcode(*(x->code_buffer));
    }
    return MAX_ERR_NONE;
}

/**
 * @brief Combo function of `pktpy_read <path> -> pktpy_execfile <path>`
 *
 * @param x pointer to object structure
 * @param s path as symbol
 */
void pktpy_load(t_pktpy* x, t_symbol* s)
{
    pktpy_read(x, s);
    pktpy_execfile(x, s);
}

/* -------------------------------------------------------------------------
 * custom class wrappers (can be used as python classes)
 */

class PyPoint {
private:
    float x;
    float y;

public:
    PY_CLASS(PyPoint, test, PyPoint)

    PyPoint(float x, float y) : x(x), y(y) {}

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<3>(type, [](VM* vm, ArgsView args){
            float x = CAST_F(args[1]);
            float y = CAST_F(args[2]);
            return VAR_T(PyPoint, x, y);
        });
        vm->bind_method<0>(type, "__repr__", [](VM* vm, ArgsView args){
            PyPoint& self = CAST(PyPoint&, args[0]);
            std::stringstream ss;
            ss << "PyPoint(" << self.x << ", " << self.y << ")";
            return VAR(ss.str());
        });
    }
};


/* -------------------------------------------------------------------------
 * custom builtin function wrappers (can be used as python functions)
 */

// example function to be wrapped by NativeProxyFunc
int add100(int a) { return a + 100; }


void add_custom_builtins(t_pktpy* x)
{
    // add dsp native module
    x->mod_dsp = x->py->new_module("dsp");

    x->py->bind_func<2>(x->mod_dsp, "add", [](VM* vm, ArgsView args) {
        f64 x = CAST(f64, args[0]);
        f64 y = CAST(f64, args[1]);
        return VAR(x + y);
    });


    PyObject* obj; // to handle `x` pointer capture case
    // see: https://github.com/blueloveTH/pocketpy/issues/89

    // builtins
    x->py->bind_builtin_func<1>("add10", [](VM* vm, ArgsView args) {
        i64 a = CAST(i64, args[0]);
        return VAR(a + 10);
    });

    x->py->bind_builtin_func<1>("add100", [](VM* vm, ArgsView args) {
        i64 a = CAST(i64, args[0]);
        return VAR(add100(a));
    });

    // example of wrapping a max api function
    // bind: void *outlet_int(t_outlet *x, t_atom_long n);
    // >>> out_int(10) -> sends 10 out of outlet
    obj = x->py->bind_builtin_func<1>("out_int", [](VM* vm, ArgsView args) {
        t_pktpy *x = lambda_get_userdata<t_pktpy *>(args.begin());
        i64 a = CAST(i64, args[0]);
        outlet_int(x->outlet, a);
        return vm->None;
    });
    py_cast<NativeFunc&>(x->py, obj).set_userdata(x);

    // wrap existing function pktpy_get_path_to_external
    obj = x->py->bind_builtin_func<0>("location", [](VM* vm, ArgsView args) {
        t_pktpy *x = lambda_get_userdata<t_pktpy *>(args.begin());
        t_symbol* sym = pktpy_get_path_to_external(x);
        outlet_anything(x->outlet, sym, 0, (t_atom*)NIL);
        return vm->None;
    });
    py_cast<NativeFunc&>(x->py, obj).set_userdata(x);

    // wrap existing function pktpy_load
    obj = x->py->bind_builtin_func<1>("load", [](VM* vm, ArgsView args) {
        t_pktpy *x = lambda_get_userdata<t_pktpy *>(args.begin());
        Str path = CAST(Str, args[0]);
        const char* path_cstr = path.c_str_dup();
        pktpy_load(x, gensym(path_cstr)); 
        return vm->None;
    });
    py_cast<NativeFunc&>(x->py, obj).set_userdata(x);

    // wrap max-api function newobject_fromboxtext as create(text: str)
    obj = x->py->bind_builtin_func<1>("create", [](VM* vm, ArgsView args) {
        t_pktpy *x = lambda_get_userdata<t_pktpy *>(args.begin());
        Str text = CAST(Str, args[0]);
        const char* text_cstr = text.c_str_dup();
        t_object *patcher;//, *obj;
        t_max_err err;
        if (object_obex_lookup(x, gensym("#P"), &patcher) == MAX_ERR_NONE)
            // obj = newobject_fromboxtext(patcher, text_cstr);
            newobject_fromboxtext(patcher, text_cstr);
        return vm->None;
    });
    py_cast<NativeFunc&>(x->py, obj).set_userdata(x);

    PyObject* mod = x->py->new_module("test");
    PyPoint::register_class(x->py, mod);

}

