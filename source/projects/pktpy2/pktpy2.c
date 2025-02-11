#include "ext.h"
#include "ext_obex.h"
// #include "cmx.h"

#include "pocketpy.h"


// ----------------------------------------------------------------------------
// constants

#define PY_MAX_ELEMS 1024
#define ITER_SUCCESS 1
#define ITER_STOP 0
#define ITER_FAILURE (-1)

// ----------------------------------------------------------------------------
// missing macros from pocketpy.h v2.0.5

#define py_checklist(self) py_checktype(self, tp_list)
#define py_checktuple(self) py_checktype(self, tp_tuple)
#define py_checkdict(self) py_checktype(self, tp_dict)

// ----------------------------------------------------------------------------
// datastructure

typedef struct _pktpy2 {
    t_object ob;
    void *ob_proxy_1;           /// inlet proxy
    void *ob_proxy_2;           /// inlet proxy
    long ob_inletnum;           /// # of inlet currently in use

    t_symbol* name;             /// name of external instance

    // params
    float param0;               /// param-0
    float param1;               /// param-1
    float param2;               /// param-2

    // text editor attrs
    t_object* p_code_editor;    /// code editor object
    char** p_code;              /// handle to code buffer for code editor
    long p_code_size;           /// length of code buffer
    t_fourcc p_code_filetype;   /// filetype four char code of 'TEXT'
    t_fourcc p_code_outtype;    /// savetype four char code of 'TEXT'
    char p_code_filename[MAX_PATH_CHARS]; /// file name field
    char p_code_pathname[MAX_PATH_CHARS]; /// file path field
    short p_code_path;          /// short code for max file system
    long p_run_on_save;         /// evaluate/run code in editor on save
    long p_run_on_close;        /// evaluate/run code in editor on close

    t_symbol* p_code_filepath;  /// default python filepath to load into
                                /// the code editor and 'globals' namespace
    t_bool p_autoload;          /// bool to autoload of p_code_filepath

    // outlet creation
    void* p_outlet_right;       /// right outlet to bang success
    void* p_outlet_middle;      /// middle outleet to bang error
    void* p_outlet_left;        /// left outleet for msg output
} t_pktpy2;


// ----------------------------------------------------------------------------
// method prototypes

// init/free methods
void* pktpy2_new(t_symbol* s, long argc, t_atom* argv);
void pktpy2_free(t_pktpy2* x);

// informational methods
void pktpy2_bang(t_pktpy2* x);
void pktpy2_bang_success(t_pktpy2* x);
void pktpy2_bang_failure(t_pktpy2* x);

// output handlers
t_max_err pktpy2_handle_output(t_pktpy2* x, py_GlobalRef retval);
t_max_err pktpy2_handle_float_output(t_pktpy2* x, py_GlobalRef pfloat);
t_max_err pktpy2_handle_long_output(t_pktpy2* x, py_GlobalRef plong);
t_max_err pktpy2_handle_string_output(t_pktpy2* x, py_GlobalRef pstring);
t_max_err pktpy2_handle_bool_output(t_pktpy2* x, py_GlobalRef pbool);
t_max_err pktpy2_handle_list_output(t_pktpy2* x, py_GlobalRef plist);
t_max_err pktpy2_handle_tuple_output(t_pktpy2* x, py_GlobalRef ptuple);

// code editor / execfile methods
t_max_err pktpy2_locate_path_from_symbol(t_pktpy2* x, t_symbol* s);
void pktpy2_read(t_pktpy2* x, t_symbol* s);
void pktpy2_doread(t_pktpy2* x, t_symbol* s, long argc, t_atom* argv);
void pktpy2_load(t_pktpy2* x, t_symbol* s); // read(f) -> execfile(f)
void pktpy2_dblclick(t_pktpy2* x);
void pktpy2_run(t_pktpy2* x);
void pktpy2_edclose(t_pktpy2* x, char** text, long size);
t_max_err pktpy2_edsave(t_pktpy2* x, char** text, long size);
void pktpy2_okclose(t_pktpy2* x, char *s, short *result);

// core methods
t_max_err pktpy2_import(t_pktpy2* x, t_symbol* s);
t_max_err pktpy2_exec(t_pktpy2* x, t_symbol* s, long argc, t_atom* argv);
t_max_err pktpy2_eval(t_pktpy2* x, t_symbol* s, long argc, t_atom* argv);
t_max_err pktpy2_execfile(t_pktpy2* x, t_symbol* s);

