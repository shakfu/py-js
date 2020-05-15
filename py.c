/**
    py.c - basic experiment in minimal max object for calling python code

    repo - github.com/shakfu/py

    This object has 1 inlet and 2 outlets

    Basic Features

    1.  Per-Object Namespace. It responds to an 'import <module>' message in 
        the left inlet which loads a python module in its namespace. Each new import
        (like python) adds to the namespace.

    2.  Eval Messages. It responds to an 'eval <expression>' message in the left inlet
        which is evaluated in the namespace and outputs results to the left outlet
        and outputs a bang from the right outlet to signal end of evaluation.

    py interpreter object
        attributes
            @imports
            @code

        messages
            import <module> [adds to @imports]
            eval <code> or eval @file <path>
            exec <code> or exec @file <path>
            run  <code> or run  @file <path>

            (phase 2)
            load @file <path> -> into @code (for persistence) and texeditor edits

            (phase N)
            embed ipython kernel? (-;

    TODO

        - [ ] add right inlet bang after eval op ends
        - [ ] add @run <script>
        - [ ] add text edit object

*/

/* max/msp api */
#include "ext.h"
#include "ext_obex.h" // this is required for all objects using the newer style for writing objects.

/* python */
#define PY_SSIZE_T_CLEAN
#include <Python.h>

/* other */
#include "api.h"
// #ifndef _GNU_SOURCE
// #define _GNU_SOURCE
// #endif
// #include <string.h>

#define PY_MAX_ATOMS 128
#define PY_NOT_STRING(x) (!PyBytes_Check(x) && !PyByteArray_Check(x) && !PyUnicode_Check(x))

typedef struct _py {
    t_object p_ob;          // object header - ALL objects MUST begin with this...
    t_symbol *p_module;     // python import: additional imports
    t_symbol *p_code;       // python code to evaluate to default outlet
    void *p_outlet;         // outlet creation - inlets are automatic, but objects must "own" their own outlets
    // python specific
    PyObject *p_globals;    // global python namespace (new ref)
} t_py;


// these are prototypes for the methods that are defined below
void py_bang(t_py *x);
void py_import(t_py *x, t_symbol *s);
void py_register(t_py *x, t_symbol *s);
void py_find(t_py *x, t_symbol *s);
void py_eval(t_py *x, t_symbol *s, long argc, t_atom *argv);
void py_run(t_py *x, t_symbol *s, long argc, t_atom *argv);
void py_dblclick(t_py *x);

void *py_new(t_symbol *s, long argc, t_atom *argv);
void py_free(t_py *x);

t_class *py_class;      // global pointer to the object class - so max can reference the object

/*--------------------------------------------------------------------------*/
// helper functions
void py_init(t_py *x);

void post_containers(t_py *x)
{
    t_patcher *p;
    t_box *b;
    // t_py *py;
    t_max_err err;

    err = object_obex_lookup(x, gensym("#P"), (t_object **)&p);
    err = object_obex_lookup(x, gensym("#B"), (t_object **)&b);
    // err = object_obex_lookup(x, gensym("py"), (t_object **)&py);

    post("my patcher is located at 0x%X", p);
    post("my box is located at 0x%X", b);
    // post("my py object is located at 0x%X", py); // gives 0x0
}

/*--------------------------------------------------------------------------*/
/* helper methods for embedded python interpreter
    The embedded python interpreter has access to calling application
    - external modules through imports msgs
    - embdedded modules:
        1. emb & emb_<methods> (which gives it selective access to the calling app)
        2. api: cython wrapped max api

*/
static PyObject* emb_classname(PyObject *self, PyObject *args)
{   //            global-------------vvvvvvvv
    const char *name = class_nameget(py_class)->s_name;
    return Py_BuildValue("s", name);
}

static PyObject *emb_check(PyObject *self, PyObject *args)
{
    t_object *obj = (t_object *)object_findregistered(gensym("myspace"), gensym("me"));
    if (obj) {
        ((t_py *)obj)->p_module = gensym("hello-python");
        return Py_BuildValue("s", "bang-bang");
    } else {
        return Py_BuildValue("s", "no-bang");
    }
}


static PyMethodDef EmbMethods[] = {
    {"classname", emb_classname, METH_VARARGS,
     "Return the classname of the max external."},

    {"check", emb_check, METH_VARARGS,
     "Bang if test succeeds."},

     /* terminating sentinel */
    {NULL, NULL, 0, NULL}
};

