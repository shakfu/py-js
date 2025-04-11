/*--------------------------------------------------------------------------*/
/* Includes */

#include "pyjs.h"

/*--------------------------------------------------------------------------*/
/* Datastructures */

struct t_pyjs {
    /* object header */
    t_object p_ob;             /*!< object header */
    /* python-related */
    PyObject* p_globals;       /*!< per object 'globals' python namespace */
    t_symbol* p_name;          /*!< unique object name */
    t_symbol* p_pythonpath;    /*!< path to python directory */
    t_symbol* p_code_filepath; /*!< python filepath */
    t_bool p_debug;            /*!< bool to switch per-object debug state */
};

/*--------------------------------------------------------------------------*/
/* Globals */

static t_class* pyjs_class;
static int pyjs_global_obj_count; /*!< when 0 then free interpreter */

/*--------------------------------------------------------------------------*/
/* External main */

void ext_main(void* module_ref)
{
    t_class* c;

    c = class_new("pyjs", (method)pyjs_new, (method)pyjs_free,
                  (long)sizeof(t_pyjs), 0L /* leave NULL!! */,    A_GIMME, 0);

    /* methods */
    class_addmethod(c, (method)pyjs_import,       "import",       A_SYM, 0);
    class_addmethod(c, (method)pyjs_eval,         "eval",         A_GIMMEBACK, 0);
    class_addmethod(c, (method)pyjs_exec,         "exec",         A_SYM, 0);
    class_addmethod(c, (method)pyjs_execfile,     "execfile",     A_SYM, 0);
    class_addmethod(c, (method)pyjs_code,         "code",         A_GIMMEBACK, 0);
    class_addmethod(c, (method)pyjs_eval_to_json, "eval_to_json", A_GIMMEBACK, 0);

    /* attributes */
    CLASS_ATTR_SYM(c, "name",       0, t_pyjs, p_name);
    CLASS_ATTR_CHAR(c, "debug",     0, t_pyjs, p_debug);
    CLASS_ATTR_SYM(c, "file",       0, t_pyjs, p_code_filepath);
    CLASS_ATTR_SYM(c, "pythonpath", 0, t_pyjs, p_pythonpath);

    /* activate for javascript wrapping */
    c->c_flags = CLASS_FLAG_POLYGLOT;
    class_register(CLASS_NOBOX, c);
    pyjs_class = c;
}

/*--------------------------------------------------------------------------*/
/* Object init and freeing */

/**
 * @brief Create new external object with optional arguments.
 *
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return void*
 */
void* pyjs_new(t_symbol* s, long argc, t_atom* argv)
{
    t_pyjs* x = NULL;

    /* object instantiation, NEW STYLE */
    if ((x = (t_pyjs*)object_alloc(pyjs_class))) {
        /* Initialize values */

        if (pyjs_global_obj_count == 0) {
            /* first py obj is called '__main__' */
            x->p_name = gensym("__main__");
        } else {
            x->p_name = symbol_unique();
        }

        x->p_pythonpath = gensym("");
        x->p_debug = 1;
        x->p_code_filepath = gensym("");

        /* process @arg attributes */
        attr_args_process(x, argc, argv);

        /* python init */
        pyjs_init(x);
    }
    return (x);
}

/**
 * @brief Free object memory when object is deleted.
 *
 * @param x pointer to object struct.
 */
void pyjs_free(t_pyjs* x)
{
    Py_XDECREF(x->p_globals);
    pyjs_log(x, "will be deleted");

    /* crashes if one attempts to free.
     * #if defined(__APPLE__) && (defined(PY_STATIC_EXT) ||
     * defined(PY_SHARED_PKG)) CFRelease(py_global_bundle); #endif
     */
    pyjs_global_obj_count--;
    if (pyjs_global_obj_count == 0) {
        Py_FinalizeEx(); // or Py_Finalize()
    }
}

/**
 * @brief Initialize python builtins
 *
 * @param x pointer to object struct
 *
 * Collects python builtin initialization steps. Meant to be called in
 * `pyjs_init` which itself should be called inside `pyjs_new`.
 */