// utility methods
void pktpy2_float(t_pktpy2 *x, double f);
t_max_err pktpy2_name_get(t_pktpy2 *x, t_object *attr, long *argc, t_atom **argv);
t_max_err pktpy2_name_set(t_pktpy2 *x, t_object *attr, long argc, t_atom *argv);


// ----------------------------------------------------------------------------
// global class pointer variable

static t_class* pktpy2_class = NULL;


// ----------------------------------------------------------------------------
// custom pktpy2 functions

bool int_add(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    py_i64 a = py_toint(py_arg(0));
    py_i64 b = py_toint(py_arg(1));
    py_newint(py_retval(), a + b);
    return true;
}

void print_to_console(const char * content)
{
    post(content);
}


static bool hello_add(int argc, py_Ref argv){
    PY_CHECK_ARGC(2);
    return py_binaryadd(py_arg(0), py_arg(1));
}

bool hello_module_initialize(void) {
    py_GlobalRef mod = py_newmodule("hello");
    py_bindfunc(mod, "add", hello_add);
    py_assign(py_retval(), mod);
    return true;
}

t_max_err demo(void) {
    // Hello world!
    bool ok = py_exec("print('Hello world!')", "<string>", EXEC_MODE, NULL);
    if(!ok) goto __ERROR;

    // Create a list: [1, 2, 3]
    py_Ref r0 = py_getreg(0);
    py_newlistn(r0, 3);
    py_newint(py_list_getitem(r0, 0), 1);
    py_newint(py_list_getitem(r0, 1), 2);
    py_newint(py_list_getitem(r0, 2), 3);

    // Eval the sum of the list
    py_Ref f_sum = py_getbuiltin(py_name("sum"));
    py_push(f_sum);
    py_pushnil();
    py_push(r0);
    ok = py_vectorcall(1, 0);
    if(!ok) goto __ERROR;

    post("Sum of the list: %d\n", (int)py_toint(py_retval()));  // 6

    // Bind native `int_add` as a global variable
    py_newnativefunc(r0, int_add);
    py_setglobal(py_name("add"), r0);

    // Call `add` in python
    ok = py_exec("add(3, 7)", "<string>", EVAL_MODE, NULL);
    if(!ok) goto __ERROR;

    py_i64 res = py_toint(py_retval());
    post("Sum of 2 variables: %d\n", (int)res);  // 10

    return MAX_ERR_NONE;

__ERROR:
    py_printexc();
    return MAX_ERR_GENERIC;
}

// ----------------------------------------------------------------------------
// external methods

void pktpy2_init(void)
{
    // Initialize pocketpy
    py_initialize();

    // redirect stdout
    py_Callbacks* callbacks = py_callbacks();
    callbacks->print = print_to_console;

    // Bind native `int_add` as a global variable
    py_Ref r0 = py_getreg(0);
    py_newnativefunc(r0, int_add);
    py_setglobal(py_name("add"), r0);

    // bind native module 'hello'
    hello_module_initialize();
}


void ext_main(void* r)
{
    t_class* c = class_new("pktpy2", (method)pktpy2_new,
                           (method)pktpy2_free, (long)sizeof(t_pktpy2), 0L,
                           A_GIMME, 0);

    class_addmethod(c, (method)pktpy2_bang,  "bang", 0);
    class_addmethod(c, (method)pktpy2_float, "float", A_FLOAT, 0);

    class_addmethod(c, (method)pktpy2_import,     "import",     A_SYM,     0);
    class_addmethod(c, (method)pktpy2_eval,       "eval",       A_GIMME,   0);
    class_addmethod(c, (method)pktpy2_exec,       "exec",       A_GIMME,   0);
    class_addmethod(c, (method)pktpy2_execfile,   "execfile",   A_DEFSYM,  0);

    // code editor
    class_addmethod(c, (method)pktpy2_read,       "read",       A_DEFSYM,  0);
    class_addmethod(c, (method)pktpy2_dblclick,   "dblclick",   A_CANT,    0);
    class_addmethod(c, (method)pktpy2_edclose,    "edclose",    A_CANT,    0);
    class_addmethod(c, (method)pktpy2_edsave,     "edsave",     A_CANT,    0);
    class_addmethod(c, (method)pktpy2_load,       "load",       A_DEFSYM,  0);
    class_addmethod(c, (method)pktpy2_run,        "run",        A_NOTHING, 0);
    class_addmethod(c, (method)pktpy2_okclose,    "okclose",    A_CANT,    0);

    // attrs
    CLASS_ATTR_LABEL(c,     "name", 0,  "patch-wide name");
    CLASS_ATTR_SYM(c,       "name", 0,  t_pktpy2, name);
    CLASS_ATTR_BASIC(c,     "name", 0);
    CLASS_ATTR_SAVE(c,      "name", 0);
    CLASS_ATTR_ACCESSORS(c, "name", pktpy2_name_get, pktpy2_name_set);

    class_register(CLASS_BOX, c);
    pktpy2_class = c;
}


