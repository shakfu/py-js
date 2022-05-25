/*

General functions for conversion between Max atoms and PyObjects

These have been extracted from more specialized code as a step
towards possible refactoring.

*/

#include "py.h"

#define PY_MAX_ATOMS 128
#define PY_MAX_LOG_CHAR 500
#define PY_MAX_ERR_CHAR PY_MAX_LOG_CHAR
#define PY_DEBUG 1

// ---------------------------------------------------------------------------------------
// Datastructure

// struct t_py
// {
//     t_symbol* c_name;           /*!< unique python object name */
//     t_symbol* c_pythonpath;     /*!< path to python directory */
//     t_bool c_debug;             /*!< bool to switch per-object debug state */
//     PyObject* c_globals;        /*!< per object 'globals' python namespace */
// };

// ---------------------------------------------------------------------------------------
// Forward Declarations not in header

void py_log(char* fmt, ...);
void py_error(char* fmt, ...);
void py_handle_error(char* fmt, ...);

t_max_err py_float_output(void* outlet, PyObject* pfloat);
t_max_err py_long_output(void* outlet, PyObject* plong);
t_max_err py_string_output(void* outlet, PyObject* pstring);
t_max_err py_list_output(void* outlet, PyObject* plist);
t_max_err py_output(void* outlet, PyObject* pval);

// ---------------------------------------------------------------------------------------
// HELPERS

/**
 * @brief      Variadic logging function with verbosity switch
 *
 * @param      fmt        The string to be outputed
 * @param      ...        Other arguments
 */
void py_log(char* fmt, ...)
{
    if (PY_DEBUG) {
        char msg[PY_MAX_LOG_CHAR];

        va_list va;
        va_start(va, fmt);
        vsprintf(msg, fmt, va);
        va_end(va);

        post("[py]: %s", msg);
    }
}

/**
 * @brief      Variadic error function
 *
 * @param      fmt        The string to be outputed
 * @param      ...        Other arguments
 */
void py_error(char* fmt, ...)
{
    char msg[PY_MAX_ERR_CHAR];

    va_list va;
    va_start(va, fmt);
    vsprintf(msg, fmt, va);
    va_end(va);

    error("[py error]: %s", msg);
}

// ---------------------------------------------------------------------------------------
// OBJECT INIT / FREE

void py_init(t_py* x)
{
    x->c_name = symbol_unique();
    x->c_pythonpath = gensym("");

    Py_Initialize();

    // python init
    PyObject* main_mod = PyImport_AddModule(x->c_name->s_name); // borrowed
    x->c_globals = PyModule_GetDict(main_mod); // borrowed reference

    PyObject* p_name = NULL;
    PyObject* builtins = NULL;

    p_name = PyUnicode_FromString(x->c_name->s_name);
    builtins = PyEval_GetBuiltins();
    PyDict_SetItemString(builtins, "PY_OBJ_NAME", p_name);
    PyDict_SetItemString(x->c_globals, "__builtins__", builtins);
    Py_XDECREF(p_name);
}

void py_free(t_py* x)
{
    Py_XDECREF(x->c_globals);
    Py_FinalizeEx();
    free(x);    
}

// ---------------------------------------------------------------------------------------
// TYPE HANDLERS

/**
 * @brief Generic python error handler
 * 
 * @param fmt format string
 * @param ... other args
 */
void py_handle_error(char* fmt, ...)
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

        py_error("%s: %s", msg, pvalue_str);
    }
}


/**
 * @brief Outputs python float as max float
 * 
 * @param outlet object outlet
 * @param pfloat python float
 * @return t_max_err error code
 */
t_max_err py_float_output(void* outlet, PyObject* pfloat)
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
    py_handle_error("py_float_output failed");
    Py_XDECREF(pfloat);
    return MAX_ERR_GENERIC;
}


/**
 * @brief Outputs to outlet python long as max int
 *
 * @param outlet object outlet
 * @param plong python long
 * @return t_max_err error code
 */
t_max_err py_long_output(void* outlet, PyObject* plong)
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
    py_handle_error("py_long_output failed");
    Py_XDECREF(plong);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Outputs to outlet python string as max symbol
 *
 * @param outlet object outlet
 * @param pstring python string
 * @return t_max_err error code
 */
t_max_err py_string_output(void* outlet, PyObject* pstring)
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
    py_handle_error("py_string_output failed");
    Py_XDECREF(pstring);
    return MAX_ERR_GENERIC;
}


