/** \file py.h
    \brief A single-header pyyhon3 library for Max externals.

    This library is supposed to enable nested py objects in a given
    Max external. This library is in the public domain.

    The latest code can be found in https : // github.com/shakfu/py-js

    If PY_IMPLEMENTATION isdefined before including the header,
    it will activate the implementation, otherwise the implementation
    will not be included.

    Usage example:

        #define PY_IMPLEMENTATION // <-- activate the implementation
        #include "py.h"

        typedef struct myobj {
            t_object obj;
            t_py* py; // <-- this is the key opaque type and instance
            void* outlet;
        } t_myobj;

*/
// ---------------------------------------------------------------------------------------
// HEADER

#ifndef PY_H
#define PY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ext.h"
#include "ext_obex.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>


// data structure declaration
typedef struct t_py t_py;

// init / free methods
t_py * py_init(void);
void py_free(t_py *x);

// core methods
t_max_err py_import(t_py* x, t_symbol* s);
t_max_err py_eval(t_py* x, t_symbol* s, long argc, t_atom* argv, void* outlet);
t_max_err py_exec(t_py* x, t_symbol* s, long argc, t_atom* argv);
t_max_err py_execfile(t_py* x, t_symbol* s);

// extra methods
t_max_err py_call(t_py* x, t_symbol* s, long argc, t_atom* argv, void* outlet);
t_max_err py_assign(t_py* x, t_symbol* s, long argc, t_atom* argv);
t_max_err py_code(t_py* x, t_symbol* s, long argc, t_atom* argv, void* outlet);
t_max_err py_anything(t_py* x, t_symbol* s, long argc, t_atom* argv, void* outlet);
t_max_err py_pipe(t_py* x, t_symbol* s, long argc, t_atom* argv, void* outlet);


#ifdef __cplusplus
}
#endif
#endif /* PY_H */

// ---------------------------------------------------------------------------------------
// END HEADER


// ---------------------------------------------------------------------------------------
// IMPLEMENTATION

//#ifdef PY_IMPLEMENTATION

/*
    py.h -- single-header library providing minimal python3 services for Max externals.
*/

#define PY_MAX_ATOMS 128
#define PY_MAX_LOG_CHAR 500
#define PY_MAX_ERR_CHAR PY_MAX_LOG_CHAR
#define PY_DEBUG 1

// ---------------------------------------------------------------------------------------
// Datastructure

struct t_py
{
    t_symbol* p_name;                       /*!< unique python object name */
    t_symbol* p_pythonpath;                 /*!< path to python directory */
    t_bool p_debug;                         /*!< bool to switch per-object debug state */
    t_fourcc p_code_filetype;               /*!< filetype four char code of 'TEXT' */
    t_fourcc p_code_outtype;                /*!< filetype four char code of 'TEXT' */
    char p_code_filename[MAX_PATH_CHARS];   /*!< file name field */
    char p_code_pathname[MAX_PATH_CHARS];   /*!< file path field */
    short p_code_path;                      /*!< short code for max file system */
    t_symbol* p_code_filepath;              /*!< filepath to python file to execfile */
    PyObject* p_globals;                    /*!< per object 'globals' python namespace */
};


// ---------------------------------------------------------------------------------------
// Forward Declarations not in header

void py_log(t_py* x, char* fmt, ...);
void py_error(t_py* x, char* fmt, ...);
void py_handle_error(t_py* x, char* fmt, ...);

t_max_err py_locate_path_from_symbol(t_py* x, t_symbol* s);

t_max_err py_handle_float_output(t_py* x, void* outlet, PyObject* pval);
t_max_err py_handle_long_output(t_py* x, void* outlet, PyObject* pval);
t_max_err py_handle_string_output(t_py* x, void* outlet, PyObject* pval);
t_max_err py_handle_list_output(t_py* x, void* outlet, PyObject* pval);
t_max_err py_handle_dict_output(t_py* x, void* outlet, PyObject* pval);
t_max_err py_handle_output(t_py* x, void* outlet, PyObject* pval);

PyObject* py_atoms_to_list(t_py* x, long argc, t_atom* argv, int start_from);