void* pktpy2_new(t_symbol* s, long argc, t_atom* argv)
{
    t_pktpy2* x = (t_pktpy2*)object_alloc(pktpy2_class);

    if (x) {
        // inlet proxy
        x->ob_inletnum = 0;
        x->ob_proxy_2 = proxy_new(x, 2, NULL);
        x->ob_proxy_1 = proxy_new(x, 1, NULL);

        // text editor
        x->p_code = sysmem_newhandle(0);
        x->p_code_size = 0;
        x->p_code_editor = NULL;
        x->p_code_filetype = FOUR_CHAR_CODE('TEXT');
        x->p_code_outtype = 0;
        x->p_code_filename[0] = 0;
        x->p_code_pathname[0] = 0;
        x->p_code_filepath = gensym("");
        x->p_autoload = 0;
        x->p_run_on_save = 0;
        x->p_run_on_close = 1;

        // create outlet(s)
        x->p_outlet_right = bangout((t_object*)x);
        x->p_outlet_middle = bangout((t_object*)x);
        x->p_outlet_left = outlet_new(x, NULL);

        x->name = gensym("");

        x->param0 = 7.0;
        x->param1 = 500.0;
        x->param2 = 250.0;

        attr_args_process(x, argc, argv);

        post("x->param0: %f", x->param0);
        post("x->param1: %f", x->param1);
        post("x->param2: %f", x->param2);

        post("x->name: %s", x->name->s_name);

        pktpy2_init();

    }
    return (x);
}

void pktpy2_free(t_pktpy2* x) {
    object_free(x->ob_proxy_2);
    object_free(x->ob_proxy_1);
    py_finalize();
}


/**
 * @brief Generic handler to output arbitrarily-typed pocketpy object as max object
 *
 * @param x pointer to object struct
 * @param retval pocketpy return value
 * @return t_max_err error code
 */
t_max_err pktpy2_handle_output(t_pktpy2* x, py_GlobalRef retval)
{
    if (retval == NULL) {
        object_error((t_object*)x, "cannot handle NULL value");
        return MAX_ERR_GENERIC;
    }

    if (py_isfloat(retval)) {
        return pktpy2_handle_float_output(x, retval);
    }

    else if (py_isint(retval)) {
        return pktpy2_handle_long_output(x, retval);
    }

    else if (py_isstr(retval)) {
        return pktpy2_handle_string_output(x, retval);
    }

    else if(py_isbool(retval)) {
        return pktpy2_handle_bool_output(x, retval);
    }

    else if(py_islist(retval)) {
        return pktpy2_handle_list_output(x, retval);
    }

    else if(py_istuple(retval)) {
        return pktpy2_handle_tuple_output(x, retval);
    }

    // else if (py_isdict(retval)) {
    //     return pktpy2_handle_dict_output(x, retval);
    // }

    else if (py_isnone(retval)) {
        object_error((t_object*)x, "cannot handle a None type value");
        return MAX_ERR_GENERIC;
    }

    else {
        object_error((t_object*)x, "cannot handle his type of value");
        return MAX_ERR_GENERIC;
    }
}


/**
 * @brief Handler to output python float as max float
 *
 * @param x pointer to object struct
 * @param pfloat python float
 * @return t_max_err error code
 *
 */