void pyjs_init_builtins(t_pyjs* x)
{
    PyObject* p_name = NULL;
    PyObject* builtins = NULL;
    int err = -1;

    p_name = PyUnicode_FromString(x->p_name->s_name);
    if (p_name == NULL) {
        goto error;
    }

    builtins = PyEval_GetBuiltins();
    if (builtins == NULL) {
        goto error;
    }

    err = PyDict_SetItemString(builtins, "PY_OBJ_NAME", p_name);
    if (err == -1) {
        goto error;
    }

    err = PyDict_SetItemString(x->p_globals, "__builtins__", builtins);
    if (err == -1) {
        goto error;
    }

    Py_XDECREF(p_name);
    return;

error:
    pyjs_handle_error(x, "could not update object namespace with object name");
    Py_XDECREF(p_name);
}



/**
 * @brief      Return path to external with optional subpath
 *
 * @param      c        t_class instance
 * @param      subpath  The subpath or NULL (if not)
 *
 * @return     path to external + (optional subpath)
 */
t_string* pyjs_get_path_to_external(t_class* c, char* subpath)
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
    snprintf_zero(external_name, MAX_FILENAME_CHARS, ext_filename, c->c_sym->s_name);
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
t_string* pyjs_get_path_to_package(t_class* c, char* subpath)
{
    char _dummy[MAX_PATH_CHARS];
    char externals_folder[MAX_PATH_CHARS];
    char package_folder[MAX_PATH_CHARS];

    t_string* result;
    t_string* external_path = pyjs_get_path_to_external(c, NULL);

    const char* ext_path_c = string_getptr(external_path);

    path_splitnames(ext_path_c, externals_folder, _dummy);     // ignore filename
    path_splitnames(externals_folder, package_folder, _dummy); // ignore filename

    result = string_new((char*)package_folder);

    if (subpath != NULL) {
        string_append(result, subpath);
    }

    return result;
}

/**
 * @brief main init function called within body of `pyjs_new`
 *
 * @param x pointer to object struct
 */
void pyjs_init(t_pyjs* x)
{
    wchar_t* python_home = NULL;

#if defined(__APPLE__) && defined(PY_STATIC_EXT)
    const char* resources_path = string_getptr(
        pyjs_get_path_to_external(pyjs_class, "/Contents/Resources"));
    python_home = Py_DecodeLocale(resources_path, NULL);
#endif

#if defined(__APPLE__) && defined(PY_SHARED_PKG)
    const char* package_path = string_getptr(
        pyjs_get_path_to_package(pyjs_class, "/support/python" PY_VER));
    python_home = Py_DecodeLocale(package_path, NULL);
#endif

#if defined(PY_37)
    if (python_home != NULL) {
        Py_SetPythonHome(python_home);
        PyMem_RawFree(python_home);
    }
    Py_Initialize();
#else
    PyStatus status;

    PyConfig config;
    PyConfig_InitPythonConfig(&config);
    // Disable parsing command line arguments
    config.parse_argv = 0;
    config.home = python_home;

    status = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(status)) {
        PyConfig_Clear(&config);
        error("could not initialize python");
    }

    PyConfig_Clear(&config);
#endif

    /* python init */
    PyObject* main_mod = PyImport_AddModule(x->p_name->s_name); /* borrowed reference */
    x->p_globals = PyModule_GetDict(main_mod); /* borrowed reference */
    pyjs_init_builtins(x); /* does this have to be a separate function? */

    /* increment global object counter */
    pyjs_global_obj_count++;
}

/*--------------------------------------------------------------------------*/
/* Helpers */

/**
 * @brief Post msg to Max console.
 *
 * @param x pointer to object struct
 * @param fmt character string with format codes
 * @param ... other arguments
 *
 * This log function is a variadic function which doesn'y `post` its msg
 * if the object struct member `x->p_debug` is 0.
 *
 * WARNING: if PY_MAX_ELEMS is less than
 * the length of the log or err message, Max will crash.
 */
void pyjs_log(t_pyjs* x, char* fmt, ...)
{
    if (x->p_debug) {
        char msg[PY_MAX_ELEMS];

        va_list va;
        va_start(va, fmt);
        vsnprintf(msg, PY_MAX_ELEMS, fmt, va);
        va_end(va);

        post("[pyjs %s]: %s", x->p_name->s_name, msg);
    }
}