static PyModuleDef EmbModule = {
    PyModuleDef_HEAD_INIT, "emb", NULL, -1, EmbMethods,
    NULL, NULL, NULL, NULL
};

static PyObject* PyInit_emb(void)
{
    return PyModule_Create(&EmbModule);
}



/*--------------------------------------------------------------------------*/

void ext_main(void *r)
{
    t_class *c;

    c = class_new("py", (method)py_new, (method)py_free, (long)sizeof(t_py),
                  0L /* leave NULL!! */, A_GIMME, 0);

    // methods
    class_addmethod(c, (method)py_bang,       "bang",       0);
    class_addmethod(c, (method)py_import,     "import",     A_DEFSYM, 0);
    class_addmethod(c, (method)py_register,   "register",   A_DEFSYM, 0);
    class_addmethod(c, (method)py_find,       "find",       A_DEFSYM, 0);
    class_addmethod(c, (method)py_eval,       "anything",   A_GIMME,  0);
    class_addmethod(c, (method)py_run,        "run",        A_GIMME,  0);

    /* you CAN'T call this from the patcher */
    class_addmethod(c, (method)py_dblclick, "dblclick", A_CANT, 0);

    // attributes
    CLASS_ATTR_SYM(c, "module", 0, t_py, p_module);
    CLASS_ATTR_BASIC(c, "module", 0);

    CLASS_ATTR_SYM(c, "code",    0, t_py, p_code);
    CLASS_ATTR_BASIC(c, "code", 0);

    class_register(CLASS_BOX, c);
    py_class = c;

    post("py object loaded...",0);
}


/*--------------------------------------------------------------------------*/

void *py_new(t_symbol *s, long argc, t_atom *argv)
{
    t_py *x;

    x = (t_py *)object_alloc(py_class);

    if (x) {
        x->p_module = gensym("");
        x->p_code = gensym("");

        // create inlet(s)
        // create outlet(s)
        x->p_outlet = outlet_new(x, NULL);
        
        // process @arg attributes
        attr_args_process(x, argc, argv);

        // python init
        py_init(x);
    }

    post("new py object instance added to patch...", 0);
    return(x);
}

void py_init(t_py *x)
{
    // PyObject *pmodule;
    // wchar_t *program = "pymx";

    /* Add a built-in module, before Py_Initialize */
    if (PyImport_AppendInittab("api", PyInit_api) == -1) {
        error("Error: could not extend in-built modules table\n");
    }

    // Py_SetProgramName(program);
    PyImport_AppendInittab("emb", &PyInit_emb);

    Py_Initialize();

    // python init
    PyObject *main_module = PyImport_AddModule("__main__"); // borrowed reference
    x->p_globals = PyModule_GetDict(main_module);
    // TODO: add init_py() here add module / import extension ...
}


void py_free(t_py *x)
{
    post("freeing globals and terminating py interpreter...");
    Py_DECREF(x->p_globals);
    Py_FinalizeEx();
}


//--------------------------------------------------------------------------

void py_dblclick(t_py *x)
{
    object_post((t_object *)x, "I got a double-click");
}


void py_import(t_py *x, t_symbol *s) {
    //x->p_module = s;
    if (s != gensym("")) {
        PyObject* x_module = PyImport_ImportModule(s->s_name); // x_module borrrowed ref
        PyDict_SetItemString(x->p_globals, s->s_name, x_module);
        post("imported: %s", s->s_name);
    }
}


void py_register(t_py *x, t_symbol *s) {
    object_register(gensym("myspace"), gensym("me"), x);
    post("object registered");
}

void py_find(t_py *x, t_symbol *s) {
    t_object *obj = (t_object *)object_findregistered(gensym("myspace"), gensym("me"));
    if (obj) py_bang((t_py *)obj);
}


void py_run(t_py *x, t_symbol *s, long argc, t_atom *argv) {

    if (gensym(s->s_name) == gensym("run")) {
        PyObject *obj = Py_BuildValue("s", atom_getsym(argv)->s_name);
         FILE *file = _Py_fopen_obj(obj, "r+");
         if (file != NULL) {
             PyRun_SimpleFile(file, "script.py");
         }        
    }
}

