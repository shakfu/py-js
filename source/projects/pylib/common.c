/*

General functions for conversion between Max atoms and PyObjects

These have been extracted from more specialized code as a step
towards possible refactoring.

*/

#include "ext.h"
#include "ext_obex.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#define PY_MAX_ATOMS 128
#define PY_MAX_LOG_CHAR 500
#define PY_MAX_ERR_CHAR PY_MAX_LOG_CHAR
#define PY_DEBUG 1

// ---------------------------------------------------------------------------------------
// Forward Declarations

static void py_log(char* fmt, ...);
static void py_error(char* fmt, ...);
static void py_handle_error(char* fmt, ...);

static PyObject* py_atom_to_py_object(t_atom* atom);
static PyObject* py_atom_list_to_py_list0(int argc, t_atom* argv);
static PyObject* py_atom_list_to_py_list(long argc, t_atom* argv, int start_from);

static void py_object_to_atom(PyObject* value, t_atom* atom);
static void py_list_to_atom_list(PyObject* seq, int* argc, t_atom** argv);

static t_max_err py_float_output(void* outlet, PyObject* pfloat);
static t_max_err py_long_output(void* outlet, PyObject* plong);
static t_max_err py_string_output(void* outlet, PyObject* pstring);
static t_max_err py_list_output(void* outlet, PyObject* plist);
static t_max_err py_output(void* outlet, PyObject* pval);

static void py_output_single(void* outlet, PyObject* pval);

// ---------------------------------------------------------------------------------------
// HELPERS

/**
 * @brief      Variadic logging function with verbosity switch
 *
 * @param      fmt        The string to be outputed
 * @param      ...        Other arguments
 */
static void py_log(char* fmt, ...)
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
static void py_error(char* fmt, ...)
{
    char msg[PY_MAX_ERR_CHAR];

    va_list va;
    va_start(va, fmt);
    vsprintf(msg, fmt, va);
    va_end(va);

    error("[py error]: %s", msg);
}


// ---------------------------------------------------------------------------------------
// TRANSLATORS

/**
 * @brief      Converts max atom to python object
 *
 * @param      atom  The atom
 *
 * @return     python object (int, float, string)
 */
static PyObject* py_atom_to_py_object(t_atom* atom)
{
    py_log("py_atom_to_py_object start");

    switch (atom->a_type) {

    case A_LONG:
        py_log("int: %i", atom_getlong(atom));
        return PyLong_FromLong(atom_getlong(atom));

    case A_FLOAT:
        py_log("float: %f", atom_getfloat(atom));
        return PyFloat_FromDouble(atom_getfloat(atom));

    case A_SYM:
        py_log("symbol: %s", atom_getsym(atom)->s_name);
        return PyUnicode_FromString(atom_getsym(atom)->s_name);

    case A_NOTHING:
        Py_RETURN_NONE;

    default:
        py_log("Warning: type %d unsupported for conversion to Python.",
             atom->a_type);
        Py_RETURN_NONE;
    }
}


/**
 * @brief      Converts an atom list to a Python list
 *
 * @param[in]  argc  The length of the atom list
 * @param      argv  The Max atom list
 *
 * @return     Python list
 */
static PyObject* py_atom_list_to_py_list0(int argc, t_atom* argv)
{
    py_log("py_atom_list_to_py_list0 start");

    PyObject* list = PyTuple_New(argc);
    int i;
    for (i = 0; i < argc; i++) {
        PyObject* value = py_atom_to_py_object(&argv[i]);
        PyTuple_SetItem(list, i, value); // pass value ref to the tuple
    }
    return list;
}


/**
 * @brief      Converts an atom list to a Python list with offset
 *
 * @param[in]  argc  The length of the atom list
 * @param      argv  The Max atom list
 * @param[in]  start_from  The offset value
 *
 * @return     Python list
 */
static PyObject* py_atom_list_to_py_list(long argc, t_atom* argv, int start_from)
{

    PyObject* plist = NULL; // python list

    if ((plist = PyList_New(0)) == NULL) {
        py_error("could not create an empty python list");
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
            py_log("cannot process unknown type");
            break;
        }
    }
    return plist;

error:
    py_error("atom to list conversion failed");
    return NULL;
}


/**
 * @brief      Converts a python object to a max atom 
 *
 * @param      value  Python value
 * @param[out] atom   Max atom
 */
static void py_object_to_atom(PyObject* value, t_atom* atom)
{
    if (value == Py_True)
        atom_setlong(atom, 1);
    else if (value == Py_False)
        atom_setlong(atom, 0);
    else if (PyFloat_Check(value))
        atom_setfloat(atom, (float)PyFloat_AsDouble(value));
    else if (PyLong_Check(value))
        atom_setlong(atom, (float)PyLong_AsLong(value));
    else if (PyUnicode_Check(value))
        atom_setsym(atom, gensym(PyUnicode_AsUTF8(value)));
    else
        atom_setsym(atom, gensym("error"));
}


/**
 * @brief      Populates in-place an empty atom list with the contents of a python list 
 *
 * @param      seq   The sequence
 * @param      argc  The count of arguments
 * @param[out] argv  The arguments array
 */
static void py_list_to_atom_list(PyObject* seq, int* argc, t_atom** argv)
{
    Py_ssize_t len = 0;
    Py_ssize_t i;

    if (PyList_Check(seq)) {
        len = PyList_Size(seq);
        *argv = (t_atom*)malloc(len * sizeof(t_atom));
        for (i = 0; i < len; i++) {
            PyObject* elem = PyList_GetItem(seq, i);
            py_object_to_atom(elem, (*argv) + i);
        }
    }
    *argc = (int)len;
}


/**
 * @brief      { function_description }
 *
 * @param      pval    Python value
 * @param      outlet  The outlet
 */
static void py_output_single(PyObject* pval, void* outlet)
{
    py_log("py_output_single start");

    if (pval == Py_True)
        outlet_int(outlet, 1);
    else if (pval == Py_False)
        outlet_int(outlet, 0);

    else if (PyFloat_Check(pval))
        outlet_float(outlet, (float)PyFloat_AsDouble(pval));
    else if (PyLong_Check(pval))
        outlet_float(outlet, (float)PyLong_AsLong(pval));
    else if (PyUnicode_Check(pval))
        outlet_anything(outlet, gensym(PyUnicode_AsUTF8(pval)), 0, NIL);

    else if (PyList_Check(pval)) {
        t_atom* argv = NULL;
        int argc = 0;
        py_list_to_atom_list(pval, &argc, &argv);

        if (argc > 0) {
            if (argv[0].a_type == A_SYM) {
                outlet_anything(outlet, atom_getsym(&argv[0]), argc - 1,
                                argv + 1);
            } else {
                // outlet_list(outlet, &s_list, argc, argv);
                outlet_list(outlet, NULL, argc, argv);
            }
        }
        if (argv)
            free(argv);
    }
}


// ---------------------------------------------------------------------------------------

/**
 * @brief Generic python error handler
 * 
 * @param fmt format string
 * @param ... other args
 */
static void py_handle_error(char* fmt, ...)
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
static t_max_err py_float_output(void* outlet, PyObject* pfloat)
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
static t_max_err py_long_output(void* outlet, PyObject* plong)
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
static t_max_err py_string_output(void* outlet, PyObject* pstring)
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
static t_max_err py_list_output(void* outlet, PyObject* plist)
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
static t_max_err py_output(void* outlet, PyObject* pval)
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

