// py.c

/* py external api */
#include "py.h"

/* max/msp api */
#include "api.h"

/*--------------------------------------------------------------------------*/
// GLOBALS

t_class* py_class; // global pointer to object class

static int py_global_obj_count = 0;

/*--------------------------------------------------------------------------*/
// helper functions

/*--------------------------------------------------------------------------*/
// main

void ext_main(void* r)
{
    t_class* c;

    c = class_new("py", (method)py_new, (method)py_free, (long)sizeof(t_py),
                  0L /* leave NULL!! */, A_GIMME, 0);

    // clang-format off
    //------------------------------------------------------------------------
    // methods
    class_addmethod(c, (method)py_bang,       "bang",       0);
    class_addmethod(c, (method)py_sym,        "sym",        A_SYM, 0);

    // core
    class_addmethod(c, (method)py_import,     "import",     A_SYM,    0);
    class_addmethod(c, (method)py_eval,       "eval",       A_GIMME,  0);
    class_addmethod(c, (method)py_exec,       "exec",       A_GIMME,  0);
    class_addmethod(c, (method)py_execfile,   "execfile",   A_SYM,    0);
    class_addmethod(c, (method)py_load,       "load",       A_SYM,    0);
    

    /* you CAN'T call this from the patcher */
    class_addmethod(c, (method)py_assist,     "assist",     A_CANT, 0);

    // meta
    class_addmethod(c, (method)py_count,      "count",      A_NOTHING, 0);

    // code editor
    class_addmethod(c, (method)py_read,       "read",       A_DEFSYM, 0);
    class_addmethod(c, (method)py_dblclick,   "dblclick",   A_CANT, 0);
    class_addmethod(c, (method)py_edclose,    "edclose",    A_CANT, 0);
    class_addmethod(c, (method)py_edsave,     "edsave",     A_CANT, 0);

    // attributes
    CLASS_ATTR_SYM(c, "name", 0,        t_py, p_name);
    // CLASS_ATTR_INVISIBLE(c, "name", 0);
    CLASS_ATTR_BASIC(c, "name", 0);

    CLASS_ATTR_SYM(c,   "file", 0,      t_py, p_code_filepath);
    CLASS_ATTR_STYLE(c, "file", 0,      "file");
    CLASS_ATTR_BASIC(c, "file", 0);

    //------------------------------------------------------------------------
    // clang-format on

    class_register(CLASS_BOX, c);
    py_class = c;

    post("py object loaded...", 0);
}

/*--------------------------------------------------------------------------*/
// object creation, initialization and release

void* py_new(t_symbol* s, long argc, t_atom* argv)
{
    t_py* x = NULL;

    x = (t_py*)object_alloc(py_class);

    if (x) {
        // core
        x->p_name = symbol_unique();

        // if (!object_obex_lookup(x, gensym("#P"), (t_object
        // **)&x->p_patcher))
        //     error("patcher object not created.");
        // if (!object_obex_lookup(x, gensym("#B"), (t_object **)&x->p_box))
        //     error("box obejct not created.");

        // text editor
        x->p_code = sysmem_newhandle(0);
        x->p_code_size = 0;
        x->p_code_editor = NULL;
        x->p_code_filepath = gensym("");

        // create inlet(s)
        // create outlet(s)
        x->p_outlet = outlet_new(x, NULL);

        // process @arg attributes
        attr_args_process(x, argc, argv);

        // python init
        py_init(x);

        for (int i = 0; i < argc; i++) {
            post("argc: %d  argv: %s", i, atom_getsym(argv + i)->s_name);
        }
    }

    post("new py object %s added to patch...", x->p_name->s_name);
    return (x);
}

void py_init(t_py* x)
{
    /* Add the cythonized 'api' built-in module, before Py_Initialize */
    if (PyImport_AppendInittab("api", PyInit_api) == -1) {
        error("Error: could not extend in-built modules table\n");
    }

    Py_Initialize();

    // python init
    PyObject* main_mod = PyImport_AddModule(x->p_name->s_name); // borrowed
    x->p_globals = PyModule_GetDict(main_mod); // borrowed reference
    PyDict_SetItemString(x->p_globals, "__builtins__", PyEval_GetBuiltins());

    /* start: add additional python objects to the globals dict here */
    /* end */

    post("globals initialized");
    object_register(CLASS_BOX, x->p_name, x);
    post("object registered");

    // increment global object counter
    py_global_obj_count++;
}

void py_free(t_py* x)
{
    // code editor cleanup
    object_free(x->p_code_editor);
    if (x->p_code)
        sysmem_freehandle(x->p_code);

    // python objects cleanup
    py_global_obj_count--;
    post("py object '%s' deleted", x->p_name->s_name);
    if (py_global_obj_count == 0) {
        post("no py objects left: freeing memory / terminating py "
             "interpreter.");
        Py_FinalizeEx();
    }
}

/*--------------------------------------------------------------------------*/
// help