/**
 * @brief Post error message to Max console.
 *
 * @param x pointer to object struct
 * @param fmt character string with format codes
 * @param ... other arguments
 */
void pyjs_error(t_pyjs* x, char* fmt, ...)
{
    char msg[PY_MAX_ELEMS];

    va_list va;
    va_start(va, fmt);
    vsnprintf(msg, PY_MAX_ELEMS, fmt, va);
    va_end(va);

    error("[pyjs %s]: %s", x->p_name->s_name, msg);
}

/**
 * @brief Searches the Max filesystem context for a file given by a symbol
 *
 * @param x pointer to object struct
 * @param s symbol to be search
 *
 * If successful, this function will set `x->p_code_filepath` with
 * the Max readable path of the found file.
 */
void pyjs_locate_path_from_symbol(t_pyjs* x, t_symbol* s)
{
    t_fourcc p_code_filetype = FOUR_CHAR_CODE('TEXT');
    t_fourcc p_code_outtype = 0;
    char p_code_filename[MAX_PATH_CHARS];
    char p_code_pathname[MAX_PATH_CHARS];
    short p_code_path;
    t_max_err err;

    if (s == gensym("")) { /* if no arg supplied ask for file */
        p_code_filename[0] = 0;

        if (open_dialog(p_code_filename, &p_code_path, &p_code_outtype, &p_code_filetype, 1)) {
                /* non-zero: cancelled */
                return;
            }
    } else {
        /* must copy symbol before calling locatefile_extended */
        strncpy_zero(p_code_filename, s->s_name, MAX_PATH_CHARS);
        if (locatefile_extended(p_code_filename, &p_code_path, &p_code_outtype,
                                &p_code_filetype, 1)) {
            /* nozero: not found */
            pyjs_error(x, "can't find file %s", s->s_name);
            return;
        }

        p_code_pathname[0] = 0;
        err = path_toabsolutesystempath(p_code_path, p_code_filename,
                                        p_code_pathname);
        if (err != MAX_ERR_NONE) {
            pyjs_error(x, "can't convert %s to absolutepath", s->s_name);
            return;
        }


        /* sucess: set attribute from pathname symbol */
        x->p_code_filepath = gensym(p_code_pathname);
    }
}


/*--------------------------------------------------------------------------*/
/* Handlers */

/**
 * @brief Generic python error handler
 *
 * @param x pointer to object struct
 * @param fmt format string
 * @param ... other args
 */
void pyjs_handle_error(t_pyjs* x, char* fmt, ...)
{
    if (PyErr_Occurred()) {

        /* build custom msg */
        char msg[PY_MAX_ELEMS];

        va_list va;
        va_start(va, fmt);
        vsnprintf(msg, PY_MAX_ELEMS, fmt, va);
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

        error("[pyjs %s] %s: %s", x->p_name->s_name, msg, pvalue_str);
    }
}


/**
 * @brief      Handler to output python float as max float
 *
 * @param      x       pointer to object struct
 * @param      pfloat  python float object
 * @param[out] rv      atom vector to populate in-place
 *
 * @return     The t_max_err error.
 */
t_max_err pyjs_handle_float_output(t_pyjs* x, PyObject* pfloat, t_atom* rv)
{
    t_atom atom_result[1];

    if (pfloat == NULL) {
        goto error;
    }

    if (PyFloat_Check(pfloat)) {
        float float_result = (float)PyFloat_AsDouble(pfloat);
        if (float_result == -1.0) {
            if (PyErr_Occurred()) {
                goto error;
            }
        }
        atom_setfloat(atom_result, float_result);
        atom_setobj(
            rv,
            object_new(gensym("nobox"), gensym("atomarray"), 1, atom_result));
    }
    Py_XDECREF(pfloat);
    return MAX_ERR_NONE;

error:
    pyjs_handle_error(x, "pyjs_handle_float_output failed");
    Py_XDECREF(pfloat);
    return MAX_ERR_GENERIC;
}

/**
 * @brief      Handler to output python long as max int
 *
 * @param      x      pointer to object struct
 * @param      plong  python long object
 * @param      rv     atom vector to populate in-place
 *
 * @return     The t_max_err error.
 */