/**
 * @brief Outputs to outlet python list as max list
 *
 * @param outlet object outlet
 * @param plist python list
 * @return t_max_err error code
 */
t_max_err py_list_output(void* outlet, PyObject* plist)
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
        py_log("seq_size: %d", seq_size);

        if (seq_size == 0) {
            py_error("cannot convert py list of length 0 to atoms");
            goto error;
        }

        if (seq_size > PY_MAX_ATOMS) {
            py_log("dynamically increasing size of atom array");
            atoms = atom_dynamic_start(atoms_static, PY_MAX_ATOMS,
                                       seq_size + 1);
            is_dynamic = 1;

        } else {
            atoms = atoms_static;
        }

        if ((iter = PyObject_GetIter(plist)) == NULL) {
            goto error;
        }
        py_log("seq_size2: %d", seq_size);

        while ((item = PyIter_Next(iter)) != NULL) {
            if (PyLong_Check(item)) {
                long long_item = PyLong_AsLong(item);
                if (long_item == -1) {
                    if (PyErr_Occurred())
                        goto error;
                }
                atom_setlong(atoms + i, long_item);
                py_log("%d long: %ld\n", i, long_item);
                i++;
            }

            if (PyFloat_Check(item)) {
                float float_item = PyFloat_AsDouble(item);
                if (float_item == -1.0) {
                    if (PyErr_Occurred())
                        goto error;
                }
                atom_setfloat(atoms + i, float_item);
                py_log("%d float: %f\n", i, float_item);
                i++;
            }

            if (PyUnicode_Check(item)) {
                const char* unicode_item = PyUnicode_AsUTF8(item);
                if (unicode_item == NULL) {
                    goto error;
                }
                atom_setsym(atoms + i, gensym(unicode_item));
                py_log("%d unicode: %s\n", i, unicode_item);
                i++;
            }
            Py_DECREF(item);
        }

        outlet_list(outlet, NULL, i, atoms);
        py_log("end iter op: %d", i);

        if (is_dynamic) {
            py_log("restoring to static atom array");
            atom_dynamic_end(atoms_static, atoms);
        }
    }

    Py_XDECREF(plist);
    return MAX_ERR_NONE;

error:
    py_handle_error("py_handle_list_output failed");
    Py_XDECREF(plist);
    return MAX_ERR_GENERIC;
}


/**
 * @brief Output to outlet python object as max object
 *
 * @param outlet object outlet
 * @param pval python object
 * @return t_max_err error code
 */
t_max_err py_output(void* outlet, PyObject* pval)
{
    if (pval == NULL) {
        py_error("cannot handle NULL value");
        return MAX_ERR_GENERIC;
    }

    if (PyFloat_Check(pval)) {
        return py_float_output(outlet, pval);
    }

    else if (PyLong_Check(pval)) {
        return py_long_output(outlet, pval);
    }

    else if (PyUnicode_Check(pval)) {
        return py_string_output(outlet, pval);
    }

    else if (PySequence_Check(pval) && !PyBytes_Check(pval)
             && !PyByteArray_Check(pval)) {
        return py_list_output(outlet, pval);
    }

    else if (pval == Py_None) {
        return MAX_ERR_GENERIC;
    }

    else {
        py_error("cannot handle his type of value");
        return MAX_ERR_GENERIC;
    }
}


// ---------------------------------------------------------------------------------------
// CORE METHODS

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
        PyDict_SetItemString(x->c_globals, s->s_name, x_module);
        PyGILState_Release(gstate);
        post("imported: %s", s->s_name);
    }
    return MAX_ERR_NONE;

error:
    py_handle_error("import %s", s->s_name);
    PyGILState_Release(gstate);
    return MAX_ERR_GENERIC;
}


t_max_err py_eval(t_py* x, t_symbol* s, long argc, t_atom* argv, void* outlet)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    char* py_argv = atom_getsym(argv)->s_name;
    post("%s %s", s->s_name, py_argv);

    PyObject* pval = PyRun_String(py_argv, Py_eval_input, x->c_globals, x->c_globals);

    if (pval != NULL) {
        py_output(outlet, pval);
        PyGILState_Release(gstate);
        return MAX_ERR_NONE;
    } else {
        py_handle_error("eval error %s", py_argv);
        PyGILState_Release(gstate);
        return MAX_ERR_GENERIC;
    }
}