void py_assist(t_py* x, void* b, long m, long a, char* s)
{
    if (m == ASSIST_INLET) { // inlet
        sprintf(s, "I am inlet %ld", a);
    } else { // outlet
        sprintf(s, "I am outlet %ld", a);
    }
}

/*--------------------------------------------------------------------------*/
// object methods

void py_bang(t_py* x) { outlet_bang(x->p_outlet); }

// retrieves a count of the number of 'active' py objects from a global var
void py_count(t_py* x) { outlet_int(x->p_outlet, py_global_obj_count); }

void py_sym(t_py* x, t_symbol* s) { post("py.sym %s", s->s_name); }

/*--------------------------------------------------------------------------*/
// code editor

void py_dblclick(t_py* x)
{
    if (x->p_code_editor)
        object_attr_setchar(x->p_code_editor, gensym("visible"), 1);
    else {
        x->p_code_editor = object_new(CLASS_NOBOX, gensym("jed"), x, 0);
        object_method(x->p_code_editor, gensym("settext"), *x->p_code,
                      gensym("utf-8"));
        object_attr_setchar(x->p_code_editor, gensym("scratch"), 1);
        object_attr_setsym(x->p_code_editor, gensym("title"),
                           gensym("py-editor"));
    }
}

void py_read(t_py* x, t_symbol* s)
{
    defer((t_object*)x, (method)py_doread, s, 0, NULL);
}

void py_doread(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    char filename[MAX_PATH_CHARS];
    short path;
    t_fourcc type = FOUR_CHAR_CODE('TEXT');
    long err;
    t_filehandle fh;

    if (s == gensym("")) {
        if (x->p_code_filepath != gensym("")) {
            strcpy(filename, x->p_code_filepath->s_name);
        } else {
            filename[0] = 0;
            if (open_dialog(filename, &path, &type, &type, 1))
                x->p_code_filepath = gensym(filename);
            return;
        }
    } else {
        strcpy(filename, s->s_name);
        x->p_code_filepath = s;
    }

    if (locatefile_extended(filename, &path, &type, &type, 1)) {
        object_error((t_object*)x, "can't find file %s", filename);
        return;
    }

    // success
    err = path_opensysfile(filename, path, &fh, READ_PERM);
    if (!err) {
        sysfile_readtextfile(fh, x->p_code, 0,
                             TEXT_LB_UNIX | TEXT_NULL_TERMINATE);
        sysfile_close(fh);
        x->p_code_size = sysmem_handlesize(x->p_code);
    }
}

void py_edclose(t_py* x, char** text, long size)
{
    if (x->p_code)
        sysmem_freehandle(x->p_code);

    x->p_code = sysmem_newhandleclear(size + 1);
    sysmem_copyptr((char*)*text, *x->p_code, size);
    x->p_code_size = size + 1;
    x->p_code_editor = NULL;
}

void py_edsave(t_py* x, char** text, long size)
{
    post("saving editor code to %s", x->p_code_filepath->s_name);
    py_execfile(x, x->p_code_filepath);
}

/*--------------------------------------------------------------------------*/
// python engine services

void py_import(t_py* x, t_symbol* s)
{
    PyObject* x_module = NULL;

    if (s != gensym("")) {
        x_module = PyImport_ImportModule(s->s_name); // x_module borrrowed ref
        if (x_module == NULL) {
            goto error;
        }
        PyDict_SetItemString(x->p_globals, s->s_name, x_module);
        post("py.imported: %s", s->s_name);
    }

error:
    if (PyErr_Occurred()) {
        PyObject *ptype, *pvalue, *ptraceback;
        PyErr_Fetch(&ptype, &pvalue, &ptraceback);
        const char* pStrErrorMessage = PyUnicode_AsUTF8(pvalue);
        error("PyException('py.import %s'): %s", s->s_name, pStrErrorMessage);
        Py_XDECREF(ptype);
        Py_XDECREF(pvalue);
        Py_XDECREF(ptraceback);
    }
}

