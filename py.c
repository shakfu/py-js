
/* py external api */
#include "py.h"

/* max/msp api */
#include "api.h"


t_class *py_class;      // global pointer to object class - so max can reference the object

/*--------------------------------------------------------------------------*/
// helper functions


// void post_containers(t_py *x)
// {
//     t_patcher *p;
//     t_box *b;
//     t_max_err err;

//     err = object_obex_lookup(x, gensym("#P"), (t_object **)&p);
//     err = object_obex_lookup(x, gensym("#B"), (t_object **)&b);

//     post("my patcher is located at 0x%X", p);
//     post("my box is located at 0x%X", b);
// }

/*--------------------------------------------------------------------------*/




/*--------------------------------------------------------------------------*/

void ext_main(void *r)
{
    t_class *c;

    c = class_new("py", (method)py_new, (method)py_free, (long)sizeof(t_py),
                  0L /* leave NULL!! */, A_GIMME, 0);

    // methods
    class_addmethod(c, (method)py_bang,       "bang",       0);
    class_addmethod(c, (method)py_import,     "import",     A_DEFSYM, 0);
    class_addmethod(c, (method)py_eval,       "eval",       A_GIMME,  0);
    class_addmethod(c, (method)py_exec,       "exec",       A_GIMME,  0);
    class_addmethod(c, (method)py_execfile,   "execfile",   A_GIMME,  0);
    class_addmethod(c, (method)py_run,        "run",        A_GIMME,  0);

    /* you CAN'T call this from the patcher */
    class_addmethod(c, (method)py_dblclick, "dblclick", A_CANT, 0);

    // attributes
    CLASS_ATTR_SYM(c, "module", 0, t_py, p_module);
    CLASS_ATTR_BASIC(c, "module", 0);

    CLASS_ATTR_SYM(c, "code",    0, t_py, p_code);
    CLASS_ATTR_BASIC(c, "code", 0);

    CLASS_ATTR_SYM(c, "name", 0, t_py, p_name);
    CLASS_ATTR_BASIC(c, "name", 0);

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
        x->p_name = symbol_unique();
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

    post("new py object %s added to patch...", x->p_name->s_name);
    return(x);
}

void py_init(t_py *x)
{
    // PyObject *pmodule;
    // wchar_t *program = "pymax";

    /* Add a built-in module, before Py_Initialize */
    if (PyImport_AppendInittab("api", PyInit_api) == -1) {
        error("Error: could not extend in-built modules table\n");
    }

    // Py_SetProgramName(program);

    Py_Initialize();

    // python init
    int err;
    PyObject *main_module = PyImport_AddModule(x->p_name->s_name); // borrowed reference
    // PyObject *main_module = PyImport_AddModule("__main__"); // borrowed reference
    x->p_globals = PyModule_GetDict(main_module);           // borrowed reference
    PyDict_SetItemString(x->p_globals, "__builtins__", PyEval_GetBuiltins());
    // PyUnicode_FromString -- new reference
    err = PyDict_SetItemString(x->p_globals, PY_MAX_NAME, PyUnicode_FromString(x->p_name->s_name));
    if (err != 0) {
        error("PyDict_SetItemString with key '%s', failed", PY_MAX_NAME);
    }
    // PyLong_FromLong -- new reference
    err = PyDict_SetItemString(x->p_globals, "testing", PyLong_FromLong(20));
    if (err != 0) {
        error("PyDict_SetItemString with key 'testing', failed");
    }
    post("globals initialized");
    object_register(gensym(PY_NAMESPACE), x->p_name, x);
    post("object registered");
}


void py_free(t_py *x)
{
    post("freeing globals and terminating py interpreter...");
    Py_FinalizeEx();
}


//--------------------------------------------------------------------------

void py_dblclick(t_py *x)
{
    object_post((t_object *)x, "I got a double-click");
}


void py_import(t_py *x, t_symbol *s)
{
    PyObject* x_module = NULL;

    if (s != gensym("")) {
        x_module = PyImport_ImportModule(s->s_name); // x_module borrrowed ref
        if (x_module == NULL) {
            goto error;
        }
        // TODO: should I Py_INCREF here  and Py_DECREF at the end
        // Py_INCREF(x_module);
        PyDict_SetItemString(x->p_globals, s->s_name, x_module);
        // Py_XDECREF(x_module);
        post("imported: %s", s->s_name);
    }

    error:
        if(PyErr_Occurred()) {
            PyObject *ptype, *pvalue, *ptraceback;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);
            const char *pStrErrorMessage = PyUnicode_AsUTF8(pvalue);
            error("PyException('import %s'): %s", s->s_name, pStrErrorMessage);
            Py_XDECREF(ptype);
            Py_XDECREF(pvalue);
            Py_XDECREF(ptraceback);
            // Py_XDECREF(x_module);
        }
}


