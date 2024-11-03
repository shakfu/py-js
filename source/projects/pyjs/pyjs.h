/* pyjs.h */

#ifndef PYJS_H
#define PYJS_H



/**
 * @file pyjs.h
 * 
 * @brief Python scripting external for Max/MSP with JavaScript integration
 * 
 * This is the main header file for the `pyjs` external. It provides Python 
 * scripting capabilities within Max/MSP with additional JavaScript integration.
 * Key features include:
 * - Full Python interpreter integration with the Max/MSP js object api which 
 *  allows you to use python within js objects.
 * - Running Python code directly in Max via messages
 * - Loading Python scripts from files 
 * - Importing Python modules
 * - Converting Python objects to JSON for JavaScript interop
 * - Support for Python packages via PYTHONPATH
 * 
 * Note that the external structure is not directly exposed at the header level.
 * 
 */


/*--------------------------------------------------------------------------*/
/* Includes */

#include "ext.h"
#include "ext_obex.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>

/*--------------------------------------------------------------------------*/
/* Constants */

#define PY_MAX_ELEMS 1024

/*--------------------------------------------------------------------------*/
/* Macros */

#define _STR(x) #x
#define STR(x) _STR(x)
#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)
#define _PY_VER CONCAT(PY_MAJOR_VERSION, CONCAT(., PY_MINOR_VERSION))
#define PY_VER STR(_PY_VER)

/*--------------------------------------------------------------------------*/
/* Datastructures */

typedef struct t_pyjs t_pyjs;

/*--------------------------------------------------------------------------*/
/* Methods */

void* pyjs_new(t_symbol* s, long argc, t_atom* argv);
void pyjs_free(t_pyjs* x);
void pyjs_init(t_pyjs* x);
void pyjs_init_builtins(t_pyjs* x);
void pyjs_log(t_pyjs* x, char* fmt, ...);
void pyjs_error(t_pyjs* x, char* fmt, ...);
void pyjs_handle_error(t_pyjs* x, char* fmt, ...);
void pyjs_locate_path_from_symbol(t_pyjs* x, t_symbol* s);
t_string* pyjs_get_path_to_external(t_class* c, char* subpath);
t_string* pyjs_get_path_to_package(t_class* c, char* subpath);
t_max_err pyjs_import(t_pyjs* x, t_symbol* s);
t_max_err pyjs_exec(t_pyjs* x, t_symbol* s);
t_max_err pyjs_execfile(t_pyjs* x, t_symbol* s);
t_max_err pyjs_eval(t_pyjs* x, t_symbol* s, long argc, t_atom* argv, t_atom* rv);
t_max_err pyjs_eval_to_json(t_pyjs* x, t_symbol* s, long argc, t_atom* argv, t_atom* rv);
t_max_err pyjs_code(t_pyjs* x, t_symbol* s, long argc, t_atom* argv, t_atom* rv);
t_max_err pyjs_handle_output(t_pyjs* x, PyObject* pval, t_atom* rv);
t_max_err pyjs_handle_float_output(t_pyjs* x, PyObject* pfloat, t_atom* rv);
t_max_err pyjs_handle_long_output(t_pyjs* x, PyObject* plong, t_atom* rv);
t_max_err pyjs_handle_list_output(t_pyjs* x, PyObject* plist, t_atom* rv);
t_max_err pyjs_handle_dict_output(t_pyjs* x, PyObject* pdict, t_atom* rv);


#endif // PYJS_H