t_max_err pktpy2_handle_float_output(t_pktpy2* x, py_GlobalRef pfloat)
{
    if (pfloat == NULL) {
        goto error;
    }

    if (py_checkfloat(pfloat)) {
        double float_result = py_tofloat(pfloat);
        outlet_float(x->p_outlet_left, float_result);
        pktpy2_bang_success(x);
    }
    return MAX_ERR_NONE;

error:
    object_error((t_object*)x, "pktpy2_handle_float_output failed");
    pktpy2_bang_failure(x);
    return MAX_ERR_GENERIC;
}


/**
 * @brief Handler to output python long as max int
 *
 * @param x pointer to object struct
 * @param plong python long
 * @return t_max_err error code
 */
t_max_err pktpy2_handle_long_output(t_pktpy2* x, py_GlobalRef plong)
{
    if (plong == NULL) {
        goto error;
    }

    if (py_checkint(plong)) {
        long long_result = py_toint(plong);
        outlet_int(x->p_outlet_left, long_result);
        pktpy2_bang_success(x);
    }

    return MAX_ERR_NONE;

error:
    object_error((t_object*)x, "pktpy2_handle_long_output failed");
    pktpy2_bang_failure(x);
    return MAX_ERR_GENERIC;
}


/**
 * @brief Handler to output python string as max symbol
 *
 * @param x pointer to object struct
 * @param pstring python string
 * @return t_max_err error code
 */
t_max_err pktpy2_handle_string_output(t_pktpy2* x, py_GlobalRef pstring)
{
    if (pstring == NULL) {
        goto error;
    }

    if (py_checkstr(pstring)) {
        const char* unicode_result = py_tostr(pstring);
        if (unicode_result == NULL) {
            goto error;
        }
        outlet_anything(x->p_outlet_left, gensym(unicode_result), 0, NIL);
        pktpy2_bang_success(x);
    }

    return MAX_ERR_NONE;

error:
    object_error((t_object*)x, "pktpy2_handle_string_output failed");
    pktpy2_bang_failure(x);
    return MAX_ERR_GENERIC;
}


/**
 * @brief Handler to output python bool as max int
 *
 * @param x pointer to object struct
 * @param plong python bool
 * @return t_max_err error code
 */
t_max_err pktpy2_handle_bool_output(t_pktpy2* x, py_GlobalRef pbool)
{
    if (pbool == NULL) {
        goto error;
    }

    if (py_checkbool(pbool)) {
        bool bool_result = py_tobool(pbool);
        if (bool_result) {
            outlet_int(x->p_outlet_left, 1);
        } else {
            outlet_int(x->p_outlet_left, 0);
        }
        pktpy2_bang_success(x);
    }

    return MAX_ERR_NONE;

error:
    object_error((t_object*)x, "pktpy2_handle_bool_output failed");
    pktpy2_bang_failure(x);
    return MAX_ERR_GENERIC;
}


/**
 * @brief Handler to output python list as max list
 *
 * @param x pointer to object struct
 * @param plist python list
 * @return t_max_err error code
 */
