void py_eval(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    char* py_argv = NULL;
    PyObject* pval = NULL;
    PyObject* locals = NULL;

    py_argv = atom_getsym(argv)->s_name;
    if (py_argv == NULL) {
        error("%s: could not retrieve quoted argv", s->s_name);
        goto error;
    }
    post("START %s: %s", s->s_name, py_argv);

    locals = PyDict_New();
    if (locals == NULL) {
        goto error;
    }

    pval = PyRun_String(py_argv, Py_eval_input, x->p_globals, locals);
    if (pval == NULL) {
        goto error;
    }

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
    if (PyList_Check(pval) || PyTuple_Check(pval) || PyAnySet_Check(pval)) {
        PyObject* iter = NULL;
        PyObject* item = NULL;
        int i = 0;

        t_atom atoms_static[PY_MAX_ATOMS];
        t_atom* atoms = NULL;
        int is_dynamic = 0;

        Py_ssize_t seq_size = PySequence_Length(pval);
        if (seq_size <= 0) {
            error("cannot convert python sequence with zero or less length to "
                  "atoms");
            goto error;
        }

        if ((iter = PyObject_GetIter(pval)) == NULL) {
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
        outlet_list(x->p_outlet, NULL, i, atoms);
        post("end iter op: %d", i);

        if (is_dynamic) {
            post("restoring to static atom array");
            atom_dynamic_end(atoms_static, atoms);
        }
    }

    // success cleanup
    Py_XDECREF(pval);
    // Py_XDECREF(locals);
    post("END %s: %s", s->s_name, py_argv);

error:
    if (PyErr_Occurred()) {
        PyObject *ptype, *pvalue, *ptraceback;
        PyErr_Fetch(&ptype, &pvalue, &ptraceback);

        // get error message
        const char* pStrErrorMessage = PyUnicode_AsUTF8(pvalue);
        error("PyException('%s %s'): %s", s->s_name, py_argv,
              pStrErrorMessage);
        Py_XDECREF(ptype);
        Py_XDECREF(pvalue);
        Py_XDECREF(ptraceback);
    }
    // cleanup
    Py_XDECREF(pval);
    // Py_XDECREF(locals);
}
