/*
This is an ongoing attempt to translate pdpython to maxmsp
from https://github.com/garthz/pdpython

/// pdpython.c : Pd external to bridge data in and out of Python
/// Copyright (c) 2014, Garth Zeglin.  All rights reserved.  Provided under the
/// terms of the BSD 3-clause license.

## TODO

- make ints, floats, and basic symbols work
- make mxpy_eval to handle  bang (see py_send), ints, floats, list, etc..
- nonsense: the restriction is that typed methods will fail,
unless we A_CANT or A_GIMMEBACK?

See a simple A_GIMMEBACK example simplejs.c example in the max-sdk

see:
- https://cycling74.com/forums/a_gimmeback-routine-appears-in-quickref-menu
- https://cycling74.com/forums/obex-how-to-get-return-values-from-object_method



## Flow

1. ext_main
   sets up one 'anything' method:
    class_addmethod(c, (method)mxpy_eval, "anything", A_GIMME, 0)

2. mxpy_new
     [mxpy module Class arg1 arg2 arg3 .. argN]
   PyObject* args = t_atom_list_to_PyObject_list(argc- 2, argv + 2)
   x->py_object = PyObject_CallObject(func, args)
   note: if Class is actually a function which return a non-callable value
   then x->py_object contains could be return with a bang

3. mxpy_eval
    responds to max message and dispatches to the python-Class

4. emit_outlet_message
    output returned values to outlet
    tuples are output in sequence

*/
#include "ext.h"
#include "ext_obex.h"

#include <libgen.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#define PY_MAX_ELEMS 1024

typedef struct _mxpy {
    t_object x_ob;       ///< standard object header
    void* x_outlet;      ///< left outlet for msg output
    PyObject* py_object; ///< Python class object represented by this object
} t_mxpy;

// forward declarations
static PyObject* t_atom_to_PyObject(t_atom* atom);
static PyObject* t_atom_list_to_PyObject_list(int argc, t_atom* argv);
static void PyObject_to_atom(PyObject* value, t_atom* atom);
static void new_list_from_sequence(PyObject* seq, int* argc, t_atom** argv);
static void emit_outlet_message(PyObject* value, void* x_outlet);

static void mxpy_eval(t_mxpy* x, t_symbol* selector, int argc, t_atom* argv);
static void* mxpy_new(t_symbol* selector, int argc, t_atom* argv);
static void mxpy_free(t_mxpy* x);
void ext_main(void* moduleRef);

t_symbol* mxpy_locate_path_to_external(t_mxpy* x);
// end forward declaration

static t_class* mxpy_class;


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

t_symbol* mxpy_locate_path_to_external(t_mxpy* x)
{
    char external_path[MAX_PATH_CHARS];
    char external_name[MAX_PATH_CHARS];
    char conform_path[MAX_PATH_CHARS];

    short path_id = class_getpath(mxpy_class);
    snprintf_zero(external_name, PY_MAX_ELEMS, "%s.mxo", mxpy_class->c_sym->s_name);
    path_toabsolutesystempath(path_id, external_name, external_path);
    /* path_nameconform(external_path, conform_path, PATH_STYLE_NATIVE, PATH_TYPE_PATH); */
    path_nameconform(external_path, conform_path, PATH_STYLE_NATIVE, PATH_TYPE_TILDE);
    /* post("path_id: %d, external_name: %s, external_path: %s conform_path: %s",
             path_id, external_name, external_path, conform_path);
    */
    return gensym(external_path);
}


static void mxpy_bang(t_mxpy* x)
{
    post("mxpy_bang start");
    mxpy_eval(x, gensym("mx_bang"), 0, NULL);
}

static void mxpy_sym(t_mxpy* x, t_symbol* s)
{
    post("mxpy_sym start");
    t_atom atoms[1];
    atom_setsym(atoms, s);
    mxpy_eval(x, gensym("mx_symbol"), 1, atoms);
}

void mxpy_int(t_mxpy* x, long n)
{
    post("mxpy_int start");
    t_atom atoms[1];
    atom_setlong(atoms, n);
    mxpy_eval(x, gensym("mx_int"), 1, atoms);
}

void mxpy_float(t_mxpy* x, double n)
{
    post("mxpy_float start");
    t_atom atoms[1];
    atom_setfloat(atoms, n);
    mxpy_eval(x, gensym("mx_float"), 1, atoms);
}

static void mxpy_eval(t_mxpy* x, t_symbol* selector, int argc, t_atom* argv)
{
    post("mxpy_eval start");
    post("selector: %s argc: %i", selector->s_name, argc);

    PyObject* func = NULL;
    PyObject* args = NULL;
    PyObject* value = NULL;

    if (x->py_object == NULL) {
        post("Warning: message sent to uninitialized python object.");
        return;
    }

    func = PyObject_GetAttrString(x->py_object, selector->s_name);
    args = t_atom_list_to_PyObject_list(argc, argv);

    if (!func) {
        post("Warning: no Python function found for selector %s.",
             selector->s_name);
    } else {
        if (!PyCallable_Check(func)) {
            post("Warning: Python attribute for selector %s is not callable.",
                 selector->s_name);
        } else {
            value = PyObject_CallObject(func, args);
        }
        Py_DECREF(func);
    }

    if (args)
        Py_DECREF(args);

    if (value == NULL) {
        post("Warning: Python call for selector %s failed.", selector->s_name);

    } else {
        if (PyTuple_Check(value)) {
            // A tuple generates a sequence of outlet messages, one per item.
            int i, len = (int)PyTuple_Size(value);
            for (i = 0; i < len; i++) {
                PyObject* elem = PyTuple_GetItem(value, i);
                emit_outlet_message(elem, x->x_outlet);
            }
        } else {
            emit_outlet_message(value, x->x_outlet);
        }

        Py_DECREF(value);
    }
}