t_max_err pktpy2_handle_list_output(t_pktpy2* x, py_GlobalRef plist)
{
    if (plist == NULL) {
        goto error;
    }

    if (py_checklist(plist)) {
        int i = 0;

        t_atom atoms_static[PY_MAX_ELEMS];
        t_atom* atoms = NULL;
        int is_dynamic = 0;

        int seq_size = py_list_len(plist);
        object_post((t_object*)x, "seq_size: %d", seq_size);

        if (seq_size == 0) {
            object_error((t_object*)x, "cannot convert py list of length 0 to atoms");
            goto error;
        }

        if (seq_size > PY_MAX_ELEMS) {
            object_post((t_object*)x, "dynamically increasing size of atom array");
            atoms = atom_dynamic_start(atoms_static, PY_MAX_ELEMS, seq_size + 1);
            is_dynamic = 1;

        } else {
            atoms = atoms_static;
        }

        bool ok = py_iter(plist);
        if (!ok) {
            goto error;
        }

        // use register here
        py_GlobalRef iter = py_retval();
        py_setreg(0, iter);

        while (true) {
            int res = py_next(py_getreg(0));
            if (res == ITER_FAILURE) {
                object_error((t_object*)x, "pktpy2_handle_list_output iter failed");
                goto error;
            }

            if (res == ITER_STOP)
                break;

            py_GlobalRef item = py_retval();

            if (py_isint(item)) {
                long long_item = py_toint(item);
                atom_setlong(atoms + i, long_item);
                object_post((t_object*)x, "%d long: %ld\n", i, long_item);
                i++;
            }

            if (py_isfloat(item)) {
                double float_item = py_isfloat(item);
                atom_setfloat(atoms + i, float_item);
                object_post((t_object*)x, "%d float: %f\n", i, float_item);
                i++;
            }

            if (py_isstr(item)) {
                const char* unicode_item = py_tostr(item);
                if (unicode_item == NULL) {
                    goto error;
                }
                atom_setsym(atoms + i, gensym(unicode_item));
                object_post((t_object*)x, "%d unicode: %s\n", i, unicode_item);
                i++;
            }
        }

        outlet_list(x->p_outlet_left, NULL, i, atoms);
        pktpy2_bang_success(x);
        object_post((t_object*)x, "end iter op: %d", i);

        if (is_dynamic) {
            object_post((t_object*)x, "restoring to static atom array");
            atom_dynamic_end(atoms_static, atoms);
        }
    }

    return MAX_ERR_NONE;

error:
    object_error((t_object*)x, "pktpy2_handle_list_output failed");
    pktpy2_bang_failure(x);
    return MAX_ERR_GENERIC;
}


/**
 * @brief Handler to output python tuple as max list
 *
 * @param x pointer to object struct
 * @param ptuple python tuple
 * @return t_max_err error code
 */
t_max_err pktpy2_handle_tuple_output(t_pktpy2* x, py_GlobalRef ptuple)
{
    if (ptuple == NULL) {
        goto error;
    }

    if (py_checktuple(ptuple)) {
        int i = 0;

        t_atom atoms_static[PY_MAX_ELEMS];
        t_atom* atoms = NULL;
        int is_dynamic = 0;

        int seq_size = py_tuple_len(ptuple);
        object_post((t_object*)x, "seq_size: %d", seq_size);

        if (seq_size == 0) {
            object_error((t_object*)x, "cannot convert py tuple of length 0 to atoms");
            goto error;
        }

        if (seq_size > PY_MAX_ELEMS) {
            object_post((t_object*)x, "dynamically increasing size of atom array");
            atoms = atom_dynamic_start(atoms_static, PY_MAX_ELEMS, seq_size + 1);
            is_dynamic = 1;

        } else {
            atoms = atoms_static;
        }

        bool ok = py_iter(ptuple);
        if (!ok) {
            goto error;
        }

        // use register here
        py_GlobalRef iter = py_retval();
        py_setreg(0, iter);

        while (true) {
            int res = py_next(py_getreg(0));
            if (res == ITER_FAILURE) {
                object_error((t_object*)x, "pktpy2_handle_tuple_output iter failed");
                goto error;
            }

            if (res == ITER_STOP)
                break;

            py_GlobalRef item = py_retval();

            if (py_isint(item)) {
                long long_item = py_toint(item);
                atom_setlong(atoms + i, long_item);
                object_post((t_object*)x, "%d long: %ld\n", i, long_item);
                i++;
            }

            if (py_isfloat(item)) {
                double float_item = py_isfloat(item);
                atom_setfloat(atoms + i, float_item);
                object_post((t_object*)x, "%d float: %f\n", i, float_item);
                i++;
            }

            if (py_isstr(item)) {
                const char* unicode_item = py_tostr(item);
                if (unicode_item == NULL) {
                    goto error;
                }
                atom_setsym(atoms + i, gensym(unicode_item));
                object_post((t_object*)x, "%d unicode: %s\n", i, unicode_item);
                i++;
            }
        }

        outlet_list(x->p_outlet_left, NULL, i, atoms);
        pktpy2_bang_success(x);
        object_post((t_object*)x, "end iter op: %d", i);

        if (is_dynamic) {
            object_post((t_object*)x, "restoring to static atom array");
            atom_dynamic_end(atoms_static, atoms);
        }
    }

    return MAX_ERR_NONE;

error:
    object_error((t_object*)x, "pktpy2_handle_tuple_output failed");
    pktpy2_bang_failure(x);
    return MAX_ERR_GENERIC;
}