t_max_err pyjs_handle_long_output(t_pyjs* x, PyObject* plong, t_atom* rv)
{
    t_atom atom_result[1];

    if (plong == NULL) {
        goto error;
    }

    if (PyLong_Check(plong)) {
        long long_result = PyLong_AsLong(plong);
        if (long_result == -1) {
            if (PyErr_Occurred()) {
                goto error;
            }
        }
        atom_setlong(atom_result, long_result);
        atom_setobj(
            rv,
            object_new(gensym("nobox"), gensym("atomarray"), 1, atom_result));
    }
    Py_XDECREF(plong);
    return MAX_ERR_NONE;

error:
    pyjs_handle_error(x, "pyjs_handle_long_output failed");
    Py_XDECREF(plong);
    return MAX_ERR_GENERIC;
}

/**
 * @brief      Handler to output python string as max symbol
 *
 * @param      x        pointer to object struct
 * @param      pstring  python string object
 * @param      rv       atom vector to populate in-place
 *
 * @return     The t_max_err error.
 */
t_max_err pyjs_handle_string_output(t_pyjs* x, PyObject* pstring, t_atom* rv)
{
    t_atom atom_result[PY_MAX_ELEMS];

    if (pstring == NULL) {
        goto error;
    }

    if (PyUnicode_Check(pstring)) {
        const char* unicode_result = PyUnicode_AsUTF8(pstring);
        if (unicode_result == NULL) {
            goto error;
        }
        atom_setsym(atom_result, gensym(unicode_result));
        atom_setobj(
            rv,
            object_new(gensym("nobox"), gensym("atomarray"), 1, atom_result));
    }
    Py_XDECREF(pstring);
    return MAX_ERR_NONE;

error:
    pyjs_handle_error(x, "pyjs_handle_string_output failed");
    Py_XDECREF(pstring);
    return MAX_ERR_GENERIC;
}

/**
 * @brief      Handler to output python list as max list
 *
 * @param      x      pointer to object struct
 * @param      plist  python list object
 * @param      rv     atom vector to populate in-place
 *
 * @return     The t_max_err error.
 */
t_max_err pyjs_handle_list_output(t_pyjs* x, PyObject* plist, t_atom* rv)
{
    if (plist == NULL) {
        goto error;
    }

    if (PySequence_Check(plist) && !PyUnicode_Check(plist)
        && !PyBytes_Check(plist) && !PyByteArray_Check(plist)) {
        PyObject* iter = NULL;
        PyObject* item = NULL;
        int i = 0;

        t_atom atoms_static[PY_MAX_ELEMS];
        t_atom* atoms = NULL;
        int is_dynamic = 0;

        Py_ssize_t seq_size = PySequence_Length(plist);

        if (seq_size == 0) {
            pyjs_error(x, "cannot convert py list of length 0 to atoms");
            goto error;
        }

        if (seq_size > PY_MAX_ELEMS) {
            pyjs_log(x, "dynamically increasing size of atom array");
            atoms = atom_dynamic_start(atoms_static, PY_MAX_ELEMS,
                                       seq_size + 1);
            is_dynamic = 1;

        } else {
            atoms = atoms_static;
        }

        if ((iter = PyObject_GetIter(plist)) == NULL) {
            goto error;
        }

        while ((item = PyIter_Next(iter)) != NULL) {

            if (PyFloat_Check(item)) {
                float float_item = PyFloat_AsDouble(item);
                if (float_item == -1.0) {
                    if (PyErr_Occurred()) {
                        goto error;
                    }
                }
                atom_setfloat(atoms + i, float_item);
                pyjs_log(x, "%d float: %f\n", i, float_item);
                i++;
            }

            if (PyLong_Check(item)) {
                long long_item = PyLong_AsLong(item);
                if (long_item == -1) {
                    if (PyErr_Occurred()) {
                        goto error;
                    }
                }
                atom_setlong(atoms + i, long_item);
                pyjs_log(x, "%d long: %ld\n", i, long_item);
                i++;
            }

            // only for numpy int64 (not recognized by PyLong_Check)
            // if (PyNumber_Check(item)) {
            //     long long_item = PyLong_AsLong(item);
            //     if (long_item == -1) {
            //         if (PyErr_Occurred())
            //             goto error;
            //     }
            //     atom_setlong(atoms + i, long_item);
            //     pyjs_log(x, "%d long: %ld\n", i, long_item);
            //     i++;
            // }

            if (PyUnicode_Check(item)) {
                const char* unicode_item = PyUnicode_AsUTF8(item);
                if (unicode_item == NULL) {
                    goto error;
                }
                atom_setsym(atoms + i, gensym(unicode_item));
                pyjs_log(x, "%d unicode: %s\n", i, unicode_item);
                i++;
            }
            Py_DECREF(item);
        }

        atom_setobj(
            rv,
            object_new(gensym("nobox"), gensym("atomarray"), (long)i, atoms));
        pyjs_log(x, "end iter op: %d", i);
        if (is_dynamic) {
            pyjs_log(x, "restoring to static atom array");
            atom_dynamic_end(atoms_static, atoms);
        }
    }

    Py_XDECREF(plist);
    return MAX_ERR_NONE;

error:
    pyjs_handle_error(x, "pyjs_handle_list_output failed");
    Py_XDECREF(plist);
    return MAX_ERR_GENERIC;
}

