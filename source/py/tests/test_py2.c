
/* python */
#define PY_SSIZE_T_CLEAN
#include <Python.h>

/* --------------------------------------- */
// types

typedef struct _py {
    PyObject* p_globals;
} t_py;

/* --------------------------------------- */
// forward func declarations

void py_import(t_py* x, int argc, char* argv);
void py_eval(t_py* x, int argc, char* argv);
void py_exec(t_py* x, int argc, char* argv);
void py_execfile(t_py* x, int argc, char* argv);
void py_assign(t_py* x, int argc, char* argv);
void py_anything(t_py* x, int argc, char* argv);
void handle_py_error(void);

int main(int argc, char* argv[])
{
    t_py obj = { .p_globals = NULL };
    t_py* x = &obj;

    Py_Initialize();

    // python init
    PyObject* main_module = PyImport_AddModule("__main__"); // borrowed
    x->p_globals = PyModule_GetDict(main_module); // borrowed reference

    if (argc > 2) {
        if (strcmp(argv[1], "import") == 0)
            py_import(x, argc, argv);
        else if (strcmp(argv[1], "eval") == 0)
            py_eval(x, argc, argv);
        else if (strcmp(argv[1], "exec") == 0)
            py_exec(x, argc, argv);
        else if (strcmp(argv[1], "execfile") == 0)
            py_execfile(x, argc, argv);
        else if (strcmp(argv[1], "assign") == 0)
            py_assign(x, argc, argv);
        else py_anything(x, argc, argv);

    } else {
        printf("usage: test [import, eval, exec, execfile, assign, ...] args\n");
    }

    Py_FinalizeEx();
    return 0;
}


/* python error handler helper function */
void handle_py_error(void)
{
    if (PyErr_Occurred()) {
        PyErr_Print();
    }
}


/*--------------------------------------------------------------------------*/
// core python methods

void py_import(t_py* x, int argc, char* argv[])
{
    PyObject* module = NULL;

    if (argv != NULL) {
        module = PyImport_ImportModule(argv[2]); // module borrrowed ref
        if (module == NULL) {
            goto error;
        }
        PyDict_SetItemString(x->p_globals, argv[2], module);
        printf("imported: %s\n", argv[2]);
    }
    return;
error:
    handle_py_error();
}


// see: https://codereview.stackexchange.com/questions/71970/creating-a-string-from-command-line-arguments
// see also: (how last entry should be null terminated e.g. filename[63] = '\0';)
// https://stackoverflow.com/questions/19194254/how-can-i-copy-the-contents-of-argv-into-a-c-style-string


void py_eval(t_py* x, int argc, char* argv[])
{
    char* py_argv = atom_getsym(argv)->s_name;
    py_log(x, "%s %s", s->s_name, py_argv);

    PyObject* pval = PyRun_String(argv[2], Py_eval_input, x->p_globals,
                                  x->p_globals);

    if (pval != NULL) {

        // handle ints and longs
        if (PyLong_Check(pval)) {
            long int_result = PyLong_AsLong(pval);
        }

        // handle floats and doubles
        if (PyFloat_Check(pval)) {
            float float_result = (float)PyFloat_AsDouble(pval);
        }

        // handle strings
        if (PyUnicode_Check(pval)) {
            const char* unicode_result = PyUnicode_AsUTF8(pval);

        }

        // handle any sequence except strings, and presently
        // bytes and byte arrays (until there is a reason to)
        if (PySequence_Check(pval) && !PyUnicode_Check(pval)
            && !PyBytes_Check(pval) && !PyByteArray_Check(pval)) {
            PyObject* iter;
            PyObject* item;
            int i = 0;

            Py_ssize_t seq_size = PySequence_Length(pval);

            if ((iter = PyObject_GetIter(pval)) != NULL) {
                while ((item = PyIter_Next(iter)) != NULL) {
                    if (PyLong_Check(item)) {
                        long long_item = PyLong_AsLong(item);
                        py_log(x, "%d long: %ld\n", i, long_item);
                        i++;
                    }

                    if PyFloat_Check (item) {
                        float float_item = PyFloat_AsDouble(item);
                        py_log(x, "%d float: %f\n", i, float_item);
                        i++;
                    }

                    if PyUnicode_Check (item) {
                        const char* unicode_item = PyUnicode_AsUTF8(item);
                        py_log(x, "%d unicode: %s\n", i, unicode_item);
                        i++;
                    }
                    Py_DECREF(item);
                }
                py_log(x, "end iter op: %d", i);
            }

        }

        // cleanup
        Py_XDECREF(pval);
    }

    else {
        handle_py_error();
        // cleanup
        Py_XDECREF(pval);
    }
}