/**
 * @brief Handler to output python dict as max list
 *
 * @param x pointer to object struct
 * @param pdict python dict
 * @return t_max_err error code
 */
// t_max_err pktpy2_handle_dict_output(t_py* x, py_GlobalRef pdict)
// {
//     PyObject* pfun = NULL;
//     PyObject* pval = NULL;

//     if (pdict == NULL) {
//         goto error;
//     }

//     if (py_checkdict(pdict)) {

//         pfun = PyDict_GetItemString(x->p_globals, "out_dict");
//         if (pfun == NULL) {
//             py_error(x, "retrieving out_dict func from globals failed");
//             goto error;
//         }

//         pval = PyObject_CallFunctionObjArgs(pfun, pdict, NULL);
//         if (pval == NULL) {
//             py_error(x, "out_dict call failed to retrieve result");
//             goto error;
//         }

//         if (PyList_Check(pval)) {           // expecting a python list
//             py_handle_list_output(x, pval); // this decrefs pval
//             py_bang_success(x);
//             return MAX_ERR_NONE;
//         } else {
//             py_error(x, "expected list output got something else");
//             goto error;
//         }
//     }

// error:
//     object_error((t_object*)x, "pktpy2_handle_dict_output failed");
//     py_bang_failure(x);
//     return MAX_ERR_GENERIC;
// }


/**
 * @brief Import a python module
 *
 * @param x pointer to object structure
 * @param s symbol of module to be imported
 * @return t_max_err error code
 */
t_max_err pktpy2_import(t_pktpy2* x, t_symbol* s)
{

    if (s != gensym("")) {
        int result = py_import(s->s_name);
        if (result == -1) {
            object_error((t_object*)x, "error importing '%s'", s->s_name);
            goto error;
        }

        if (result == 0) {
            object_error((t_object*)x, "module '%s' not found", s->s_name);
            goto error;
        }

        py_GlobalRef mod = py_retval();
        py_setglobal(py_name(s->s_name), mod);

        pktpy2_bang_success(x);
        object_post((t_object*)x, "imported: %s", s->s_name);
    }
    return MAX_ERR_NONE;

error:
    pktpy2_bang_failure(x);
    return MAX_ERR_GENERIC;
}


/**
 * @brief Execute a max symbol as one to many lines of pktpy2 code
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return t_max_err error code
 */
t_max_err pktpy2_exec(t_pktpy2* x, t_symbol* s, long argc, t_atom* argv)
{
    const char* py_argv = NULL;

    py_argv = atom_getsym(argv)->s_name;
    if (py_argv == NULL) {
        goto error;
    }

    bool ok = py_exec(py_argv, "<string>", EXEC_MODE, NULL);
    if(!ok) goto error;

    pktpy2_bang_success(x);
    object_post((t_object*)x, "exec %s", py_argv);
    return MAX_ERR_NONE;

error:
    py_printexc();
    pktpy2_bang_failure(x);
    return MAX_ERR_GENERIC;
}


/**
 * @brief Evaluate a max symbol as a python expression
 *
 * @param x pointer to object structure
 * @param s symbol of object to be evaluated
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return t_max_err error code
 */
t_max_err pktpy2_eval(t_pktpy2* x, t_symbol* s, long argc, t_atom* argv)
{
    char* py_argv = atom_getsym(argv)->s_name;
    object_post((t_object*)x, "%s %s", s->s_name, py_argv);

    bool ok = py_eval(py_argv, NULL);

    if (ok) {
        py_GlobalRef retval = py_retval();
        pktpy2_handle_output(x, retval);
        pktpy2_bang_success(x);
        return MAX_ERR_NONE;
    } else {
        py_printexc();
        pktpy2_bang_failure(x);
        return MAX_ERR_GENERIC;
    }
}


/**
 * @brief Read text file into code-editor.
 *
 * @param x pointer to object structure
 * @param s path to text file
 */
void pktpy2_read(t_pktpy2* x, t_symbol* s)
{
    defer((t_object*)x, (method)pktpy2_doread, s, 0, NULL);
}

