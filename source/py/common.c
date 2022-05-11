/*
==> CURRENTLY UNUSED in any code

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

// ---------------------------------------------------------------------------------------
// Forward Declarations

static PyObject* t_atom_to_PyObject(t_atom* atom);
static PyObject* t_atom_list_to_PyObject_list(int argc, t_atom* argv);
static void PyObject_to_atom(PyObject* value, t_atom* atom);
static void new_list_from_sequence(PyObject* seq, int* argc, t_atom** argv);
static void emit_outlet_message(PyObject* value, void* x_outlet);

static void py_handle_error(char* fmt, ...);
static t_max_err py_float_output(void* outlet, PyObject* pfloat);
static t_max_err py_long_output(void* outlet, PyObject* plong);
static t_max_err py_string_output(void* outlet, PyObject* pstring);
static t_max_err py_list_output(void* outlet, PyObject* plist);
static t_max_err py_output(void* outlet, PyObject* pval);
static PyObject* py_atoms_to_list(long argc, t_atom* argv, int start_from);

// ---------------------------------------------------------------------------------------

static PyObject* t_atom_to_PyObject(t_atom* atom)
{
    post("t_atom_to_PyObject start");

    switch (atom->a_type) {

    case A_LONG:
        post("int: %i", atom_getlong(atom));
        return PyLong_FromLong(atom_getlong(atom));

    case A_FLOAT:
        post("float: %f", atom_getfloat(atom));
        return PyFloat_FromDouble(atom_getfloat(atom));

    case A_SYM:
        post("symbol: %s", atom_getsym(atom)->s_name);
        return PyUnicode_FromString(atom_getsym(atom)->s_name);

    case A_NOTHING:
        Py_RETURN_NONE;

    default:
        post("Warning: type %d unsupported for conversion to Python.",
             atom->a_type);
        Py_RETURN_NONE;
    }
}

static PyObject* t_atom_list_to_PyObject_list(int argc, t_atom* argv)
{
    post("t_atom_list_to_PyObject_list start");

    PyObject* list = PyTuple_New(argc);
    int i;
    for (i = 0; i < argc; i++) {
        PyObject* value = t_atom_to_PyObject(&argv[i]);
        PyTuple_SetItem(list, i, value); // pass value ref to the tuple
    }
    return list;
}

static void PyObject_to_atom(PyObject* value, t_atom* atom)
{
    post("PyObject_to_atom start");

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

static void new_list_from_sequence(PyObject* seq, int* argc, t_atom** argv)
{
    post("new_list_from_sequence start");

    Py_ssize_t len = 0;
    Py_ssize_t i;

    if (PyList_Check(seq)) {
        len = PyList_Size(seq);
        *argv = (t_atom*)malloc(len * sizeof(t_atom));
        for (i = 0; i < len; i++) {
            PyObject* elem = PyList_GetItem(seq, i);
            PyObject_to_atom(elem, (*argv) + i);
        }
    }
    *argc = (int)len;
}

static void emit_outlet_message(PyObject* value, void* x_outlet)
{
    post("emit_outlet_message start");

    if (value == Py_True)
        outlet_float(x_outlet, 1.0);
    else if (value == Py_False)
        outlet_float(x_outlet, 0.0);

    else if (PyFloat_Check(value))
        outlet_float(x_outlet, (float)PyFloat_AsDouble(value));
    else if (PyLong_Check(value))
        outlet_float(x_outlet, (float)PyLong_AsLong(value));
    else if (PyUnicode_Check(value))
        outlet_anything(x_outlet, gensym(PyUnicode_AsUTF8(value)), 0, NIL);

    else if (PyList_Check(value)) {
        t_atom* argv = NULL;
        int argc = 0;
        new_list_from_sequence(value, &argc, &argv);

        if (argc > 0) {
            if (argv[0].a_type == A_SYM) {
                outlet_anything(x_outlet, atom_getsym(&argv[0]), argc - 1,
                                argv + 1);
            } else {
                // outlet_list(x_outlet, &s_list, argc, argv);
                outlet_list(x_outlet, NULL, argc, argv);
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

        error("[py] %s: %s", msg, pvalue_str);
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
 * @brief Outputs python long as max int
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
 * @brief Outputs python string as max symbol
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
 * @brief Outputs python list as max list
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
        post("seq_size: %d", seq_size);

        if (seq_size == 0) {
            error("cannot convert py list of length 0 to atoms");
            goto error;
        }

        if (seq_size > PY_MAX_ATOMS) {
            post("dynamically increasing size of atom array");
            atoms = atom_dynamic_start(atoms_static, PY_MAX_ATOMS,
                                       seq_size + 1);
            is_dynamic = 1;

        } else {
            atoms = atoms_static;
        }

        if ((iter = PyObject_GetIter(plist)) == NULL) {
            goto error;
        }
        post("seq_size2: %d", seq_size);

        while ((item = PyIter_Next(iter)) != NULL) {
            if (PyLong_Check(item)) {
                long long_item = PyLong_AsLong(item);
                if (long_item == -1) {
                    if (PyErr_Occurred())
                        goto error;
                }
                atom_setlong(atoms + i, long_item);
                post("%d long: %ld\n", i, long_item);
                i++;
            }

            if (PyFloat_Check(item)) {
                float float_item = PyFloat_AsDouble(item);
                if (float_item == -1.0) {
                    if (PyErr_Occurred())
                        goto error;
                }
                atom_setfloat(atoms + i, float_item);
                post("%d float: %f\n", i, float_item);
                i++;
            }

            if (PyUnicode_Check(item)) {
                const char* unicode_item = PyUnicode_AsUTF8(item);
                if (unicode_item == NULL) {
                    goto error;
                }
                atom_setsym(atoms + i, gensym(unicode_item));
                post("%d unicode: %s\n", i, unicode_item);
                i++;
            }
            Py_DECREF(item);
        }

        outlet_list(outlet, NULL, i, atoms);
        post("end iter op: %d", i);

        if (is_dynamic) {
            post("restoring to static atom array");
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
 * @brief Output python object as max object
 *
 * @param outlet object outlet
 * @param pval python object
 * @return t_max_err error code
 */
t_max_err py_output(void* outlet, PyObject* pval)
{
    if (pval == NULL) {
        error("cannot handle NULL value");
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
        error("cannot handle his type of value");
        return MAX_ERR_GENERIC;
    }
}


/*--------------------------------------------------------------------------*/
// TRANSLATORS

PyObject* py_atoms_to_list(long argc, t_atom* argv, int start_from)
{

    PyObject* plist = NULL; // python list

    if ((plist = PyList_New(0)) == NULL) {
        error("could not create an empty python list");
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
            post("cannot process unknown type");
            break;
        }
    }
    return plist;

error:
    error("atom to list conversion failed");
    return NULL;
}