t_max_err py_eval_text(t_py* x, long argc, t_atom* argv, int offset, void* outlet);


// ---------------------------------------------------------------------------------------
// HELPERS

/**
 * @brief Post msg to Max console.
 *
 * @param x pointer to object struct
 * @param fmt character string with format codes
 * @param ... other arguments
 *
 * This log function is a variadic function which does not `post` its message
 * if the object struct member `x->p_debug` is 0.
 *
 * WARNING: if PY_MAX_LOG_CHAR (which defines PY_MAX_ERR_CHAR) is less than
 * the length of the log or err message, Max will crash.
 */
void py_log(t_py* x, char* fmt, ...)
{
    if (x->p_debug) {
        char msg[PY_MAX_LOG_CHAR];

        va_list va;
        va_start(va, fmt);
        vsprintf(msg, fmt, va);
        va_end(va);

        post("[py %s]: %s", x->p_name->s_name, msg);
    }
}

/**
 * @brief Post error message to Max console.
 *
 * @param x pointer to object struct
 * @param fmt character string with format codes
 * @param ... other arguments
 */
void py_error(t_py* x, char* fmt, ...)
{
    char msg[PY_MAX_ERR_CHAR];

    va_list va;
    va_start(va, fmt);
    vsprintf(msg, fmt, va);
    va_end(va);

    error("[py %s]: %s", x->p_name->s_name, msg);
}

// ---------------------------------------------------------------------------------------
// OBJECT INIT / FREE

/**
 * @brief Initializes and allocate t_py object instance
 *
 * @return t_py*  pointer to object structure
 */
t_py * py_init(void)
{
    // t_py *x = calloc(1, sizeof *x);
    // t_py *x = calloc(1, sizeof (struct t_py));
    t_py *x = malloc(sizeof (struct t_py));

    x->p_name = symbol_unique();
    x->p_pythonpath = gensym("");
    x->p_debug = PY_DEBUG;

    x->p_code_filetype = FOUR_CHAR_CODE('TEXT');
    x->p_code_outtype = 0;
    x->p_code_filename[0] = 0;
    x->p_code_pathname[0] = 0;
    // x->p_code_path = 0;
    x->p_code_filepath = gensym("");

    Py_Initialize();

    // python init
    PyObject* main_mod = PyImport_AddModule(x->p_name->s_name); // borrowed
    x->p_globals = PyModule_GetDict(main_mod); // borrowed reference

    PyObject* p_name = NULL;
    PyObject* builtins = NULL;

    p_name = PyUnicode_FromString(x->p_name->s_name);
    builtins = PyEval_GetBuiltins();
    PyDict_SetItemString(builtins, "PY_OBJ_NAME", p_name);
    PyDict_SetItemString(x->p_globals, "__builtins__", builtins);
    Py_XDECREF(p_name);

    return x;
}

/**
 * @brief Free t_py object
 *
 * @param x  pointer to object structure
 */
void py_free(t_py* x)
{
    py_log(x, "deleting object %s", x->p_name->s_name);
    Py_XDECREF(x->p_globals);
    Py_FinalizeEx();
    free(x);    
}


// ---------------------------------------------------------------------------------------
// HELPERS




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
t_max_err py_locate_path_from_symbol(t_py* x, t_symbol* s)
{
    t_max_err ret = 0;

    if (s == gensym("")) { // if no arg supplied ask for file
        x->p_code_filename[0] = 0;

        if (open_dialog(x->p_code_filename, &x->p_code_path,
                        &x->p_code_outtype, &x->p_code_filetype, 1))
            // non-zero: cancelled
            ret = MAX_ERR_GENERIC;
            goto finally;

    } else {
        // must copy symbol before calling locatefile_extended
        strncpy_zero(x->p_code_filename, s->s_name, MAX_PATH_CHARS);
        if (locatefile_extended(x->p_code_filename, &x->p_code_path,
                                &x->p_code_outtype, &x->p_code_filetype, 1)) {
            // nozero: not found
            py_error(x, "can't find file %s", s->s_name);
            ret = MAX_ERR_GENERIC;
            goto finally;
        } else {
            x->p_code_pathname[0] = 0;
            ret = path_toabsolutesystempath(x->p_code_path, x->p_code_filename,
                                            x->p_code_pathname);
            if (ret != MAX_ERR_NONE) {
                py_error(x, "can't convert %s to absolutepath", s->s_name);
                goto finally;
            }
        }

        // success
        // set attribute from pathname symbol
        x->p_code_filepath = gensym(x->p_code_pathname);
        assert(ret == MAX_ERR_NONE);
    }

finally:
    return ret;
}