/**
 * @brief Read function callback
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 */
void pktpy2_doread(t_pktpy2* x, t_symbol* s, long argc, t_atom* argv)
{
    t_max_err err;
    t_filehandle fh;

    pktpy2_locate_path_from_symbol(x, s);
    err = path_opensysfile(x->p_code_filename, x->p_code_path, &fh, READ_PERM);
    if (!err) {
        sysfile_readtextfile(fh, x->p_code, 0,
                             TEXT_LB_UNIX | TEXT_NULL_TERMINATE);
        sysfile_close(fh);
        x->p_code_size = sysmem_handlesize(x->p_code);
    }
}


/**
 * @brief Run python code stored in editor buffer
 *
 * @param x pointer to object structure
 */
void pktpy2_run(t_pktpy2* x)
{
    if ((*(x->p_code) != NULL) && (*(x->p_code)[0] == '\0'))
        // is empty string
        goto error;

    bool ok = py_exec(*x->p_code, "<string>", EXEC_MODE, NULL);
    if(!ok) {
        goto error;
    }

    // success cleanup
    pktpy2_bang_success(x);
    return;

error:
    object_error((t_object*)x, "run x->p_code failed");
    pktpy2_bang_failure(x);
}


/**
 * @brief Combo function of `pktpy2_read <path> -> pktpy2_execfile <path>`
 *
 * @param x pointer to object structure
 * @param s path as symbol
 */
void pktpy2_load(t_pktpy2* x, t_symbol* s)
{
    pktpy2_read(x, s);
    pktpy2_execfile(x, s);
}


/**
 * @brief Execute contents of a file (filename obtained from symbol) as python
 * code
 *
 * @param x pointer to object structure
 * @param s symbol
 * @return t_max_err error code
 */
t_max_err pktpy2_execfile(t_pktpy2* x, t_symbol* s)
{
    t_max_err err;
    t_filehandle fh = NULL;
    t_handle p_code = sysmem_newhandle(0);
    int code_size = 0;

    if (s != gensym("")) {
        // set x->p_code_filepath
        err = pktpy2_locate_path_from_symbol(x, s);
        if (err != MAX_ERR_NONE) {
            object_error((t_object*)x, "could not locate path from symbol");
            goto error;
        }
    }

    if (s == gensym("") || x->p_code_filepath == gensym("")) {
        object_error((t_object*)x, "could not set filepath");
        goto error;
    }

    // assume x->p_code_filepath has be been set without errors

    err = path_opensysfile(x->p_code_filename, x->p_code_path, &fh, READ_PERM);
    if (err == MAX_ERR_NONE) {
        sysfile_readtextfile(fh, p_code, 0, TEXT_LB_UNIX | TEXT_NULL_TERMINATE);
        sysfile_close(fh);
        code_size = sysmem_handlesize(p_code);
    }

    bool ok = py_exec(*p_code, "<string>", EXEC_MODE, NULL);
    if(!ok) goto error;

    // success cleanup
    pktpy2_bang_success(x);
    return MAX_ERR_NONE;

error:
    py_printexc();
    pktpy2_bang_failure(x);
    return MAX_ERR_GENERIC;
}


/**
 * @brief Event of double-clicking on external object launches code-editor UI
 *
 * @param x pointer to object structure
 *
 */
void pktpy2_dblclick(t_pktpy2* x)
{
    if (x->p_code_editor)
        object_attr_setchar(x->p_code_editor, gensym("visible"), 1);
    else {
        x->p_code_editor = object_new(CLASS_NOBOX, gensym("jed"), x, 0);
        object_method(x->p_code_editor, gensym("settext"), *x->p_code, gensym("utf-8"));
        object_attr_setchar(x->p_code_editor, gensym("scratch"), 1);
        object_attr_setsym(x->p_code_editor, gensym("title"), gensym("py-editor"));
    }
}


/**
 * @brief Event function to preserve text in buffer after editor is closed
 *
 * @param x pointer to object structure
 * @param text text to be saved to buffer
 * @param size size of text to be saved to buffer
 */
void pktpy2_edclose(t_pktpy2* x, char** text, long size)
{
    if (x->p_code)
        sysmem_freehandle(x->p_code);

    x->p_code = sysmem_newhandleclear(size + 1);
    sysmem_copyptr((char*)*text, *x->p_code, size);
    x->p_code_size = size + 1;
    x->p_code_editor = NULL;
    if (x->p_run_on_close && x->p_code_size > 2) {
        pktpy2_run(x);
    }
}


