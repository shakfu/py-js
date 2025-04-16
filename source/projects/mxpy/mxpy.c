#include "ext.h"
#include "ext_obex.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>

enum { PY_MAX_ELEMS = 1024 };

typedef struct _mxpy {
    t_object x_ob;       ///< standard object header
    void* x_outlet;      ///< left outlet for msg output
    PyObject* py_object; ///< Python class object represented by this object
    int x_debug;         ///< Switch on and off debug logging.
} t_mxpy;

// forward declarations
static void mxpy_pyobject_to_atom(t_mxpy* x, PyObject* value, t_atom* atom);
static PyObject* mxpy_atom_to_pyobject(t_mxpy* x, t_atom* atom);
static PyObject* mxpy_atoms_to_pylist(t_mxpy* x, int argc, t_atom* argv);
static void mxpy_new_list_from_sequence(t_mxpy* x, PyObject* seq, int* argc, t_atom** argv);
static void mxpy_emit_outlet_message(t_mxpy* x, PyObject* value, void* x_outlet);

static void mxpy_eval(t_mxpy* x, t_symbol* s, int argc, t_atom* argv);
static void* mxpy_new(t_symbol* s, int argc, t_atom* argv);
static void mxpy_free(t_mxpy* x);

t_symbol* mxpy_locate_path_to_external(t_mxpy* x);
// end forward declaration

static t_class* mxpy_class;

#define py_error(x, ...) object_error((t_object*)(x), __VA_ARGS__);
#define py_warn(x, ...)  object_warn((t_object*)(x), __VA_ARGS__);
#define py_info(x, ...) object_post((t_object*)(x), __VA_ARGS__);
#define py_debug(x, ...) if ((x->x_debug)) object_post((t_object*)(x), __VA_ARGS__);

static PyObject* mxpy_atom_to_pyobject(t_mxpy* x, t_atom* atom)
{
    py_debug(x, "mxpy_atom_to_pyobject start");

    switch (atom->a_type) {

    case A_LONG:
        py_debug(x, "int: %i", atom_getlong(atom));
        return PyLong_FromLong(atom_getlong(atom));

    case A_FLOAT:
        py_debug(x, "float: %f", atom_getfloat(atom));
        return PyFloat_FromDouble(atom_getfloat(atom));

    case A_SYM:
        py_debug(x, "symbol: %s", atom_getsym(atom)->s_name);
        return PyUnicode_FromString(atom_getsym(atom)->s_name);

    case A_NOTHING:
        Py_RETURN_NONE;

    default:
        py_warn(x, "Warning: type %d unsupported for conversion to Python.",
             atom->a_type);
        Py_RETURN_NONE;
    }
}

static PyObject* mxpy_atoms_to_pylist(t_mxpy* x, int argc, t_atom* argv)
{
    py_debug(x, "mxpy_atoms_to_pylist start");

    PyObject* list = PyTuple_New(argc);
    for (int i = 0; i < argc; i++) {
        PyObject* value = mxpy_atom_to_pyobject(x, &argv[i]);
        PyTuple_SetItem(list, i, value); // pass value ref to the tuple
    }
    return list;
}

static void mxpy_pyobject_to_atom(t_mxpy* x, PyObject* value, t_atom* atom)
{
    py_debug(x, "mxpy_pyobject_to_atom start");

    if (value == Py_True) {
        atom_setlong(atom, 1);
    } else if (value == Py_False) {
        atom_setlong(atom, 0);
    } else if (PyFloat_Check(value)) {
        atom_setfloat(atom, (float)PyFloat_AsDouble(value));
    } else if (PyLong_Check(value)) {
        atom_setlong(atom, (float)PyLong_AsLong(value));
    } else if (PyUnicode_Check(value)) {
        atom_setsym(atom, gensym(PyUnicode_AsUTF8(value)));
    } else {
        atom_setsym(atom, gensym("error"));
    }
}

static void mxpy_new_list_from_sequence(t_mxpy* x, PyObject* seq, int* argc, t_atom** argv)
{
    py_debug(x, "mxpy_new_list_from_sequence start");

    Py_ssize_t len = 0;
    Py_ssize_t i;

    if (PyList_Check(seq)) {
        len = PyList_Size(seq);
        *argv = (t_atom*)sysmem_newptr(len * sizeof(t_atom));
        for (i = 0; i < len; i++) {
            PyObject* elem = PyList_GetItem(seq, i);
            mxpy_pyobject_to_atom(x, elem, (*argv) + i);
        }
    }
    *argc = (int)len;
}