/**
 * @brief      Handler to output python dict as max list
 *
 * @param      x      pointer to object struct
 * @param      pdict  python dict object
 * @param      rv     atom vector to populate in-place
 *
 * @return     The t_max_err error.
 */
t_max_err pyjs_handle_dict_output(t_pyjs* x, PyObject* pdict, t_atom* rv)
{
    PyObject* pfun_co = NULL;
    PyObject* pfun = NULL;
    PyObject* pval = NULL;

    if (pdict == NULL) {
        goto error;
    }

    if (PyDict_Check(pdict)) {

        pfun_co = PyRun_String("def __py_maxmsp_out_dict(arg):\n"
                               "\tres = []\n"
                               "\tfor k,v in arg.items():\n"
                               "\t\tres.append(k)\n"
                               "\t\tres.append(':')\n"
                               "\t\tif type(v) in [list, set, tuple]:\n"
                               "\t\t\tfor i in v:\n"
                               "\t\t\t\tres.append(i)\n"
                               "\t\telse:\n"
                               "\t\t\tres.append(v)\n"
                               "\treturn res\n",
                               Py_single_input, x->p_globals, x->p_globals);

        if (pfun_co == NULL) {
            pyjs_error(x, "out_dict function code object is NULL");
            goto error;
        }

        pfun = PyDict_GetItemString(x->p_globals, "__py_maxmsp_out_dict");
        if (pfun == NULL) {
            pyjs_error(x, "retrieving out_dict func from globals failed");
            goto error;
        }

        pval = PyObject_CallFunctionObjArgs(pfun, pdict, NULL);
        if (pval == NULL) {
            pyjs_error(x, "out_dict call failed to retrieve result");
            goto error;
        }

        if (PyList_Check(pval)) { // expecting a python list
            Py_XDECREF(pfun_co);
            return pyjs_handle_list_output(x, pval, rv); // this decrefs pval
        } else {
            pyjs_error(x, "expected list output got something else");
            goto error;
        }
    }

error:
    pyjs_handle_error(x, "pyjs_handle_dict_output failed");
    Py_XDECREF(pfun_co);
    Py_XDECREF(pval);
    return MAX_ERR_GENERIC;
}

/**
 * @brief      Generic handler to output arbitrarily-typed python object as max object
 *
 * @param      x     pointer to object struct
 * @param      pval  python value object
 * @param      rv    atom vector to populate in-place
 *
 * @return     The t_max_err error.
 */
t_max_err pyjs_handle_output(t_pyjs* x, PyObject* pval, t_atom* rv)
{
    if (pval == NULL) {
        pyjs_error(x, "cannot handle NULL value");
        return MAX_ERR_GENERIC;
    }

    if (PyFloat_Check(pval)) {
        return pyjs_handle_float_output(x, pval, rv);
    }

    if (PyLong_Check(pval)) {
        return pyjs_handle_long_output(x, pval, rv);
    }

    if (PyUnicode_Check(pval)) {
        return pyjs_handle_string_output(x, pval, rv);
    }

    if (PySequence_Check(pval) && !PyBytes_Check(pval)
             && !PyByteArray_Check(pval)) {
        return pyjs_handle_list_output(x, pval, rv);
    }

    if (PyDict_Check(pval)) {
        return pyjs_handle_dict_output(x, pval, rv);
    }

    if (pval == Py_None) {
        return MAX_ERR_NONE;
    }

    else {
        pyjs_error(x, "cannot handle this type of value");
        return MAX_ERR_GENERIC;
    }
}