// ---------------------------------------------------------------------------------------
// TYPE HANDLERS

/**
 * @brief Generic python error handler
 * 
 * @param x pointer to object struct
 * @param fmt format string
 * @param ... other args
 */
void py_handle_error(t_py* x, char* fmt, ...)
{
    if (PyErr_Occurred()) {

        // build custom msg
        char msg[PY_MAX_ERR_CHAR];

        va_list va;
        va_start(va, fmt);
        vsprintf(msg, fmt, va);
        va_end(va);

        // get error info
        PyObject *ptype, *pvalue, *ptraceback;
        PyErr_Fetch(&ptype, &pvalue, &ptraceback);
        PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);
        Py_XDECREF(ptype);

        PyObject* pvalue_pstr = PyObject_Repr(pvalue);
        const char* pvalue_str = PyUnicode_AsUTF8(pvalue_pstr);
        Py_XDECREF(pvalue);
        Py_XDECREF(pvalue_pstr);

        Py_XDECREF(ptraceback);

        error("[py %s] %s: %s", x->p_name->s_name, msg, pvalue_str);
    }
}


/**
 * @brief Handler to output python float as max float
 * 
 * @param x pointer to object struct
 * @param pfloat python float
 * @return t_max_err error code
 */
t_max_err py_handle_float_output(t_py* x, void* outlet, PyObject* pfloat)
{
    if (pfloat == NULL) {
        goto error;
    }

    if (PyFloat_Check(pfloat)) {
        float float_result = (float)PyFloat_AsDouble(pfloat);
        if (float_result == -1.0) {
            if (PyErr_Occurred())
                goto error;
        }

        outlet_float(outlet, float_result);
    }
    Py_XDECREF(pfloat);
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "py_handle_float_output failed");
    Py_XDECREF(pfloat);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Handler to output python long as max int
 *
 * @param x pointer to object struct
 * @param plong python long
 * @return t_max_err error code
 */
t_max_err py_handle_long_output(t_py* x, void* outlet, PyObject* plong)
{
    if (plong == NULL) {
        goto error;
    }

    if (PyLong_Check(plong)) {
        long long_result = PyLong_AsLong(plong);
        if (long_result == -1) {
            if (PyErr_Occurred())
                goto error;
        }
        outlet_int(outlet, long_result);
    }

    Py_XDECREF(plong);
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "py_handle_long_output failed");
    Py_XDECREF(plong);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Handler to output python string as max symbol
 *
 * @param x pointer to object struct
 * @param pstring python string
 * @return t_max_err error code
 */
t_max_err py_handle_string_output(t_py* x, void* outlet, PyObject* pstring)
{
    if (pstring == NULL) {
        goto error;
    }

    if (PyUnicode_Check(pstring)) {
        const char* unicode_result = PyUnicode_AsUTF8(pstring);
        if (unicode_result == NULL) {
            goto error;
        }
        outlet_anything(outlet, gensym(unicode_result), 0, NIL);
    }

    Py_XDECREF(pstring);
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "py_handle_string_output failed");
    Py_XDECREF(pstring);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Handler to output python list as max list
 *
 * @param x pointer to object struct
 * @param plist python list
 * @return t_max_err error code
 */