static void mxpy_emit_outlet_message(t_mxpy* x, PyObject* value, void* x_outlet)
{
    py_debug(x, "mxpy_emit_outlet_message start");

    if (value == Py_True) {
        outlet_float(x_outlet, 1.0);
    } else if (value == Py_False) {
        outlet_float(x_outlet, 0.0);
    }

    else if (PyFloat_Check(value)) {
        outlet_float(x_outlet, (float)PyFloat_AsDouble(value));
    } else if (PyLong_Check(value)) {
        outlet_float(x_outlet, (float)PyLong_AsLong(value));
    } else if (PyUnicode_Check(value)) {
        outlet_anything(x_outlet, gensym(PyUnicode_AsUTF8(value)), 0, NIL);
    }

    else if (PyList_Check(value)) {
        t_atom* argv = NULL;
        int argc = 0;
        mxpy_new_list_from_sequence(x, value, &argc, &argv);

        if (argc > 0) {
            if (argv[0].a_type == A_SYM) {
                outlet_anything(x_outlet, atom_getsym(&argv[0]), argc - 1,
                                argv + 1);
            } else {
                // outlet_list(x_outlet, &s_list, argc, argv);
                outlet_list(x_outlet, NULL, argc, argv);
            }
        }
        if (argv) {
            sysmem_freeptr(argv);
        }
    }
}


/**
 * @brief      Return path to external with optional subpath
 *
 * @param      c        t_class instance
 * @param      subpath  The subpath or NULL (if not)
 *
 * @return     path to external + (optional subpath)
 */
static t_string* mxpy_get_path_to_external(t_class* c, char* subpath)
{
    char external_path[MAX_PATH_CHARS];
    char external_name[MAX_PATH_CHARS];
    char conform_path[MAX_PATH_CHARS];
    short path_id = class_getpath(c);
    t_string* result;

#ifdef __APPLE__
    const char* ext_filename = "%s.mxo";
#else
    const char* ext_filename = "%s.mxe64";
#endif
    snprintf_zero(external_name, MAX_FILENAME_CHARS, ext_filename,
                  c->c_sym->s_name);
    path_toabsolutesystempath(path_id, external_name, external_path);
    path_nameconform(external_path, conform_path, PATH_STYLE_NATIVE,
        PATH_TYPE_TILDE);
    result = string_new(external_path);
    if (subpath != NULL) {
        string_append(result, subpath);
    }
    return result;
}


/**
 * @brief      Return path to package with optional subpath
 *
 * @param      c        t_class instance
 * @param      subpath  The subpath or NULL (if not)
 *
 * @return     path to package + (optional subpath)
 */
static t_string* mxpy_get_path_to_package(t_class* c, char* subpath)
{
    char _dummy[MAX_PATH_CHARS];
    char externals_folder[MAX_PATH_CHARS];
    char package_folder[MAX_PATH_CHARS];

    t_string* result;
    t_string* external_path = mxpy_get_path_to_external(c, NULL);

    const char* ext_path_c = string_getptr(external_path);

    path_splitnames(ext_path_c, externals_folder, _dummy); // ignore filename
    path_splitnames(externals_folder, package_folder, _dummy); // ignore filename

    // post("externals_folder: %s", externals_folder);
    // post("package_folder: %s", package_folder);

    result = string_new((char*)package_folder);

    if (subpath != NULL) {
        string_append(result, subpath);
    }

    return result;
}


static void mxpy_bang(t_mxpy* x)
{
    py_debug(x, "mxpy_bang start");
    mxpy_eval(x, gensym("mx_bang"), 0, NULL);
}

static void mxpy_sym(t_mxpy* x, t_symbol* s)
{
    py_debug(x, "mxpy_sym start");
    t_atom atoms[1];
    atom_setsym(atoms, s);
    mxpy_eval(x, gensym("mx_symbol"), 1, atoms);
}

static void mxpy_int(t_mxpy* x, long n)
{
    py_debug(x, "mxpy_int start");
    t_atom atoms[1];
    atom_setlong(atoms, n);
    mxpy_eval(x, gensym("mx_int"), 1, atoms);
}

static void mxpy_float(t_mxpy* x, double n)
{
    py_debug(x, "mxpy_float start");
    t_atom atoms[1];
    atom_setfloat(atoms, n);
    mxpy_eval(x, gensym("mx_float"), 1, atoms);
}

static void mxpy_eval(t_mxpy* x, t_symbol* s, int argc, t_atom* argv)
{
    py_debug(x, "mxpy_eval start");
    py_debug(x, "s: %s argc: %i", s->s_name, argc);

    PyObject* func = NULL;
    PyObject* args = NULL;
    PyObject* value = NULL;

    if (x->py_object == NULL) {
        py_warn(x, "Warning: message sent to uninitialized python object.");
        return;
    }

    func = PyObject_GetAttrString(x->py_object, s->s_name);
    args = mxpy_atoms_to_pylist(x, argc, argv);

    if (!func) {
        py_warn(x, "Warning: no Python function found for s %s.", s->s_name);
    } else {
        if (!PyCallable_Check(func)) {
            py_warn(x, "Warning: Python attribute for s %s is not callable.",
                 s->s_name);
        } else {
            value = PyObject_CallObject(func, args);
        }
        Py_DECREF(func);
    }

    if (args) {
        Py_DECREF(args);
    }

    if (value == NULL) {
        py_warn(x, "Warning: Python call for '%s' failed.", s->s_name);

    } else {
        if (PyTuple_Check(value)) {
            // A tuple generates a sequence of outlet messages, one per item.
            int len = (int)PyTuple_Size(value);
            for (int i = 0; i < len; i++) {
                PyObject* elem = PyTuple_GetItem(value, i);
                mxpy_emit_outlet_message(x, elem, x->x_outlet);
            }
        } else {
            mxpy_emit_outlet_message(x, value, x->x_outlet);
        }

        Py_DECREF(value);
    }
}