void py_exec(t_py* x, int argc, char* args)
{
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
    outlet_bang(x->p_outlet0);

    // success cleanup
    Py_DECREF(pval);
    py_log(x, "exec %s", py_argv);
    return;

error:
    handle_py_error(x, "exec %s", py_argv);
    Py_XDECREF(pval);
}

void py_execfile(t_py* x, int argc, char* args)
{
    PyObject* pval = NULL;
    FILE* fhandle = NULL;

    if (s == gensym("")) {
        py_error(x, "py execfile: missing filepath");
        goto error;
    }

    fhandle = fopen(s->s_name, "r");
    if (fhandle == NULL) {
        py_error(x, "could not open file '%s'", s->s_name);
        goto error;
    }

    pval = PyRun_File(fhandle, s->s_name, Py_file_input, x->p_globals,
                      x->p_globals);
    if (pval == NULL) {
        goto error;
    }
    outlet_bang(x->p_outlet0);

    // success cleanup
    fclose(fhandle);
    Py_DECREF(pval);
    py_log(x, "execfile %s", s->s_name);
    return;

error:
    handle_py_error(x, "execfile %s", s->s_name);
    Py_XDECREF(pval);
}

/*--------------------------------------------------------------------------*/
// extra python methods

void py_assign(t_py* x, int argc, char* args)
{
    char* varname = NULL;
    PyObject* list = NULL;

    if (s != gensym(""))
        py_log(x, "s: %s", s->s_name);

    // first atom in argv must be a symbol
    if (argv->a_type != A_SYM) {
        py_error(x, "first atom must be a symbol!");
        goto error;

    } else {
        // strncpy_zero(varname, atom_getsym(argv)->s_name, 50);
        varname = atom_getsym(argv)->s_name;
        py_log(x, "varname: %s", varname);
    }

    if ((list = PyList_New(0)) == NULL) {
        py_error(x, "list == NULL");
        goto error;
    }

    // NOTE: n C it’s illegal to have a declaration as the first statement
    // after a label enclosing the whole subblock in a {} seems to work
    for (int i = 1; i < argc; i++) {
        switch ((argv + i)->a_type) {
        case A_FLOAT: {
            double c_float = atom_getfloat(argv + i);
            PyObject* p_float = PyFloat_FromDouble(c_float);
            if (p_float == NULL) {
                error("p_float == NULL");
                goto error;
            }
            PyList_Append(list, p_float);
            Py_DECREF(p_float);
            py_log(x, "%d: %f", i, atom_getfloat(argv + i));
            break;
        }
        case A_LONG: {
            PyObject* p_long = PyLong_FromLong(atom_getlong(argv + i));
            if (p_long == NULL) {
                py_error(x, "p_long == NULL");
                goto error;
            }
            PyList_Append(list, p_long);
            Py_DECREF(p_long);
            py_log(x, "%d: %ld", i, atom_getlong(argv + i));
            break;
        }
        case A_SYM: {
            PyObject* p_str = PyUnicode_FromString(
                atom_getsym(argv + i)->s_name);
            if (p_str == NULL) {
                py_error(x, "p_str == NULL");
                goto error;
            }
            PyList_Append(list, p_str);
            Py_DECREF(p_str);
            py_log(x, "%d: %s", i, atom_getsym(argv + i)->s_name);
            break;
        }
        default:
            py_log(x, "cannot process unknown type");
            break;
        }
    }

    if (PyList_Size(list) != argc - 1) {
        py_error(x, "PyList_Size(list) != argc - 1");
        goto error;
    } else {
        py_log(x, "length of list: %d", PyList_Size(list));
    }

    // finally, assign list to varname in object namespace
    py_log(x, "setting %s to list in namespace", varname);
    int res = PyDict_SetItemString(x->p_globals, varname, list);
    if (res != 0) {
        py_error(x, "assign varname to list failed");
        goto error;
    }
    // Py_XDECREF(list); // causes a crash
    return;

error:
    handle_py_error(x, "assign %s", s->s_name);
    Py_XDECREF(list);
}