void py_eval(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    char* py_argv = atom_getsym(argv)->s_name;
    post("py.%s: %s", s->s_name, py_argv);

    PyObject* locals = PyDict_New();
    PyObject* pval = PyRun_String(py_argv, Py_eval_input, x->p_globals,
                                  locals);

    if (pval != NULL) {
        // if (PyErr_ExceptionMatches(PyExc_SyntaxError)) {
        //     error("Cannot import in 'eval', use 'import' msg instead.");
        //     PyErr_Clear();
        // }

        // handle ints and longs
        if (PyLong_Check(pval)) {
            long int_result = PyLong_AsLong(pval);
            outlet_int(x->p_outlet, int_result);
        }

        // handle floats and doubles
        if (PyFloat_Check(pval)) {
            float float_result = (float)PyFloat_AsDouble(pval);
            outlet_float(x->p_outlet, float_result);
        }

        // handle strings
        if (PyUnicode_Check(pval)) {
            const char* unicode_result = PyUnicode_AsUTF8(pval);
            outlet_anything(x->p_outlet, gensym(unicode_result), 0, NIL);
        }

        // handle lists, tuples and sets
        if (PyList_Check(pval) || PyTuple_Check(pval)
            || PyAnySet_Check(pval)) {
            PyObject* iter;
            PyObject* item;
            int i = 0;

            t_atom atoms_static[PY_MAX_ATOMS];
            t_atom* atoms;
            int is_dynamic = 0;

            Py_ssize_t seq_size = PySequence_Length(pval);

            if (seq_size > PY_MAX_ATOMS) {
                post("dynamically increasing size of atom array");
                atoms = atom_dynamic_start(atoms_static, PY_MAX_ATOMS,
                                           seq_size + 1);
                is_dynamic = 1;

            } else {
                atoms = atoms_static;
            }

            if ((iter = PyObject_GetIter(pval)) != NULL) {
                while ((item = PyIter_Next(iter)) != NULL) {
                    if (PyLong_Check(item)) {
                        long long_item = PyLong_AsLong(item);
                        atom_setlong(atoms + i, long_item);
                        post("%d long: %ld\n", i, long_item);
                        i++;
                    }

                    if PyFloat_Check (item) {
                        float float_item = PyFloat_AsDouble(item);
                        atom_setfloat(atoms + i, float_item);
                        post("%d float: %f\n", i, float_item);
                        i++;
                    }

                    if PyUnicode_Check (item) {
                        const char* unicode_item = PyUnicode_AsUTF8(item);
                        post("%d unicode: %s\n", i, unicode_item);
                        atom_setsym(atoms + i, gensym(unicode_item));
                        i++;
                    }
                    Py_DECREF(item);
                }
                outlet_anything(x->p_outlet, gensym("list"), i, atoms);
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
        // if (PyErr_ExceptionMatches(PyExc_SyntaxError)) {
        //     error("Cannot import in 'eval', use 'import' msg instead.");
        // }
        if (PyErr_Occurred()) {
            PyObject *ptype, *pvalue, *ptraceback;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);

            // Get error message
            const char* pStrErrorMessage = PyUnicode_AsUTF8(pvalue);
            error("PyException('py.%s %s'): %s", s->s_name, py_argv,
                  pStrErrorMessage);
            Py_DECREF(ptype);
            Py_DECREF(pvalue);
            Py_DECREF(ptraceback);
        }
        // cleanup
        Py_XDECREF(pval);
    }
}

void py_exec(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    char* py_argv = NULL;
    PyObject* pval = NULL;

    py_argv = atom_getsym(argv)->s_name;
    if (py_argv == NULL) {
        error("py.%s: could not retrieve argv", s->s_name);
        goto error;
    }
    post("START py.%s: %s", s->s_name, py_argv);

    pval = PyRun_String(py_argv, Py_single_input, x->p_globals, x->p_globals);
    if (pval == NULL) {
        goto error;
    }

    // success cleanup
    Py_DECREF(pval);
    post("END py.%s: %s", s->s_name, py_argv);

error:
    if (PyErr_Occurred()) {
        PyObject *ptype, *pvalue, *ptraceback;
        PyErr_Fetch(&ptype, &pvalue, &ptraceback);
        const char* pStrErrorMessage = PyUnicode_AsUTF8(pvalue);
        error("PyException('py.%s %s'): %s", s->s_name, py_argv,
              pStrErrorMessage);
        Py_XDECREF(pval);
        Py_XDECREF(ptype);
        Py_XDECREF(pvalue);
        Py_XDECREF(ptraceback);
    }
}

void py_execfile(t_py* x, t_symbol* s)
{
    PyObject* pval = NULL;
    FILE* fhandle = NULL;

    if (s == gensym("")) {
        error("py.execfile: missing filepath");
        goto error;
    }
    post("START py.execfile: %s", s->s_name);

    fhandle = fopen(s->s_name, "r");
    if (fhandle == NULL) {
        error("could not open file '%s'", s->s_name);
        goto error;
    }

    pval = PyRun_File(fhandle, s->s_name, Py_file_input, x->p_globals,
                      x->p_globals);
    if (pval == NULL) {
        goto error;
    }

    // success cleanup
    fclose(fhandle);
    Py_DECREF(pval);
    post("END py.execfile: %s", s->s_name);

error:
    if (PyErr_Occurred()) {
        PyObject *ptype, *pvalue, *ptraceback;
        PyErr_Fetch(&ptype, &pvalue, &ptraceback);
        const char* pStrErrorMessage = PyUnicode_AsUTF8(pvalue);
        error("PyException('py.execfile %s'): %s", s->s_name,
              pStrErrorMessage);
        Py_XDECREF(pval);
        Py_XDECREF(ptype);
        Py_XDECREF(pvalue);
        Py_XDECREF(ptraceback);
    }
}

void py_load(t_py* x, t_symbol* s)
{
    post("py.load: %s", s->s_name);
    py_read(x, s);
    py_execfile(x, s);
}