void py_eval(t_py *x, t_symbol *s, long argc, t_atom *argv) {

    if (gensym(s->s_name) == gensym("eval") || gensym(s->s_name) == gensym("exec") ||
        gensym(s->s_name) == gensym("execfile")) {

        PyObject *pval = NULL; // python value

        char *py_argv = atom_getsym(argv)->s_name;
        post("%s: %s", s->s_name, py_argv);

        // local are per eval
        PyObject *locals = PyDict_New();

        if (gensym(s->s_name) == gensym("execfile")) {
            post("execfile:path: %s", py_argv);
            FILE* fhandle = fopen(py_argv, "r");
            if (fhandle != NULL) {
                // char *fname = basename(py_argv); // TODO (Causes Crash!)
                // post("execfile:fname: %s", fname);
                pval = PyRun_File(fhandle, "script.py", Py_file_input, x->p_globals, locals);
            }
            fclose(fhandle);
            post("execfile:fhandle closed: %s", py_argv);
        }

        else if (gensym(s->s_name) == gensym("exec")) {
            post("exec:code: %s", py_argv);
            pval = PyRun_String(py_argv, Py_single_input, x->p_globals, locals);
        }

        else if (gensym(s->s_name) == gensym("eval")) {
            post("eval:code: %s", py_argv);
            pval = PyRun_String(py_argv, Py_eval_input, x->p_globals, locals);
        } 

        else { // redundant!
            error("this should not be reached!");
            return;
        }

        if (pval) {

            if (PyLong_Check(pval)) {
                long int_result = PyLong_AsLong(pval);
                outlet_int(x->p_outlet, int_result);
            } 

            else if (PyFloat_Check(pval)) {
                float float_result = (float) PyFloat_AsDouble(pval);
                outlet_float(x->p_outlet, float_result);
            }

            else if (PyUnicode_Check(pval)) {
                const char *unicode_result = PyUnicode_AsUTF8(pval);
                outlet_anything(x->p_outlet, gensym(unicode_result), 0, NIL);
            }

            else if (PyList_Check(pval) || PyTuple_Check(pval) || PyAnySet_Check(pval)) {
                PyObject *iter;
                PyObject *item;
                int i = 0;

                t_atom atoms_static[PY_MAX_ATOMS];
                t_atom *atoms;
                int is_dynamic = 0;

                Py_ssize_t seq_size = PySequence_Length(pval);

                if (seq_size > PY_MAX_ATOMS) {
                    post("dynamically increasing size of atom array");
                    atoms = atom_dynamic_start(atoms_static, PY_MAX_ATOMS, seq_size+1);
                    is_dynamic = 1;

                } else {
                    atoms = atoms_static;
                }

                if ((iter = PyObject_GetIter(pval)) == NULL) {
                    error("PyObject_GetIter(PyList||PyTuple||PySet) returns NULL");
                    return;
                }

                while ((item = PyIter_Next(iter)) != NULL) {
                    if (PyLong_Check(item)) {
                        long long_item = PyLong_AsLong(item);
                        atom_setlong(atoms+i, long_item);
                        post("%d long: %ld\n", i, long_item);
                        i++;
                    }
                    
                    if PyFloat_Check(item) {
                        float float_item = PyFloat_AsDouble(item);
                        atom_setfloat(atoms+i, float_item);
                        post("%d float: %f\n", i, float_item);
                        i++;
                    }

                    if PyUnicode_Check(item) {
                        const char *unicode_item = PyUnicode_AsUTF8(item);
                        post("%d unicode: %s\n", i, unicode_item);
                        atom_setsym(atoms+i, gensym(unicode_item));
                        i++;
                    }

                    Py_DECREF(item);
                }
                outlet_anything(x->p_outlet, gensym("res"), i, atoms);
                post("end pylist op: %d", i);

                if (is_dynamic) {
                    post("restoring to static atom array");
                    atom_dynamic_end(atoms_static, atoms);
                }
            }

            else {
                error("failure to evaluate expression");
            }

            Py_DECREF(pval);

        } else {
            if (PyErr_Occurred()) {
                // PyErr_Print();
                PyObject *ptype, *pvalue, *ptraceback;
                PyErr_Fetch(&ptype, &pvalue, &ptraceback);
                //pvalue contains error message
                //ptraceback contains stack snapshot and many other information
                //(see python traceback structure)

                //Get error message
                const char *pStrErrorMessage = PyUnicode_AsUTF8(pvalue);
                error("%s: %s", pStrErrorMessage, py_argv);
                Py_DECREF(ptype);
                Py_DECREF(pvalue);
                Py_DECREF(ptraceback);
            }
        }
        
        // --------------
        // new refs
        Py_DECREF(locals);
        // --------------

    }
    return;  
}


void py_bang(t_py *x)
{
    outlet_bang(x->p_outlet);
}