void py_anything(t_py* x, int argc, char* args)
{
    char* py_argv = NULL;
    PyObject* pval = NULL;
    // PyObject* py_callable_str = NULL;
    PyObject* py_callable = NULL;
    PyObject* py_argslist = NULL; // python list
    PyObject* py_args = NULL;     // python tuple

    if (s == gensym("")) {
        py_error(x, "could not retrieve callable name", s->s_name);
        goto error;
    }

    py_callable = PyRun_String(s->s_name, Py_eval_input, x->p_globals,
                               x->p_globals);
    if (py_callable == NULL) {
        py_error(x, "could not evaluate '%s' as a python callable", s->s_name);
        goto error;
    }

    if ((py_argslist = PyList_New(0)) == NULL) {
        py_error(x, "could not create an empyt python list");
        goto error;
    }

    for (int i = 0; i < argc; i++) {
        switch ((argv + i)->a_type) {
        case A_FLOAT: {
            double c_float = atom_getfloat(argv + i);
            PyObject* p_float = PyFloat_FromDouble(c_float);
            if (p_float == NULL) {
                py_error(x, "p_float == NULL");
                goto error;
            }
            PyList_Append(py_argslist, p_float);
            Py_DECREF(p_float);
            py_log(x, "%d: %f", i, atom_getfloat(argv + i));
            break;
        }
        case A_LONG: {
            PyObject* p_long = PyLong_FromLong(atom_getlong(argv + i));
            if (p_long == NULL) {
                py_error(x, "p_long == NULL");
                goto error;
            }
            PyList_Append(py_argslist, p_long);
            Py_DECREF(p_long);
            py_log(x, "%d: %ld", i, atom_getlong(argv + i));
            break;
        }
        case A_SYM: {
            PyObject* p_str = PyUnicode_FromString(
                atom_getsym(argv + i)->s_name);
            if (p_str == NULL) {
                py_error(x, "p_str == NULL");
                goto error;
            }
            PyList_Append(py_argslist, p_str);
            Py_DECREF(p_str);
            py_log(x, "%d: %s", i, atom_getsym(argv + i)->s_name);
            break;
        }
        default:
            py_log(x, "cannot process unknown type");
            break;
        }
    }

    if (PyList_Size(py_argslist) != argc) {
        py_error(x, "PyList_Size(list) != argc");
        goto error;
    } else {
        py_log(x, "length of list: %d", PyList_Size(py_argslist));
    }

    // convert py_args to tuple
    py_args = PyList_AsTuple(py_argslist);
    if (py_args == NULL) {
        py_error(x, "unable to convert args list to tuple");
        goto error;
    }

    pval = PyObject_Call(py_callable, py_args, NULL);
    if (pval == NULL) {
        py_error(x, "could not retrieve result of call");
        goto error;
    }

    // handle ints and longs
    if (PyLong_Check(pval)) {
        long int_result = PyLong_AsLong(pval);
        outlet_int(x->p_outlet1, int_result);
    }

    // handle floats and doubles
    if (PyFloat_Check(pval)) {
        float float_result = (float)PyFloat_AsDouble(pval);
        outlet_float(x->p_outlet1, float_result);
    }

    // handle strings
    if (PyUnicode_Check(pval)) {
        const char* unicode_result = PyUnicode_AsUTF8(pval);
        outlet_anything(x->p_outlet1, gensym(unicode_result), 0, NIL);
    }

    // handle lists, tuples and sets
    if (PyList_Check(pval) || PyTuple_Check(pval) || PyAnySet_Check(pval)) {
        PyObject* iter = NULL;
        PyObject* item = NULL;
        int i = 0;

        t_atom atoms_static[PY_MAX_ATOMS];
        t_atom* atoms = NULL;
        int is_dynamic = 0;

        Py_ssize_t seq_size = PySequence_Length(pval);
        if (seq_size <= 0) {
            py_error(x,
                     "cannot convert python sequence with zero or less length "
                     "to atoms");
            goto error;
        }

        if ((iter = PyObject_GetIter(pval)) == NULL) {
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

        while ((item = PyIter_Next(iter)) != NULL) {
            if (PyLong_Check(item)) {
                long long_item = PyLong_AsLong(item);
                atom_setlong(atoms + i, long_item);
                py_log(x, "%d long: %ld\n", i, long_item);
                i++;
            }

            if PyFloat_Check (item) {
                float float_item = PyFloat_AsDouble(item);
                atom_setfloat(atoms + i, float_item);
                py_log(x, "%d float: %f\n", i, float_item);
                i++;
            }

            if PyUnicode_Check (item) {
                const char* unicode_item = PyUnicode_AsUTF8(item);
                py_log(x, "%d unicode: %s\n", i, unicode_item);
                atom_setsym(atoms + i, gensym(unicode_item));
                i++;
            }
            Py_DECREF(item);
        }
        outlet_list(x->p_outlet1, NULL, i, atoms);
        py_log(x, "end iter op: %d", i);

        if (is_dynamic) {
            py_log(x, "restoring to static atom array");
            atom_dynamic_end(atoms_static, atoms);
        }
    }

    // success cleanup
    Py_XDECREF(py_callable);
    Py_XDECREF(py_argslist);
    Py_XDECREF(pval);
    py_log(x, "END %s: %s", s->s_name, py_argv);
    return;

error:
    handle_py_error(x, "anythinh %s", s->s_name);
    // cleanup
    Py_XDECREF(py_callable);
    Py_XDECREF(py_argslist);
    Py_XDECREF(pval);
}