t_max_err py_handle_list_output(t_py* x, void* outlet, PyObject* plist)
{
    if (plist == NULL) {
        goto error;
    }

    if (PySequence_Check(plist) && !PyUnicode_Check(plist)
        && !PyBytes_Check(plist) && !PyByteArray_Check(plist)) {
        PyObject* iter = NULL;
        PyObject* item = NULL;
        int i = 0;

        t_atom atoms_static[PY_MAX_ATOMS];
        t_atom* atoms = NULL;
        int is_dynamic = 0;

        Py_ssize_t seq_size = PySequence_Length(plist);
        py_log(x, "seq_size: %d", seq_size);

        if (seq_size == 0) {
            py_error(x, "cannot convert py list of length 0 to atoms");
            goto error;
        }

        if (seq_size > PY_MAX_ATOMS) {
            py_log(x, "dynamically increasing size of atom array");
            atoms = atom_dynamic_start(atoms_static, PY_MAX_ATOMS,
                                       seq_size + 1);
            is_dynamic = 1;

        } else {
            atoms = atoms_static;
        }

        if ((iter = PyObject_GetIter(plist)) == NULL) {
            goto error;
        }
        py_log(x, "seq_size2: %d", seq_size);

        while ((item = PyIter_Next(iter)) != NULL) {
            if (PyLong_Check(item)) {
                long long_item = PyLong_AsLong(item);
                if (long_item == -1) {
                    if (PyErr_Occurred())
                        goto error;
                }
                atom_setlong(atoms + i, long_item);
                py_log(x, "%d long: %ld\n", i, long_item);
                i++;
            }

            if (PyFloat_Check(item)) {
                float float_item = PyFloat_AsDouble(item);
                if (float_item == -1.0) {
                    if (PyErr_Occurred())
                        goto error;
                }
                atom_setfloat(atoms + i, float_item);
                py_log(x, "%d float: %f\n", i, float_item);
                i++;
            }

            if (PyUnicode_Check(item)) {
                const char* unicode_item = PyUnicode_AsUTF8(item);
                if (unicode_item == NULL) {
                    goto error;
                }
                atom_setsym(atoms + i, gensym(unicode_item));
                py_log(x, "%d unicode: %s\n", i, unicode_item);
                i++;
            }
            Py_DECREF(item);
        }

        outlet_list(outlet, NULL, i, atoms);
        py_log(x, "end iter op: %d", i);

        if (is_dynamic) {
            py_log(x, "restoring to static atom array");
            atom_dynamic_end(atoms_static, atoms);
        }
    }

    Py_XDECREF(plist);
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "py_handle_list_output failed");
    Py_XDECREF(plist);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Handler to output python dict as max list
 *
 * @param x pointer to object struct
 * @param pdict python dict
 * @return t_max_err error code
 */
