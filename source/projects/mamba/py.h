#ifndef PY_H
#define PY_H

#include "ext.h"
#include "ext_obex.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>



// ---------------------------------------------------------------------------------------
// Forward Declarations

// typedef struct t_py t_py;

typedef struct _py
{
    t_symbol* c_name;           /*!< unique python object name */
    t_symbol* c_pythonpath;     /*!< path to python directory */
    t_bool c_debug;             /*!< bool to switch per-object debug state */
    PyObject* c_globals;        /*!< per object 'globals' python namespace */
} t_py;

void py_init(t_py* x);
void py_free(t_py *x);

// void py_log(char* fmt, ...);
// void py_error(char* fmt, ...);
// void py_handle_error(char* fmt, ...);

// t_max_err py_float_output(void* outlet, PyObject* pfloat);
// t_max_err py_long_output(void* outlet, PyObject* plong);
// t_max_err py_string_output(void* outlet, PyObject* pstring);
// t_max_err py_list_output(void* outlet, PyObject* plist);
// t_max_err py_output(void* outlet, PyObject* pval);

t_max_err py_import(t_py* x, t_symbol* s);
t_max_err py_eval(t_py* x, t_symbol* s, long argc, t_atom* argv, void* outlet);

#endif // PY_H