static void* mxpy_new(t_symbol* s, int argc, t_atom* argv)
{
    t_mxpy* x = (t_mxpy*)object_alloc(mxpy_class);

    if (x) {
        x->py_object = NULL;
        x->x_debug = 1;

        // create an outlet on which to return values
        x->x_outlet = outlet_new(x, NULL);


        attr_args_process(x, argc, argv);

        if (argc < 2) {
            py_error(x, "python module and function args required.");
            goto except;
        }

        // get package root and then add path mxpy dir to pythonpath
        // all this to import py-js/source/projects/mxpy/python_help.py
        t_string* modpath = mxpy_get_path_to_package(mxpy_class,
                                                     "/source/projects/mxpy");
        const char* modpath_cstr = string_getptr(modpath);

        PyObject* module_path = NULL;
        PyObject* sys_path = NULL;
        PyObject* os_name = NULL;
        PyObject* os_module = NULL;
        PyObject* module_name = NULL;
        PyObject* module = NULL;
        PyObject* func = NULL;
        PyObject* args = NULL;

        module_path = PyUnicode_FromString(modpath_cstr);
        if (module_path == NULL) {
            py_error(x, "could not get python module_path");
            goto except;
        }
        
        sys_path = PySys_GetObject((char*)"path"); // borrowed reference
        if (sys_path == NULL) {
            py_error(x, "could not get python sys.path");
            goto except;
        }

        if (!PySequence_Contains(sys_path, module_path)) {
            py_info(x, "Appending %s to Python load path", modpath_cstr);
            PyList_Append(sys_path, module_path);
        }
        Py_DECREF(module_path);

        // try loading a system module
        os_name = PyUnicode_FromString("os");
        os_module = PyImport_Import(os_name);
        Py_DECREF(os_name);

        if (os_module == NULL) {
            py_error(x, "unable to import os module");
            goto except;
        }
        py_info(x, "os module imported");
        Py_DECREF(os_module);

        // try loading the module
        module_name = mxpy_atom_to_pyobject(x, &argv[0]);
        module = PyImport_Import(module_name);
        Py_DECREF(module_name);

        if (module == NULL) {
            py_error(x, "unable to import Python module %s.",
                     atom_getsym(argv + 0)->s_name);
            goto except;
        }

        func = PyObject_GetAttrString(
            module, atom_getsym(argv + 1)->s_name);

        if (func == NULL) {
             py_error(x, "Python function %s not found.",
                 atom_getsym(argv + 1)->s_name);
        }

        py_debug(x, "%s module imported", atom_getsym(argv + 0)->s_name);
        if (!PyCallable_Check(func)) {
             py_error(x, "Python attribute %s is not callable.",
                 atom_getsym(argv + 1)->s_name);
             goto except;
        }

        args = mxpy_atoms_to_pylist(x, argc - 2, argv + 2);
        if (args == NULL) {
            py_error(x, "could not convert atom list to python list");
            goto except;
        }

        x->py_object = PyObject_CallObject(func, args);

        assert(!PyErr_Occurred());
        assert(x);
        goto finally;

    except:
        assert(PyErr_Occurred());
        error("failed to initialize mxpy object");
        x = NULL;

    finally:
        Py_XDECREF(args);
        Py_XDECREF(func);
        Py_XDECREF(module);
        return x;
    }
}

static void mxpy_free(t_mxpy* x)
{
    py_debug(x, "python freeing object");

    if (x) {
        object_free(x->x_outlet); // ????
        // outlet_free(x->x_outlet);
        if (x->py_object) {
            Py_DECREF(x->py_object);
        }
        Py_Finalize();
        x->x_outlet = NULL;
        x->py_object = NULL;
    }
}

void ext_main(void* r)
{
    t_class* c;

    c = class_new("mxpy",               // t_symbol *name
                  (method)mxpy_new,     // t_newmethod newmethod
                  (method)mxpy_free,    // t_method freemethod
                  (long)sizeof(t_mxpy), // size_t size
                  0L,                   // int flags
                  A_GIMME, 0);          // t_atomtype arg1, ...

    class_addmethod(c, (method)mxpy_eval,   "anything", A_GIMME, 0);
    class_addmethod(c, (method)mxpy_bang,   "bang",              0);
    class_addmethod(c, (method)mxpy_sym,    "symbol",   A_SYM,   0);
    class_addmethod(c, (method)mxpy_int,    "int",      A_LONG,  0);
    class_addmethod(c, (method)mxpy_float,  "float",    A_FLOAT, 0);

    class_register(CLASS_BOX, c);

    mxpy_class = c;

    Py_Initialize();
}