t_max_err py_handle_dict_output(t_py* x, void* outlet, PyObject* pdict)
{
    PyObject* pfun_co = NULL;
    PyObject* pfun = NULL;
    PyObject* pval = NULL;

    if (pdict == NULL) {
        goto error;
    }

    if (PyDict_Check(pdict)) {

        pfun_co = PyRun_String("def __py_maxmsp_out_dict(arg):\n"
                               "\tres = []\n"
                               "\tfor k,v in arg.items():\n"
                               "\t\tres.append(k)\n"
                               "\t\tres.append(':')\n"
                               "\t\tif type(v) in [list, set, tuple]:\n"
                               "\t\t\tfor i in v:\n"
                               "\t\t\t\tres.append(i)\n"
                               "\t\telse:\n"
                               "\t\t\tres.append(v)\n"
                               "\treturn res\n",
                               Py_single_input, x->p_globals, x->p_globals);

        if (pfun_co == NULL) {
            py_error(x, "out_dict function code object is NULL");
            goto error;
        }

        pfun = PyDict_GetItemString(x->p_globals, "__py_maxmsp_out_dict");
        if (pfun == NULL) {
            py_error(x, "retrieving out_dict func from globals failed");
            goto error;
        }

        pval = PyObject_CallFunctionObjArgs(pfun, pdict, NULL);
        if (pval == NULL) {
            py_error(x, "out_dict call failed to retrieve result");
            goto error;
        }

        if (PyList_Check(pval)) {           // expecting a python list
            py_handle_list_output(x, outlet, pval); // this decrefs pval
            Py_XDECREF(pfun_co);
            return MAX_ERR_NONE;
        } else {
            py_error(x, "expected list output got something else");
            goto error;
        }
    }

error:
    py_handle_error(x, "py_handle_dict_output failed");
    Py_XDECREF(pfun_co);
    Py_XDECREF(pval);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Generic handler to output arbitrarily-typed python object as max object
 *
 * @param x pointer to object struct
 * @param pval python object
 * @return t_max_err error code
 */
t_max_err py_handle_output(t_py* x, void* outlet, PyObject* pval)
{
    if (pval == NULL) {
        py_error(x, "cannot handle NULL value");
        return MAX_ERR_GENERIC;
    }

    if (PyFloat_Check(pval)) {
        return py_handle_float_output(x, outlet, pval);
    }

    else if (PyLong_Check(pval)) {
        return py_handle_long_output(x, outlet, pval);
    }

    else if (PyUnicode_Check(pval)) {
        return py_handle_string_output(x, outlet, pval);
    }

    else if (PySequence_Check(pval) && !PyBytes_Check(pval)
             && !PyByteArray_Check(pval)) {
        return py_handle_list_output(x, outlet, pval);
    }

    else if (PyDict_Check(pval)) {
        return py_handle_dict_output(x, outlet, pval);
    }

    else if (pval == Py_None) {
        return MAX_ERR_GENERIC;
    }

    else {
        py_error(x, "cannot handle his type of value");
        return MAX_ERR_GENERIC;
    }
}


// ---------------------------------------------------------------------------------------
// CORE METHODS


/**
 * @brief Import a python module
 * 
 * @param x pointer to object structure
 * @param s symbol of module to be imported
 * @return t_max_err error code
 */
t_max_err py_import(t_py* x, t_symbol* s)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    PyObject* x_module = NULL;

    if (s != gensym("")) {
        x_module = PyImport_ImportModule(s->s_name);
        // x_module borrrowed ref
        if (x_module == NULL) {
            goto error;
        }
        PyDict_SetItemString(x->p_globals, s->s_name, x_module);
        PyGILState_Release(gstate);
        py_log(x, "imported: %s", s->s_name);
    }
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "import %s", s->s_name);
    PyGILState_Release(gstate);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Evaluate a max symbol as a python expression
 * 
 * @param x pointer to object structure
 * @param s symbol of object to be evaluated
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param outlet object outlet
 * 
 * @return t_max_err error code
 */
t_max_err py_eval(t_py* x, t_symbol* s, long argc, t_atom* argv, void* outlet)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    char* py_argv = atom_getsym(argv)->s_name;
    py_log(x, "%s %s", s->s_name, py_argv);

    PyObject* pval = PyRun_String(py_argv, Py_eval_input, x->p_globals,
                                  x->p_globals);

    if (pval != NULL) {
        py_handle_output(x, outlet, pval);
        PyGILState_Release(gstate);
        return MAX_ERR_NONE;
    } else {
        py_handle_error(x, "eval %s", py_argv);
        PyGILState_Release(gstate);
        return MAX_ERR_GENERIC;
    }
}

/**
 * @brief Execute a max symbol as a line of python code
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return t_max_err error code
 */
t_max_err py_exec(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    char* py_argv = NULL;
    PyObject* pval = NULL;

    py_argv = atom_getsym(argv)->s_name;
    if (py_argv == NULL) {
        goto error;
    }

    pval = PyRun_String(py_argv, Py_single_input, x->p_globals, x->p_globals);
    if (pval == NULL) {
        goto error;
    }
    Py_DECREF(pval);
    PyGILState_Release(gstate);

    py_log(x, "exec %s", py_argv);
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "exec %s", py_argv);
    Py_XDECREF(pval);
    PyGILState_Release(gstate);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Execute contents of a file (obtained from symbol) as python code
 *
 * @param x pointer to object structure
 * @param s symbol
 * @return t_max_err error code
 */
