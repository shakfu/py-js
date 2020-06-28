#ifndef PY_H
#define PY_H

/* py.h */

/*--------------------------------------------------------------------------*/
// INCLUDES

/* max api */
#include "ext.h"
#include "ext_obex.h"

/* python */
#define PY_SSIZE_T_CLEAN
#include <Python.h>
/*--------------------------------------------------------------------------*/
// CONSTANTS

#define PY_MAX_ATOMS 128
#define PY_MAX_LOG_CHAR 500 // high number during development
#define PY_MAX_ERR_CHAR PY_MAX_LOG_CHAR

/*--------------------------------------------------------------------------*/
// GLOBALS

t_class* py_class;                    // global pointer to object class
static int py_global_obj_count;       // when 0 then free interpreter
static t_hashtab* py_global_registry; // global object lookups
/*--------------------------------------------------------------------------*/
// OBJECT TYPES

typedef struct _py {
    /* object header */
    t_object p_ob;

    /* object attributes */
    t_symbol* p_name; /* unique object name */

    /* python-related */
    t_symbol* p_pythonpath; /* path to python directory */
    t_bool p_debug;         /* bool to switch per-object debug state */
    PyObject* p_globals;    /* per object 'globals' python namespace */

    /* infra objects */
    t_patcher* p_patcher; /* to send msgs to objects */
    t_box* p_box;         /* the ui box of the py instance? */

    /* text editor attrs */
    t_object* p_code_editor;
    char** p_code;
    long p_code_size;

    t_fourcc p_code_filetype; // = FOUR_CHAR_CODE('TEXT')
    t_fourcc p_code_outtype;  // = FOUR_CHAR_CODE('TEXT')
    char p_code_filename[MAX_PATH_CHARS];
    char p_code_pathname[MAX_PATH_CHARS];
    short p_code_path;

    t_symbol* p_code_filepath; /* default python filepath to load into
                                  the code editor and object 'globals'
                                  namespace */
    t_bool p_autoload;         /* bool to autoload of p_code_filepath  */

    /* outlet creation */
    void* p_outlet_right;  // right outlet to bang success
    void* p_outlet_middle; // middle outleet to bang error
    void* p_outlet_left;   // left outleet for msg output

} t_py;

/*--------------------------------------------------------------------------*/
// FUNCTION TYPES
/*--------------------------------------------------------------------------*/
// ENUMS
/*--------------------------------------------------------------------------*/
// MACROS
/*--------------------------------------------------------------------------*/
// METHODS

/* object creation and destruction */
void* py_new(t_symbol* s, long argc, t_atom* argv);
void py_free(t_py* x);
void py_init(t_py* x);

/* helpers */
void py_log(t_py* x, char* fmt, ...);
void py_error(t_py* x, char* fmt, ...);
void py_init_builtins(t_py* x);
void py_locate_path_from_symbol(t_py* x, t_symbol* s);
t_hashtab* get_global_registry(void);

/* common handlers */
void py_handle_error(t_py* x, char* fmt, ...);
void py_handle_float_output(t_py* x, PyObject* pval);
void py_handle_long_output(t_py* x, PyObject* pval);
void py_handle_string_output(t_py* x, PyObject* pval);
void py_handle_list_output(t_py* x, PyObject* pval);
void py_handle_dict_output(t_py* x, PyObject* pval);
void py_handle_output(t_py* x, PyObject* pval);

/* core python methods */
void py_import(t_py* x, t_symbol* s);
void py_eval(t_py* x, t_symbol* s, long argc, t_atom* argv);
void py_exec(t_py* x, t_symbol* s, long argc, t_atom* argv);
void py_execfile(t_py* x, t_symbol* s);

/* extra python methods */
void py_assign(t_py* x, t_symbol* s, long argc, t_atom* argv);
void py_call(t_py* x, t_symbol* s, long argc, t_atom* argv);
void py_code(t_py* x, t_symbol* s, long argc, t_atom* argv);
void py_pipe(t_py* x, t_symbol* s, long argc, t_atom* argv);
void py_anything(t_py* x, t_symbol* s, long argc, t_atom* argv);

/* informational */
void py_count(t_py* x);
void py_assist(t_py* x, void* b, long m, long a, char* s);
void py_appendtodict(t_py* x, t_dictionary* dict);

/* testing */
void py_bang(t_py* x);

/* interobject communications */
void py_send(t_py* x, t_symbol* s, long argc, t_atom* argv);
void py_scan(t_py* x);
long py_scan_callback(t_py* x, t_object* obj);

/* code editor */
void py_read(t_py* x, t_symbol* s);
void py_load(t_py* x, t_symbol* s); // read(f) -> execfile(f)
void py_doread(t_py* x, t_symbol* s, long argc, t_atom* argv);
void py_dblclick(t_py* x);
void py_edclose(t_py* x, char** text, long size);
long py_edsave(t_py* x, char** text, long size);


#endif // PY_H