/*--------------------------------------------------------------------------*/
/* Core Methods */

/**
 * @brief      Converts atom vector to text and evaluate as python code
 *
 * @param      x     pointer to object struct
 * @param      s     symbol
 * @param[in]  argc  atom argument count
 * @param      argv  atom argument vector
 * @param      rv    atom vector to populate in-place
 *
 * @return     The t_max_err error.
 */
t_max_err pyjs_code(t_pyjs* x, t_symbol* s, long argc, t_atom* argv,
                    t_atom* rv)
{
    long textsize = 0;
    char* text = NULL;
    PyObject* co = NULL;
    PyObject* pval = NULL;
    t_max_err err;
    int is_eval = 1;

    err = atom_gettext(argc, argv, &textsize, &text,
                       OBEX_UTIL_ATOM_GETTEXT_DEFAULT);
    if (err == MAX_ERR_NONE && textsize && text) {
        pyjs_log(x, ">>> %s", text);
    } else {
        goto error;
    }

    co = Py_CompileString(text, x->p_name->s_name, Py_eval_input);

    if (PyErr_ExceptionMatches(PyExc_SyntaxError)) {
        PyErr_Clear();
        co = Py_CompileString(text, x->p_name->s_name, Py_single_input);
        is_eval = 0;
    }
    pyjs_log(x, "code is_eval: %d", is_eval);

    if (co == NULL) { // can be eval-co or exec-co or NULL here
        goto error;
    }
    sysmem_freeptr(text);

    pval = PyEval_EvalCode(co, x->p_globals, x->p_globals);
    if (pval == NULL) {
        goto error;
    }
    Py_DECREF(co);

    if (is_eval) {
        pyjs_handle_output(x, pval, rv);
    } else {
        Py_XDECREF(pval);
    }
    return MAX_ERR_NONE;

error:
    pyjs_handle_error(x, "pyjs code failed");
    Py_XDECREF(pval);
    return MAX_ERR_GENERIC;
}

/**
 * @brief      Import a python module
 *
 * @param      x     pointer to object struct
 * @param      s     symbol of module to be imported
 *
 * @return     The t_max_err error.
 */
t_max_err pyjs_import(t_pyjs* x, t_symbol* s)
{
    PyObject* x_module = NULL;

    if (s != gensym("")) {
        x_module = PyImport_ImportModule(s->s_name);

        if (x_module == NULL) {
            goto error;
        }

        PyDict_SetItemString(x->p_globals, s->s_name, x_module);
        pyjs_log(x, "imported: %s", s->s_name);
        return MAX_ERR_NONE;
    }

error:
    pyjs_handle_error(x, "import %s", s->s_name);
    return MAX_ERR_GENERIC;
}


/**
 * @brief      Evaluate a max symbol as a python expression
 *
 * @param      x     pointer to object struct
 * @param      s     symbol of object to be evaluated
 * @param[in]  argc  atom argument count
 * @param      argv  atom argument vector
 * @param      rv    atom vector to populate in-place
 *
 * @return     The t_max_err error.
 */
t_max_err pyjs_eval(t_pyjs* x, t_symbol* s, long argc, t_atom* argv,
                    t_atom* rv)
{
    char* py_argv = atom_getsym(argv)->s_name;
    pyjs_log(x, "%s %s", s->s_name, py_argv);

    PyObject* pval = PyRun_String(py_argv, Py_eval_input, x->p_globals,
                                  x->p_globals);

    if (pval != NULL) {
        pyjs_handle_output(x, pval, rv);
        return MAX_ERR_NONE;
    } else {
        pyjs_handle_error(x, "eval %s", py_argv);
        return MAX_ERR_GENERIC;
    }
}

/**
 * @brief      Execute contents of a file (obtained from symbol) as python code
 *
 * @param      x     pointer to object struct
 * @param      s     symbol of the file to executed.
 *
 * @return     The t_max_err error.
 */