t_max_err py_execfile(t_py* x, t_symbol* s)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    PyObject* pval = NULL;
    FILE* fhandle = NULL;

    if (s != gensym("")) {
        // set x->p_code_filepath
        t_max_err err = py_locate_path_from_symbol(x, s);
        if (err != MAX_ERR_NONE) {
            py_error(x, "could not locate path from symbol");
            goto error;
        }
    }

    if (s == gensym("") || x->p_code_filepath == gensym("")) {
        py_error(x, "could not set filepath");
        goto error;
    }

    // assume x->p_code_filepath has be been set without errors

    py_log(x, "pathname: %s", x->p_code_filepath->s_name);
    fhandle = fopen(x->p_code_filepath->s_name, "r+");

    if (fhandle == NULL) {
        py_error(x, "could not open file");
        goto error;
    }

    pval = PyRun_File(fhandle, x->p_code_filepath->s_name, Py_file_input,
                      x->p_globals, x->p_globals);
    if (pval == NULL) {
        fclose(fhandle);
        goto error;
    }

    // success cleanup
    fclose(fhandle);
    Py_DECREF(pval);
    PyGILState_Release(gstate);
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "execfile");
    Py_XDECREF(pval);
    PyGILState_Release(gstate);
    return MAX_ERR_GENERIC;
}

// ---------------------------------------------------------------------------------------
// EXTRA METHODS HELPERS

/**
 * @brief Translates atom vector to python list
 * 
 * @param x pointer to object struct
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param start_from index of vector to start from
 * @return PyObject* python list
 */
PyObject* py_atoms_to_list(t_py* x, long argc, t_atom* argv, int start_from)
{

    PyObject* plist = NULL; // python list

    if ((plist = PyList_New(0)) == NULL) {
        py_error(x, "could not create an empty python list");
        goto error;
    }

    for (int i = start_from; i < argc; i++) {
        switch ((argv + i)->a_type) {
        case A_FLOAT: {
            double c_float = atom_getfloat(argv + i);
            PyObject* p_float = PyFloat_FromDouble(c_float);
            if (p_float == NULL) {
                goto error;
            }
            PyList_Append(plist, p_float);
            Py_DECREF(p_float);
            break;
        }
        case A_LONG: {
            PyObject* p_long = PyLong_FromLong(atom_getlong(argv + i));
            if (p_long == NULL) {
                goto error;
            }
            PyList_Append(plist, p_long);
            Py_DECREF(p_long);
            break;
        }
        case A_SYM: {
            PyObject* p_str = PyUnicode_FromString(
                atom_getsym(argv + i)->s_name);
            if (p_str == NULL) {
                goto error;
            }
            PyList_Append(plist, p_str);
            Py_DECREF(p_str);
            break;
        }
        default:
            py_log(x, "cannot process unknown type");
            break;
        }
    }
    return plist;

error:
    py_error(x, "atom to list conversion failed");
    return NULL;
}


/**
 * @brief A helper function to evaluate Max text as a Python expression.
 *
 * @param x pointer to object structure
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param offset offset of atom vector from which to evaluate
 * @param outlet object outlet
 * 
 * @return t_max_err error code
 */
t_max_err py_eval_text(t_py* x, long argc, t_atom* argv, int offset, void* outlet)
{
    PyGILState_STATE gstate = PyGILState_Ensure();

    long textsize = 0;
    char* text = NULL;
    int is_eval = 1;

    t_max_err err = atom_gettext(argc + offset, argv, &textsize, &text,
                                 OBEX_UTIL_ATOM_GETTEXT_DEFAULT);
    if (err == MAX_ERR_NONE && textsize && text) {
        py_log(x, ">>> %s", text);
    } else {
        goto error;
    }

    PyObject* co = Py_CompileString(text, x->p_name->s_name, Py_eval_input);

    if (PyErr_ExceptionMatches(PyExc_SyntaxError)) {
        PyErr_Clear();
        co = Py_CompileString(text, x->p_name->s_name, Py_single_input);
        is_eval = 0;
    }

    if (co == NULL) { // can be eval-co or exec-co or NULL here
        goto error;
    }
    sysmem_freeptr(text);

    PyObject* pval = PyEval_EvalCode(co, x->p_globals, x->p_globals);
    if (pval == NULL) {
        goto error;
    }
    Py_DECREF(co);

    if (!is_eval) {
        // bang for exec-type op
        PyGILState_Release(gstate);
    } else {
        py_handle_output(x, outlet, pval);
    }
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "python code evaluation failed");
    PyGILState_Release(gstate);
    return MAX_ERR_GENERIC;
}


// ---------------------------------------------------------------------------------------
// EXTRA METHODS

/**
 * @brief Converts a Max list to call a python function with arguments
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param outlet object outlet
 * 
 * @return t_max_err error code
 */