static void* mxpy_new(t_symbol* selector, int argc, t_atom* argv)
{
    post("mxpy_new start");
    
    t_mxpy* x = (t_mxpy*)object_alloc(mxpy_class);
    
    if (x) {
        x->py_object = NULL;
    
        // create an outlet on which to return values
        x->x_outlet = outlet_new(x, NULL);
    }

    attr_args_process(x, argc, argv);


    if (argc < 2) {
        post("Error: python objects require a module and function specified "
             "in the creation arguments.");

    } else {

        // hackish way to get project root and then add path mxpy dir to pythonpath
        // all this to import py-js/source/projects/mxpy/python_help.py
        // TODO convert this to a function.
        t_symbol* path_to_ext_sym = mxpy_locate_path_to_external(x);
        post("path to external: %s", path_to_ext_sym->s_name);
        post("path to root: %s", dirname(dirname(path_to_ext_sym->s_name)));
        t_string* root_path = string_new(dirname(dirname(path_to_ext_sym->s_name)));
        string_append(root_path, "/source/projects/mxpy");
        const char* modpath_cstr = string_getptr(root_path);
        PyObject* modulePath = PyUnicode_FromString(modpath_cstr);
        PyObject* sysPath = PySys_GetObject((char*)"path"); // borrowed reference
        if (!PySequence_Contains(sysPath, modulePath)) {
            post("Appending %s to Python load path", modpath_cstr);
            PyList_Append(sysPath, modulePath);
        }
        Py_DECREF(modulePath);
        
        // try loading a system module
        PyObject* os_name = PyUnicode_FromString("os");
        PyObject* os_module = PyImport_Import(os_name);
        Py_DECREF(os_name);
        if (os_module == NULL) {
            post("ERROR: unable to import os module");
        } else {
            post("os module imported");
            Py_DECREF(os_module);
        }

        // try loading the module
        PyObject* module_name = t_atom_to_PyObject(&argv[0]);
        PyObject* module = PyImport_Import(module_name);
        Py_DECREF(module_name);

        if (module == NULL) {
            post("Error: unable to import Python module %s.",
                 atom_getsym(argv + 0)->s_name);

        } else {
            PyObject* func = PyObject_GetAttrString(
                module, atom_getsym(argv + 1)->s_name);

            if (func == NULL) {
                post("Error: Python function %s not found.",
                     atom_getsym(argv + 1)->s_name);

            } else {
                post("%s module imported", atom_getsym(argv + 0)->s_name);
                if (!PyCallable_Check(func)) {
                    post("Error: Python attribute %s is not callable.",
                         atom_getsym(argv + 1)->s_name);

                } else {
                    PyObject* args = t_atom_list_to_PyObject_list(argc - 2,
                                                                  argv + 2);
                    x->py_object = PyObject_CallObject(func, args);
                    Py_DECREF(args);
                }
                Py_DECREF(func);
            }
            Py_DECREF(module);
        }
    }

    return (x);
}

static void mxpy_free(t_mxpy* x)
{
    post("python freeing object");
    
    if (x) {
        object_free(x->x_outlet); // ????
        //outlet_free(x->x_outlet);
        if (x->py_object)
            Py_DECREF(x->py_object);
        x->x_outlet = NULL;
        x->py_object = NULL;
    }
}

void ext_main(void* moduleRef)
{
    post("ext_main start");

    t_class* c;

    c = class_new("mxpy",               // t_symbol *name
                  (method)mxpy_new,     // t_newmethod newmethod
                  (method)mxpy_free,    // t_method freemethod
                  (long)sizeof(t_mxpy), // size_t size
                  0L,                   // int flags
                  A_GIMME, 0);          // t_atomtype arg1, ...

    class_addmethod(c, (method)mxpy_eval, "anything", A_GIMME, 0);
    class_addmethod(c, (method)mxpy_bang, "bang", 0);
    class_addmethod(c, (method)mxpy_sym, "symbol", A_SYM, 0);
    class_addmethod(c, (method)mxpy_int, "int", A_LONG, 0);
    class_addmethod(c, (method)mxpy_float, "float", A_FLOAT, 0);

    class_register(CLASS_BOX, c);

    mxpy_class = c;

    // wchar_t* program;
    // program = Py_DecodeLocale("mxpy", NULL);
    // if (program == NULL) {
    //     exit(1);
    // }

    // Py_SetProgramName(program);

    Py_Initialize();

    // static wchar_t* arg0 = NULL;
    // PySys_SetArgv(0, &arg0);

    post("completed: ext_main");
}
