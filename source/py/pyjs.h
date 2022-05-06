
/* pyjs.h */

#ifndef PYJS_H
#define PYJS_H
                                                                                                                                            

#include "ext.h"
#include "ext_obex.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#if defined(__APPLE__) && (defined(PY_STATIC_EXT) || defined(PY_SHARED_PKG))
#include <CoreFoundation/CoreFoundation.h>
#include <libgen.h>
#endif

#define PY_MAX_ATOMS 128
#define PY_MAX_LOG_CHAR 500 // high number during development
#define PY_MAX_ERR_CHAR PY_MAX_LOG_CHAR

#define _STR(x) #x
#define STR(x) _STR(x)
#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)
#define _PY_VER CONCAT(PY_MAJOR_VERSION, CONCAT(., PY_MINOR_VERSION))
#define PY_VER STR(_PY_VER)

typedef struct t_pyjs t_pyjs;

void* pyjs_new(t_symbol* s, long argc, t_atom* argv);
void pyjs_free(t_pyjs* x);
void pyjs_init(t_pyjs* x);
void pyjs_init_builtins(t_pyjs* x);
void pyjs_log(t_pyjs* x, char* fmt, ...);
void pyjs_error(t_pyjs* x, char* fmt, ...);
void pyjs_handle_error(t_pyjs* x, char* fmt, ...);
void pyjs_locate_path_from_symbol(t_pyjs* x, t_symbol* s);
t_max_err pyjs_import(t_pyjs* x, t_symbol* s);
t_max_err pyjs_exec(t_pyjs* x, t_symbol* s);
t_max_err pyjs_execfile(t_pyjs* x, t_symbol* s);
t_max_err pyjs_eval(t_pyjs* x, t_symbol* s, long argc, t_atom* argv,
                    t_atom* rv);
t_max_err pyjs_eval_to_json(t_pyjs* x, t_symbol* s, long argc, t_atom* argv,
                            t_atom* rv);
t_max_err pyjs_code(t_pyjs* x, t_symbol* s, long argc, t_atom* argv,
                    t_atom* rv);
t_max_err pyjs_handle_output(t_pyjs* x, PyObject* pval, t_atom* rv);
t_max_err pyjs_handle_float_output(t_pyjs* x, PyObject* pfloat, t_atom* rv);
t_max_err pyjs_handle_long_output(t_pyjs* x, PyObject* plong, t_atom* rv);
t_max_err pyjs_handle_list_output(t_pyjs* x, PyObject* plist, t_atom* rv);
t_max_err pyjs_handle_dict_output(t_pyjs* x, PyObject* pdict, t_atom* rv);


#endif // PYJS_H
