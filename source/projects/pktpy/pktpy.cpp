#include "pktpy.h"


typedef struct _pktpy {
    t_object ob;
    t_symbol* name;
    t_object* p_code_editor;    /*!< code editor object */
    char** p_code;              /*!< handle to code buffer for code editor */
    long p_code_size;           /*!< length of code buffer */
    long p_run_on_save;         /*!< evaluate/run code in editor on save */
    long p_run_on_close;        /*!< evaluate/run code in editor on close */
    void* outlet;
    PktpyInterpreter* py;
} t_pktpy;


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
// int add100(int a);


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
    class_addmethod(c, (method)pktpy_eval,       "eval",     A_GIMME,    0);
    class_addmethod(c, (method)pktpy_exec,       "exec",     A_GIMME,    0);
    class_addmethod(c, (method)pktpy_anything,   "anything", A_GIMME,    0);
    class_addmethod(c, (method)pktpy_execfile,   "execfile", A_DEFSYM,   0);

    // code editor
    class_addmethod(c, (method)pktpy_read,       "read",       A_DEFSYM,  0);
    class_addmethod(c, (method)pktpy_dblclick,   "dblclick",   A_CANT,    0);
    class_addmethod(c, (method)pktpy_edclose,    "edclose",    A_CANT,    0);
    class_addmethod(c, (method)pktpy_edsave,     "edsave",     A_CANT,    0);
    class_addmethod(c, (method)pktpy_load,       "load",       A_DEFSYM,  0);
    class_addmethod(c, (method)pktpy_run,        "run",        A_NOTHING, 0);
    class_addmethod(c, (method)pktpy_okclose,    "okclose",    A_CANT,    0);


    CLASS_ATTR_LABEL(c, "name", 0,  "unique object id");
    CLASS_ATTR_SYM(c, "name", 0,   t_pktpy, name);
    CLASS_ATTR_BASIC(c, "name", 0);

    class_register(CLASS_BOX, c); /* CLASS_NOBOX */
    pktpy_class = c;
}


t_symbol* pktpy_get_path_to_external(t_pktpy* x)
{
    return x->py->get_path_to_external(pktpy_class);
}



void pktpy_assist(t_pktpy* x, void* b, long m, long a, char* s)
{
    if (m == ASSIST_INLET) { // inlet
        snprintf(s, PY_MAX_ELEMS, "I am inlet %ld", a);
    } else { // outlet
        snprintf(s, PY_MAX_ELEMS, "I am outlet %ld", a);
    }
}



// example function to be wrapped by NativeProxyFunc
int add100(int a){
    return a + 100;
}

void add_custom_builtins(t_pktpy* x)
{
        // builtins
        x->py->p_vm->bind_builtin_func<1>("add10", [](VM* vm, Args& args) {
            i64 a = CAST(i64, args[0]);
            return VAR(a + 10);
        });

        // example of wrapping a max api function
        // bind: void *outlet_int(t_outlet *x, t_atom_long n);
        // >>> out_int(10) -> sends 10 out of outlet
        x->py->p_vm->bind_builtin_func<1>("out_int", [x](VM* vm, Args& args) {
            i64 a = CAST(i64, args[0]);
            outlet_int(x->outlet, a);
            return vm->None;
        });

        // wrap exist function pktpy_get_path_to_external
        x->py->p_vm->bind_builtin_func<0>("location", [x](VM* vm, Args& args) {
            t_symbol* sym = pktpy_get_path_to_external(x);
            outlet_anything(x->outlet, sym, 0, (t_atom*)NIL);
            return vm->None;
        });

        // example of wrapping function using NativeProxyFunc
        // It can be constructed from a function pointer,
        // a lambda function, or a std::function.
        x->py->p_vm->bind_builtin_func<1>("add100", NativeProxyFunc(&add100));
}



void pktpy_free(t_pktpy* x)
{
    // not required
}