t_max_err pyjs_execfile(t_pyjs* x, t_symbol* s)
{
    PyObject* pval = NULL;
    FILE* fhandle = NULL;

    if (s != gensym("")) {
        // set x->p_code_filepath
        pyjs_locate_path_from_symbol(x, s);
    }

    if (s == gensym("") || x->p_code_filepath == gensym("")) {
        pyjs_error(x, "could not set filepath");
        goto error;
    }

    // assume x->p_code_filepath has be been set without errors

    pyjs_log(x, "pathname: %s", x->p_code_filepath->s_name);
    fhandle = fopen(x->p_code_filepath->s_name, "r+");

    if (fhandle == NULL) {
        pyjs_error(x, "could not open file");
        goto error;
    }

    pval = PyRun_File(fhandle, x->p_code_filepath->s_name, Py_file_input,
                      x->p_globals, x->p_globals);
    if (pval == NULL) {
        fclose(fhandle);
        goto error;
    }

    // success cleanup
    fclose(fhandle);
    Py_DECREF(pval);
    return MAX_ERR_NONE;

error:
    pyjs_handle_error(x, "execfile failed");
    Py_XDECREF(pval);
    return MAX_ERR_GENERIC;
}

/**
 * @brief      Execute a max symbol as a line of python code
 *
 * @param      x     pointer to object struct
 * @param      s     symbol of the object to executed
 *
 * @return     The t_max_err error.
 */
t_max_err pyjs_exec(t_pyjs* x, t_symbol* s)
{
    PyObject* pval = NULL;

    if (s == gensym("")) {
        pyjs_log(x, "no input given");
        goto error;
    }

    pval = PyRun_String(s->s_name, Py_single_input, x->p_globals,
                        x->p_globals);
    if (pval == NULL) {
        goto error;
    }

    // success cleanup
    Py_DECREF(pval);
    pyjs_log(x, "exec %s", s->s_name);
    return MAX_ERR_NONE;

error:
    pyjs_handle_error(x, "exec %s", s->s_name);
    Py_XDECREF(pval);
    return MAX_ERR_GENERIC;
}

/**
 * @brief      Evaluates atom as python code and returns result as json
 *
 * @param      x     pointer to object struct
 * @param      s     symbol of object to be evaluated
 * @param[in]  argc  The count of arguments
 * @param      argv  The arguments array
 * @param      rv    atom vector to populate in-place
 *
 * @return     The t_max_err error.
 */
t_max_err pyjs_eval_to_json(t_pyjs* x, t_symbol* s, long argc, t_atom* argv,
                            t_atom* rv)
{
    t_atom atoms[PY_MAX_ELEMS];
    PyObject* pval = NULL;
    PyObject* json_module = NULL;
    PyObject* json_dict = NULL;
    PyObject* json_dumps = NULL;
    PyObject* json_pstr = NULL;

    char* cstring = atom_getsym(argv)->s_name;

    pval = PyRun_String(cstring, Py_eval_input, x->p_globals, x->p_globals);
    if (pval == NULL) {
        goto error;
    }

    json_module = PyImport_ImportModule("json");
    if (json_module == NULL) {
        goto error;
    }

    json_dict = PyModule_GetDict(json_module); // borrowed ref
    if (json_dict == NULL) {
        goto error;
    }

    json_dumps = PyDict_GetItemString(json_dict, "dumps"); // borrowed ref
    if (json_dumps == NULL) {
        goto error;
    }

    json_pstr = PyObject_CallFunctionObjArgs(json_dumps, pval, NULL);
    if (json_pstr == NULL) {
        goto error;
    }

    const char* unicode_result = PyUnicode_AsUTF8(json_pstr);
    if (unicode_result == NULL) {
        goto error;
    }

    atom_setsym(atoms, gensym(unicode_result));
    atom_setobj(rv,
                object_new(gensym("nobox"), gensym("atomarray"), 1, atoms));

    Py_XDECREF(pval);
    Py_XDECREF(json_module);
    Py_XDECREF(json_pstr);

    return MAX_ERR_NONE;

error:
    pyjs_handle_error(x, "pyjs_eval_to_json failed");
    Py_XDECREF(pval);
    Py_XDECREF(json_module);
    Py_XDECREF(json_pstr);
    return MAX_ERR_GENERIC;
}