t_max_err py_call(t_py* x, t_symbol* s, long argc, t_atom* argv, void* outlet)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    char* callable_name = NULL;
    PyObject* py_argslist = NULL;
    PyObject* pval = NULL;
    PyObject* py_callable = NULL;
    // python list
    PyObject* py_args = NULL; // python tuple

    // first atom in argv must be a symbol
    if (argv->a_type != A_SYM) {
        py_error(x, "first atom must be a symbol!");
        goto error;

    } else {
        callable_name = atom_getsym(argv)->s_name;
        py_log(x, "callable_name: %s", callable_name);
    }

    py_callable = PyRun_String(callable_name, Py_eval_input, x->p_globals,
                               x->p_globals);
    if (py_callable == NULL) {
        py_error(x, "could not evaluate %s", callable_name);
        goto error;
    }

    py_argslist = py_atoms_to_list(x, argc, argv, 1);
    if (py_argslist == NULL) {
        py_error(x, "atom to py list conversion failed");
        goto error;
    }

    py_log(x, "length of argc:%ld list: %d", argc, PyList_Size(py_argslist));

    // convert py_args to tuple
    py_args = PyList_AsTuple(py_argslist);
    if (py_args == NULL) {
        py_error(x, "unable to convert args list to tuple");
        goto error;
    }

    pval = PyObject_CallObject(py_callable, py_args);
    if (!PyErr_ExceptionMatches(PyExc_TypeError)) {
        if (pval == NULL) {
            py_error(x, "unable to apply callable(*args)");
            goto error;
        }
        goto handle_output;
    }
    PyErr_Clear();

    pval = PyObject_CallFunctionObjArgs(py_callable, py_argslist, NULL);
    if (pval == NULL) {
        py_error(x, "could not retrieve result of callable(list)");
        goto error;
    }
    goto handle_output; // this is redundant but safer in case code is added

handle_output:
    py_handle_output(x, outlet, pval);
    // success cleanup
    Py_XDECREF(py_callable);
    Py_XDECREF(py_argslist);
    PyGILState_Release(gstate);
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "method %s", s->s_name);
    // cleanup
    Py_XDECREF(py_callable);
    Py_XDECREF(py_argslist);
    Py_XDECREF(pval);
    PyGILState_Release(gstate);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Converts an atom list to a python assignment
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return t_max_err error code
 * 
 * The first item of the Max list must be a symbol. This is converted into a python variable
 * and the rest of the list is assignment to this variable in the object's python
 * namespace.
 */
t_max_err py_assign(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    char* varname = NULL;
    PyObject* list = NULL;

    if (s != gensym(""))
        py_log(x, "s: %s", s->s_name);

    // first atom in argv must be a symbol
    if (argv->a_type != A_SYM) {
        py_error(x, "first atom must be a symbol!");
        goto error;

    } else {
        varname = atom_getsym(argv)->s_name;
        py_log(x, "varname: %s", varname);
    }

    list = py_atoms_to_list(x, argc, argv, 1);
    if (list == NULL) {
        py_error(x, "atom to py list conversion failed");
        goto error;
    }

    if (PyList_Size(list) != argc - 1) {
        py_error(x, "PyList_Size(list) != argc - 1");
        goto error;
    } else {
        py_log(x, "length of list: %d", PyList_Size(list));
    }

    // finally, assign list to varname in object namespace
    py_log(x, "setting %s to list in namespace", varname);
    // following does not steal ref to list
    int res = PyDict_SetItemString(x->p_globals, varname, list);
    if (res != 0) {
        py_error(x, "assign varname to list failed");
        goto error;
    }
    PyGILState_Release(gstate);
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "assign %s", s->s_name);
    Py_XDECREF(list);
    PyGILState_Release(gstate);
    return MAX_ERR_GENERIC;
}


/**
 * @brief Converts all of the atom to text and evaluate as python code.
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param outlet object outlet
 * 
 * @return t_max_err error code
 */
t_max_err py_code(t_py* x, t_symbol* s, long argc, t_atom* argv, void* outlet)
{
    return py_eval_text(x, argc, argv, 0, outlet);
}