void* pktpy_new(t_symbol* s, long argc, t_atom* argv)
{
    t_pktpy* x = NULL;
    long i;

    if ((x = (t_pktpy*)object_alloc(pktpy_class))) {
        object_post((t_object*)x, "a new %s object was created: %p",
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
        x->py = new PktpyInterpreter(); // <-- can also be a struct

        // text editor
        x->p_code = sysmem_newhandle(0);
        x->p_code_size = 0;
        x->p_code_editor = NULL;
        // x->p_code_filetype = FOUR_CHAR_CODE('TEXT');
        // x->p_code_outtype = 0;
        // x->p_code_filename[0] = 0;
        // x->p_code_pathname[0] = 0;
        // short p_code_path;
        // x->p_code_filepath = gensym("");
        // x->p_autoload = 0;
        x->p_run_on_save = 0;
        x->p_run_on_close = 1;
        // x->p_run_on = gensym("close");
        // custom builtins
        add_custom_builtins(x);

        attr_args_process(x, argc, argv);
    }
    return (x);
}



void pktpy_bang(t_pktpy* x)
{
    outlet_bang(x->outlet);
}


t_max_err pktpy_exec(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->exec(s, argc, argv);
}


t_max_err pktpy_eval(t_pktpy* x, t_symbol* s, long argc, t_atom* argv) {
    return x->py->eval(s, argc, argv, x->outlet);
}


t_max_err pktpy_anything(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
{
    return x->py->anything(s, argc, argv, x->outlet);
}


t_max_err pktpy_execfile(t_pktpy* x, t_symbol* s)
{
    return x->py->execfile(s);
}


// ---------------------------------------------------------------------------
// code-editor methods



/**
 * @brief Event of double-clicking on external object launches code-editor UI
 * 
 * @param x pointer to object structure
 * 
 */
void pktpy_dblclick(t_pktpy* x)
{
    if (x->p_code_editor)
        object_attr_setchar(x->p_code_editor, gensym("visible"), 1);
    else {
        x->p_code_editor = (t_object *)object_new(CLASS_NOBOX, gensym("jed"), x, 0);
        object_method(x->p_code_editor, gensym("settext"), *x->p_code,
                      gensym("utf-8"));
        object_attr_setchar(x->p_code_editor, gensym("scratch"), 1);
        object_attr_setsym(x->p_code_editor, gensym("title"),
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
    err = path_opensysfile((const char*)x->py->p_source_name, (const short)x->py->p_path_code, &fh, READ_PERM);
    if (!err) {
        sysfile_readtextfile(fh, x->p_code, 0, (t_sysfile_text_flags)(TEXT_LB_UNIX|TEXT_NULL_TERMINATE));
        sysfile_close(fh);
        x->p_code_size = sysmem_handlesize(x->p_code);
    }
}

// /** Read a text file from disk.
//     This function reads up to the maximum number of bytes given by 
//     maxlen from file handle at current file position into the htext 
//     handle, performing linebreak translation if set in flags.

//     @ingroup        files
//     @param  f       The #t_filehandle structure of the text file the user wants to open.
//     @param  htext   Handle that the data will be read into.
//     @param  maxlen  The maximum length in bytes to be read into the handle. 
//                     Passing the value 0L indicates no maximum (i.e. read the entire file).
//     @param  flags   Flags to set linebreak translation as defined in #t_sysfile_text_flags.
//     @return         An error code.
// */
// t_max_err sysfile_readtextfile(t_filehandle f, t_handle htext, t_ptr_size maxlen, t_sysfile_text_flags flags);



/**
 * @brief Run python code stored in editor buffer
 * 
 * @param x pointer to object structure
 */
t_max_err pktpy_run(t_pktpy* x)
{
    if ((*(x->p_code) != NULL) && (*(x->p_code)[0] == '\0'))
        // is empty string
        return MAX_ERR_GENERIC;

    return x->py->exec_pcode(*(x->p_code));
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
    if (x->p_code)
        sysmem_freehandle(x->p_code);

    x->p_code = sysmem_newhandleclear(size + 1);
    sysmem_copyptr((char*)*text, *x->p_code, size);
    x->p_code_size = size + 1;
    x->p_code_editor = NULL;
    if (x->p_run_on_close) {
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
void pktpy_okclose(t_pktpy* x, char *s, short *result)
{
    // see: https://cycling74.com/forums/text-editor-without-dirty-bit
    x->py->log_debug((char*)"okclose: called -- run-on-close: %d", x->p_run_on_close);
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
    if (x->p_run_on_save) {

        x->py->log_debug((char*)"run-on-save activated");

        return x->py->exec_pcode(*(x->p_code));
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