void py_run(t_py *x, t_symbol *s, long argc, t_atom *argv)
{
    PyObject *pval = NULL;
    FILE* fhandle  = NULL;
    char *py_argv = NULL;
    int ret = -0;

    py_argv = atom_getsym(argv)->s_name;
    if (py_argv == NULL) {
        error("%s: could not retrieve argv", s->s_name);
        goto error;
    }
    post("START %s: %s", s->s_name, py_argv);

    pval = Py_BuildValue("s", py_argv); // new reference
    if (pval == NULL) {
        goto error;
    }

    fhandle = _Py_fopen_obj(pval, "r+");
    if (fhandle == NULL) {
        error("could not open file '%s'", py_argv);
        goto error;
    }

    ret = PyRun_SimpleFile(fhandle, py_argv);
    if (ret == -1){
        goto error;
    }

    // success
    fclose(fhandle);
    Py_DECREF(pval);
    post("END %s: %s", s->s_name, py_argv);

    error:
        if(PyErr_Occurred()) {
            PyObject *ptype, *pvalue, *ptraceback;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);
            const char *pStrErrorMessage = PyUnicode_AsUTF8(pvalue);
            error("PyException('%s %s'): %s", s->s_name, py_argv, pStrErrorMessage);
            Py_XDECREF(pval);   
            Py_XDECREF(ptype);
            Py_XDECREF(pvalue);
            Py_XDECREF(ptraceback);
        }
}


void py_execfile(t_py *x, t_symbol *s, long argc, t_atom *argv)
{
    PyObject *pval = NULL;
    FILE* fhandle  = NULL;
    char *py_argv = NULL;

    py_argv = atom_getsym(argv)->s_name;
    if (py_argv == NULL) {
        error("%s: could not retrieve argv", s->s_name);
        goto error;
    }
    post("START %s: %s", s->s_name, py_argv);

    fhandle = fopen(py_argv, "r");
    if (fhandle == NULL) {
        error("could not open file '%s'", py_argv);
        goto error;
    }

    pval = PyRun_File(fhandle, py_argv, Py_file_input, x->p_globals, x->p_globals);
    if (pval == NULL) {
        goto error;
    }

    // success cleanup
    fclose(fhandle);
    Py_DECREF(pval);
    post("END %s: %s", s->s_name, py_argv);

    error:
        if(PyErr_Occurred()) {
            PyObject *ptype, *pvalue, *ptraceback;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);
            const char *pStrErrorMessage = PyUnicode_AsUTF8(pvalue);
            error("PyException('%s %s'): %s", s->s_name, py_argv, pStrErrorMessage);
            Py_XDECREF(pval);   
            Py_XDECREF(ptype);
            Py_XDECREF(pvalue);
            Py_XDECREF(ptraceback);
        }
}


void py_exec(t_py *x, t_symbol *s, long argc, t_atom *argv)
{
    char *py_argv = NULL;
    PyObject *pval = NULL;

    py_argv = atom_getsym(argv)->s_name;
    if (py_argv == NULL) {
        error("%s: could not retrieve argv", s->s_name);
        goto error;
    }
    post("START %s: %s", s->s_name, py_argv);

    pval = PyRun_String(py_argv, Py_single_input, x->p_globals, x->p_globals);
    if (pval == NULL) {
        goto error;
    }

    // success cleanup
    Py_DECREF(pval);
    post("END %s: %s", s->s_name, py_argv);

    error:
        if(PyErr_Occurred()) {
            PyObject *ptype, *pvalue, *ptraceback;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);
            const char *pStrErrorMessage = PyUnicode_AsUTF8(pvalue);
            error("PyException('%s %s'): %s", s->s_name, py_argv, pStrErrorMessage);
            Py_XDECREF(pval);   
            Py_XDECREF(ptype);
            Py_XDECREF(pvalue);
            Py_XDECREF(ptraceback);
        }
}

void py_eval(t_py *x, t_symbol *s, long argc, t_atom *argv)
{
    char *py_argv = atom_getsym(argv)->s_name;
    post("%s: %s", s->s_name, py_argv);

    PyObject *locals = PyDict_New();
    PyObject *pval = PyRun_String(py_argv, Py_eval_input, x->p_globals, locals);

    if (pval != NULL) {

        // handle ints and longs
        if (PyLong_Check(pval)) {
            long int_result = PyLong_AsLong(pval);
            outlet_int(x->p_outlet, int_result);
        } 

        // handle floats and doubles
        if (PyFloat_Check(pval)) {
            float float_result = (float) PyFloat_AsDouble(pval);
            outlet_float(x->p_outlet, float_result);
        }

        // handle strings
        if (PyUnicode_Check(pval)) {
            const char *unicode_result = PyUnicode_AsUTF8(pval);
            outlet_anything(x->p_outlet, gensym(unicode_result), 0, NIL);
        }

        // handle lists, tuples and sets
        if (PyList_Check(pval) || PyTuple_Check(pval) || PyAnySet_Check(pval)) {
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

            if ((iter = PyObject_GetIter(pval)) != NULL) {
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
                // outlet_anything(x->p_outlet, gensym("list"), i, atoms);
                outlet_list(x->p_outlet, NULL, i, atoms);
                post("end iter op: %d", i);
            }

            if (is_dynamic) {
                post("restoring to static atom array");
                atom_dynamic_end(atoms_static, atoms);
            }
        }

        // cleanup
        Py_XDECREF(pval);
    }

    else {
        if (PyErr_Occurred()) {
            PyObject *ptype, *pvalue, *ptraceback;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);

            //Get error message
            const char *pStrErrorMessage = PyUnicode_AsUTF8(pvalue);
            error("PyException('%s %s'): %s", s->s_name, py_argv, pStrErrorMessage);
            Py_DECREF(ptype);
            Py_DECREF(pvalue);
            Py_DECREF(ptraceback);
        }
        // cleanup
        Py_XDECREF(pval);
    }
}



void py_bang(t_py *x)
{
    outlet_bang(x->p_outlet);
}