/**
 * @brief Anything method converting all of the atom to text and evaluate as python code.
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param outlet object outlet
 * 
 * @return t_max_err error code
 */
t_max_err py_anything(t_py* x, t_symbol* s, long argc, t_atom* argv, void* outlet)
{
    t_atom atoms[PY_MAX_ATOMS];

    if (s == gensym("")) {
        return MAX_ERR_GENERIC; 
    }

    // set '=' as shorthand for assign method
    if (s == gensym("=")) {
        py_assign(x, gensym(""), argc, argv);
        return MAX_ERR_NONE;
    }

    // set symbol as first atom in new atoms array
    atom_setsym(atoms, s);

    for (int i = 0; i < argc; i++) {
        switch ((argv + i)->a_type) {
        case A_FLOAT: {
            atom_setfloat((atoms + (i + 1)), atom_getfloat(argv + i));
            break;
        }
        case A_LONG: {
            atom_setlong((atoms + (i + 1)), atom_getlong(argv + i));
            break;
        }
        case A_SYM: {
            atom_setsym((atoms + (i + 1)), atom_getsym(argv + i));
            break;
        }
        default:
            py_log(x, "cannot process unknown type");
            break;
        }
    }

    return py_eval_text(x, argc, atoms, 1, outlet);
}

/**
 * @brief Create a function python pipeline from a Max list
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param outlet object outlet
 * 
 * @return t_max_err error code
 */
t_max_err py_pipe(t_py* x, t_symbol* s, long argc, t_atom* argv, void* outlet)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    long textsize = 0;
    char* text = NULL;
    t_max_err err;
    PyObject* pipe_pre = NULL;
    PyObject* pipe_fun = NULL;
    PyObject* pval = NULL;
    PyObject* pstr = NULL;

    err = atom_gettext(argc, argv, &textsize, &text,
                       OBEX_UTIL_ATOM_GETTEXT_DEFAULT);
    if (err != MAX_ERR_NONE || !textsize || !text) {
        py_error(x, "atom -> text conversion failed");
        goto error;
    }

    pipe_pre = PyRun_String(
        "def __py_maxmsp_pipe(arg):\n"
        "\targs = arg.split()\n"
        "\tval = eval(args[0], locals(), globals())\n"
        "\tfuncs = [eval(f, locals(), globals()) for f in args[1:]]\n"
        "\tfor f in funcs:\n"
        "\t\tval = f(val)\n"
        "\treturn val\n",
        Py_single_input, x->p_globals, x->p_globals);

    if (pipe_pre == NULL) {
        py_error(x, "pipe func is NULL");
        goto error;
    }

    pstr = PyUnicode_FromString(text);
    if (pstr == NULL) {
        py_error(x, "cstr -> pyunicode conversion failed");
        goto error;
    }

    sysmem_freeptr(text);

    pipe_fun = PyDict_GetItemString(x->p_globals, "__py_maxmsp_pipe");
    if (pipe_fun == NULL) {
        py_error(x, "retrieving pipe func from globals failed");
        goto error;
    }

    pval = PyObject_CallFunctionObjArgs(pipe_fun, pstr, NULL);

    if (pval != NULL) {

        if (!PyUnicode_Check(pval)) {
            py_handle_output(x, outlet, pval); // this decrefs pval
        } else {
            // special case strings, which will cause crash if handled
            // out of this methods's scope. (huge PITA to debug!)
            const char* unicode_result = PyUnicode_AsUTF8(pval);
            if (unicode_result == NULL) {
                goto error;
            }
            outlet_anything(outlet, gensym(unicode_result), 0, NIL);
            Py_XDECREF(pval);
        }

        Py_XDECREF(pipe_pre);
        Py_XDECREF(pstr);
        PyGILState_Release(gstate);
        return MAX_ERR_NONE;
    } else {
        goto error;
    }

error:
    py_handle_error(x, "pipe failed");
    Py_XDECREF(pipe_pre);
    Py_XDECREF(pstr);
    Py_XDECREF(pval);
    // fail bang
    PyGILState_Release(gstate);
    return MAX_ERR_GENERIC;
}

//#endif

// ---------------------------------------------------------------------------------------
// END IMPLEMENTATION