/**
 * @brief Cnfigures behavior of system responding to editor window close
 *
 * @param x       pointer to object structure
 * @param s       custom save text (optional)
 * @param result  set values [0-4] to adjust what happens
 *                     how the system responds when the editor
 *                     window is closed.
 */
void pktpy2_okclose(t_pktpy2* x, char* s, short* result)
{
    // see: https://cycling74.com/forums/text-editor-without-dirty-bit
    object_post((t_object*)x, "okclose: called -- run-on-close: %d", x->p_run_on_close);
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
t_max_err pktpy2_edsave(t_pktpy2* x, char** text, long size)
{
    if (x->p_run_on_save) {

        object_post((t_object*)x, "run-on-save activated");

        bool ok = py_exec(*text, "<string>", EXEC_MODE, NULL);
        if(!ok) {
            goto error;
        }
    }
    return MAX_ERR_NONE;

error:
    object_error((t_object*)x, "py_edsave with execution failed");
    return MAX_ERR_GENERIC;
}




void pktpy2_float(t_pktpy2 *x, double f)
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
        error("pktpy2_float switch out-of-index");
    }
}


/*--------------------------------------------------------------------------*/
/* Side-effects */


/**
 * @brief Output bang from left outlet.
 *
 * @param x pointer to object struct.
 */
void pktpy2_bang(t_pktpy2* x) {
    t_max_err result = demo();

    outlet_bang(x->p_outlet_left);
}

/**
 * @brief Output bang from right outlet.
 *
 * @param x pointer to object struct.
 */
void pktpy2_bang_success(t_pktpy2* x) { outlet_bang(x->p_outlet_right); }

/**
 * @brief Output bang from middle outlet.
 *
 * @param x pointer to object struct.
 */
void pktpy2_bang_failure(t_pktpy2* x) { outlet_bang(x->p_outlet_middle); }



// ---- attr setting / getting



t_max_err pktpy2_name_get(t_pktpy2 *x, t_object *attr, long *argc, t_atom **argv)
{
    char alloc;

    if (argc && argv) {
        if (atom_alloc(argc, argv, &alloc)) {
                return MAX_ERR_OUT_OF_MEM;
            }
            if (alloc) {
                atom_setsym(*argv, x->name);
                post("pktpy2_name_get: %s", x->name->s_name);
        }
    }
    return MAX_ERR_NONE;
}

t_max_err pktpy2_name_set(t_pktpy2 *x, t_object *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        x->name = atom_getsym(argv);
        post("pktpy2_name_set: %s", x->name->s_name);
    }
    return MAX_ERR_NONE;
}


/**
 * @brief Searches the Max filesystem context for a file given by a symbol
 *
 * @param x pointer to object struct
 * @param s symbol to be searched
 * @return t_max_err
 *
 * If successful, this function will set `x->p_code_filepath` with
 * the Max readable path of the found file.
 */
t_max_err pktpy2_locate_path_from_symbol(t_pktpy2* x, t_symbol* s)
{
    t_max_err ret = MAX_ERR_NONE;

    if (s == gensym("")) {
        x->p_code_filename[0] = 0;

        if (open_dialog(x->p_code_filename, &x->p_code_path,
                        &x->p_code_outtype, &x->p_code_filetype, 1))
            /* non-zero: cancelled */
            ret = MAX_ERR_GENERIC;
        goto finally;

    } else {

        strncpy_zero(x->p_code_filename, s->s_name, MAX_PATH_CHARS);

        if (locatefile_extended(x->p_code_filename, &x->p_code_path,
                                &x->p_code_outtype, &x->p_code_filetype, 1)) {
            // nozero: not found
            object_error((t_object*)x, "can't find file %s", s->s_name);
            ret = MAX_ERR_GENERIC;
            goto finally;
        } else {
            x->p_code_pathname[0] = 0;
            ret = path_toabsolutesystempath(x->p_code_path, x->p_code_filename,
                                            x->p_code_pathname);
            if (ret != MAX_ERR_NONE) {
                object_error((t_object*)x, "can't convert %s to absolutepath", s->s_name);
                goto finally;
            }
        }

        // success: set attribute from pathname symbol
        x->p_code_filepath = gensym(x->p_code_pathname);
        assert(ret == MAX_ERR_NONE);
    }

finally:
    return ret;
}

