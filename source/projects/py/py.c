/* py.c */

/*--------------------------------------------------------------------------*/
/* Includes */

/* py external api */
#include "py.h"

/* max/msp api */
#include "api.h"

/*--------------------------------------------------------------------------*/
/* Globals */

t_class* py_class; // global pointer to object class

static int py_global_obj_count = 0; // when 0 then free interpreter

static t_hashtab* py_global_registry = NULL; // global object lookups

static uintptr_t py_global_obj_ref = 0;

/*--------------------------------------------------------------------------*/
/* Datastructures */


struct t_py {
    /* object header */
    t_object p_ob;              /*!< object header */

    /* object attributes */
    t_symbol* p_name;           /*!< unique object name */

    /* python-related */
    t_symbol* p_pythonpath;     /*!< path to python directory */
    t_bool p_debug;             /*!< bool to switch per-object debug state */
    PyObject* p_globals;        /*!< per object 'globals' python namespace */

    /* infrastructure objects */
    t_patcher* p_patcher;       /*!< to send msgs to objects */
    t_box* p_box;               /*!< the ui box of the py instance? */

    /* time-based ops */
    void* p_clock;              /*!< a clock in case of scheduled ops */
    t_atomarray* p_sched_atoms; /*!< atomarray for scheduled python function call */

    /* text editor attrs */
    t_object* p_code_editor;    /*!< code editor object */
    char** p_code;              /*!< handle to code buffer for code editor */
    long p_code_size;           /*!< length of code buffer */
    t_fourcc p_code_filetype;   /*!< filetype four char code of 'TEXT' */
    t_fourcc p_code_outtype;    /*!< savetype four char code of 'TEXT' */
    char p_code_filename[MAX_PATH_CHARS]; /*!< file name field */
    char p_code_pathname[MAX_PATH_CHARS]; /*!< file path field */
    short p_code_path;          /*!< short code for max file system */
    long p_run_on_save;         /*!< evaluate/run code in editor on save */
    long p_run_on_close;        /*!< evaluate/run code in editor on close */
    t_symbol* p_run_on;

    t_symbol* p_code_filepath;  /*!< default python filepath to load into
                                  the code editor and object 'globals'
                                  namespace */
    t_bool p_autoload;          /*!< bool to autoload of p_code_filepath  */

    /* outlet creation */
    void* p_outlet_right;       /*!< right outlet to bang success */
    void* p_outlet_middle;      /*!< middle outleet to bang error */
    void* p_outlet_left;        /*!< left outleet for msg output  */

};


/*--------------------------------------------------------------------------*/
/* External main */

/**
 * @brief Main external function / entrypoint.
 *
 * @param module_ref used to obtain metadata
 *
 * The sole parameter `module_ref` can be used to obtain a reference
 * to the bundle itself.
 *
 */
void ext_main(void* module_ref)
{
    t_class* c;

    c = class_new("py", (method)py_new, (method)py_free, (long)sizeof(t_py),
                  0L, A_GIMME, 0);

    // object methods
    //------------------------------------------------------------------------
    // clang-format off

    // testing
    class_addmethod(c, (method)py_bang,       "bang",                  0);
    class_addmethod(c, (method)py_info,       "info",                  0);

    // core
    class_addmethod(c, (method)py_import,     "import",     A_SYM,     0);
    class_addmethod(c, (method)py_eval,       "eval",       A_GIMME,   0);
    class_addmethod(c, (method)py_exec,       "exec",       A_GIMME,   0);
    class_addmethod(c, (method)py_execfile,   "execfile",   A_DEFSYM,  0);

    // core extra
    class_addmethod(c, (method)py_assign,     "assign",     A_GIMME,   0);
    class_addmethod(c, (method)py_call,       "call",       A_GIMME,   0);
    class_addmethod(c, (method)py_code,       "code",       A_GIMME,   0);
    class_addmethod(c, (method)py_pipe,       "pipe",       A_GIMME,   0);
    class_addmethod(c, (method)py_fold,       "fold",       A_GIMME,   0);
    class_addmethod(c, (method)py_shell,      "shell",      A_GIMME,   0);
    class_addmethod(c, (method)py_anything,   "anything",   A_GIMME,   0);

    // time-based
    class_addmethod(c, (method)py_sched,      "sched",      A_GIMME,   0);

    // meta
    class_addmethod(c, (method)py_assist,     "assist",     A_CANT,    0);
    class_addmethod(c, (method)py_count,      "count",      A_NOTHING, 0);

    // interobject
    class_addmethod(c, (method)py_scan,       "scan",       A_NOTHING, 0);
    class_addmethod(c, (method)py_send,       "send",       A_GIMME,   0);

    // code editor
    class_addmethod(c, (method)py_read,       "read",       A_DEFSYM,  0);
    class_addmethod(c, (method)py_dblclick,   "dblclick",   A_CANT,    0);
    class_addmethod(c, (method)py_edclose,    "edclose",    A_CANT,    0);
    class_addmethod(c, (method)py_edsave,     "edsave",     A_CANT,    0);
    class_addmethod(c, (method)py_load,       "load",       A_DEFSYM,  0);
    class_addmethod(c, (method)py_run,        "run",        A_NOTHING, 0);
    class_addmethod(c, (method)py_okclose,    "okclose",    A_CANT,    0);

    // experimental
    class_addmethod(c, (method)py_appendtodict,  "appendtodictionary",  A_CANT, 0);

    // class attributes
    //------------------------------------------------------------------------

    CLASS_ATTR_LABEL(c,     "name", 0,      "unique object id");
    CLASS_ATTR_SYM(c,       "name", 0,      t_py, p_name);
    CLASS_ATTR_BASIC(c,     "name", 0);

    CLASS_ATTR_LABEL(c,     "file", 0,     "default python script");
    CLASS_ATTR_SYM(c,       "file", 0,     t_py,  p_code_filepath);
    CLASS_ATTR_STYLE(c,     "file", 0,     "file");
    CLASS_ATTR_BASIC(c,     "file", 0);
    CLASS_ATTR_SAVE(c,      "file", 0);

    CLASS_ATTR_LABEL(c,     "autoload", 0,     "autoload default python script");
    CLASS_ATTR_LONG(c,      "autoload", 0,     t_py, p_autoload);
    CLASS_ATTR_STYLE(c,     "autoload", 0,     "onoff");
    CLASS_ATTR_BASIC(c,     "autoload", 0);
    CLASS_ATTR_SAVE(c,      "autoload", 0);

    CLASS_ATTR_LABEL(c,     "run_on_save", 0,  "run content of editor on save");
    CLASS_ATTR_LONG(c,      "run_on_save", 0,  t_py, p_run_on_save);
    CLASS_ATTR_STYLE(c,     "run_on_save", 0, "onoff");
    CLASS_ATTR_DEFAULT(c,   "run_on_save", 0, "0");
    CLASS_ATTR_BASIC(c,     "run_on_save", 0);
    CLASS_ATTR_SAVE(c,      "run_on_save", 0);

    CLASS_ATTR_LABEL(c,     "run_on_close", 0,  "run content of editor on close");
    CLASS_ATTR_LONG(c,      "run_on_close", 0,  t_py, p_run_on_close);
    CLASS_ATTR_STYLE(c,     "run_on_close", 0, "onoff");
    CLASS_ATTR_DEFAULT(c,   "run_on_close", 0, "1");
    CLASS_ATTR_BASIC(c,     "run_on_close", 0);
    CLASS_ATTR_SAVE(c,      "run_on_close", 0);

    CLASS_ATTR_LABEL(c,     "run_on", 0,  "set run_on to run on save or close");
    CLASS_ATTR_SYM(c,       "run_on", 0,  t_py, p_run_on);
    CLASS_ATTR_STYLE(c,     "run_on", 0,  "enum");
    CLASS_ATTR_ENUM(c,      "run_on", 0,  "close save");
    CLASS_ATTR_DEFAULT(c,   "run_on", 0,  "close");
    CLASS_ATTR_BASIC(c,     "run_on", 0);
    CLASS_ATTR_SAVE(c,      "run_on", 0);

    CLASS_ATTR_LABEL(c,     "pythonpath", 0,  "patch-wide pythonpath");
    CLASS_ATTR_SYM(c,       "pythonpath", 0,  t_py, p_pythonpath);
    // disabled since it provides a dialogue only for file instead of dir
    // CLASS_ATTR_STYLE(c,     "pythonpath", 0,  "file");
    CLASS_ATTR_BASIC(c,     "pythonpath", 0);
    CLASS_ATTR_SAVE(c,      "pythonpath", 0);

    CLASS_ATTR_LABEL(c,     "debug", 0,  "debug log to console");
    CLASS_ATTR_LONG(c,      "debug", 0,  t_py, p_debug);
    CLASS_ATTR_STYLE(c,     "debug", 0, "onoff");
    CLASS_ATTR_BASIC(c,     "debug", 0);
    CLASS_ATTR_SAVE(c,      "debug", 0);

    CLASS_ATTR_ORDER(c,     "name",         0,  "1");
    CLASS_ATTR_ORDER(c,     "file",         0,  "2");
    CLASS_ATTR_ORDER(c,     "autoload",     0,  "3");
    CLASS_ATTR_ORDER(c,     "run_on_save",  0,  "4");
    CLASS_ATTR_ORDER(c,     "run_on_close", 0,  "5");
    CLASS_ATTR_ORDER(c,     "run_on",       0,  "6");
    CLASS_ATTR_ORDER(c,     "pythonpath",   0,  "7");
    CLASS_ATTR_ORDER(c,     "debug",        0,  "8");

    // clang-format on
    //------------------------------------------------------------------------

    class_register(CLASS_BOX, c);

    /* for js registration (can't be both box and nobox) */
    // c->c_flags = CLASS_FLAG_POLYGLOT;
    // class_register(CLASS_NOBOX, c);

    py_class = c;
}

/*--------------------------------------------------------------------------*/
/* Object new, init and free */

/**
 * @brief Create new external object with optional arguments.
 *
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return void*
 */
void* py_new(t_symbol* s, long argc, t_atom* argv)
{
    t_py* x = NULL;

    x = (t_py*)object_alloc(py_class);

    if (x) {

        if (py_global_obj_count == 0) {
            // first py obj is called '__main__'
            x->p_name = gensym("__main__");
        } else {
            x->p_name = symbol_unique();
        }

        // communication
        x->p_patcher = NULL;
        x->p_box = NULL;

        // python-related
        x->p_pythonpath = gensym("");

        // text editor
        x->p_code = sysmem_newhandle(0);
        x->p_code_size = 0;
        x->p_code_editor = NULL;
        x->p_code_filetype = FOUR_CHAR_CODE('TEXT');
        x->p_code_outtype = 0;
        x->p_code_filename[0] = 0;
        x->p_code_pathname[0] = 0;
        // short p_code_path;
        x->p_code_filepath = gensym("");
        x->p_autoload = 0;
        x->p_run_on_save = 0;
        x->p_run_on_close = 1;
        x->p_run_on = gensym("close");

        // set default debug level
        x->p_debug = 0;

        // test tasks
        x->p_clock = clock_new((t_object*)x, (method)py_task);
        x->p_sched_atoms = NULL;

        // create outlet(s)
        x->p_outlet_right = bangout((t_object*)x);
        x->p_outlet_middle = bangout((t_object*)x);
        x->p_outlet_left = outlet_new(x, NULL);

        // process @arg attributes
        attr_args_process(x, argc, argv);

        object_obex_lookup(x, gensym("#P"), (t_patcher**)&x->p_patcher);
        if (x->p_patcher == NULL)
            error("patcher object not created.");

        object_obex_lookup(x, gensym("#B"), (t_box**)&x->p_box);
        if (x->p_box == NULL)
            error("patcher object not created.");

        // create scripting name
        t_max_err err = jbox_set_varname(x->p_box, x->p_name);
        if (err != MAX_ERR_NONE) {
            error("could not set scripting name");
        }

        // python init
        py_init(x);

        post("initialized python version: %s", PY_VERSION);

        py_log(x, "object created");
        for (int i = 0; i < argc; i++) {
            py_log(x, "%d: %s", i, atom_getsym(argv + i)->s_name);
            post("argc: %d  argv: %s", i, atom_getsym(argv + i)->s_name);
        }

        t_dictionary* dict = (t_dictionary*)gensym("#D")->s_thing;
        if (dict) {
            dictionary_getsym(dict, gensym("file"), &x->p_code_filepath);
            dictionary_getlong(dict, gensym("autoload"),
                               (t_atom_long*)&x->p_autoload);
            dictionary_getsym(dict, gensym("pythonpath"), &x->p_pythonpath);
        }

        // process autoload
        py_log(x, "checking autoload / code_filepath / pythonpath");
        py_log(x, "autoload: %d\ncode_filepath: %s\npythonpath: %s",
               x->p_autoload, x->p_code_filepath->s_name,
               x->p_pythonpath->s_name);
        py_log(x, "via object_attr_getsym: %s",
               object_attr_getsym(x, gensym("file"))->s_name);

        if ((x->p_autoload == 1) && (x->p_code_filepath != gensym(""))) {
            py_log(x, "autoloading: %s", x->p_code_filepath->s_name);
            py_load(x, x->p_code_filepath);
        }

        if (x->p_pythonpath != gensym("")) {
            PyObject* sys_path = PySys_GetObject((char*)"path");
            PyObject* py_path = PyUnicode_FromString(x->p_pythonpath->s_name);
            PyList_Append(sys_path, py_path);
        }
    }
    return (x);
}



/**
 * @brief main init function called within body of `py_new`
 *
 * @param x object instance
 */
void py_init(t_py* x)
{
    wchar_t* python_home = NULL;

    /* Add the cythonized 'api' built-in module, before Py_Initialize */
    if (PyImport_AppendInittab("api", PyInit_api) == -1) {
        py_error(x, "could not add api module to builtin modules table");
    }

#if defined(__APPLE__) && defined(PY_STATIC_EXT)
    const char* resources_path = string_getptr(
        py_get_path_to_external(py_class, "/Contents/Resources"));
    python_home = Py_DecodeLocale(resources_path, NULL);
#endif

#if defined(__APPLE__) && defined(PY_SHARED_PKG)
    const char* package_path = string_getptr(
        py_get_path_to_package(py_class, "/support/python" PY_VER));
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
    config.parse_argv = 0; // Disable parsing command line arguments
    config.home = python_home;

    status = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(status)) {
        PyConfig_Clear(&config);
        py_error(x, "could not initialize python");
    }

    PyConfig_Clear(&config);
#endif

    // python init
    PyObject* main_mod = PyImport_AddModule(x->p_name->s_name); // borrowed
    x->p_globals = PyModule_GetDict(main_mod); // borrowed reference
    py_init_builtins(x); // does this have to be a separate function?

    // register the object
    object_register(CLASS_BOX, x->p_name, x);

    // increment global object counter
    py_global_obj_count++;

    if (py_global_obj_count == 1) {
        // if first py object create the py_global_registry;
        py_global_registry = (t_hashtab*)hashtab_new(0);
        hashtab_flags(py_global_registry, OBJ_FLAG_REF);
    }

    // set object ref which can be accessed from api module
    py_global_obj_ref = (uintptr_t)x;
}


/**
 * @brief Free object memory when deleted.
 *
 * @param x pointer to object struct.
 */
void py_free(t_py* x)
{
    // code editor cleanup
    object_free(x->p_code_editor);
    object_free(x->p_clock);
    if (x->p_sched_atoms)
        object_free(x->p_sched_atoms);
    if (x->p_code)
        sysmem_freehandle(x->p_code);

    Py_XDECREF(x->p_globals);
    // python objects cleanup
    py_log(x, "will be deleted");
    py_global_obj_count--;
    if (py_global_obj_count == 0) {
        /* WARNING: don't call x here or max will crash */
        hashtab_chuck(py_global_registry);

        post("last py obj freed -> finalizing py mem / interpreter.");
        // Py_FinalizeEx();
        Py_Finalize();
    }
}


/*--------------------------------------------------------------------------*/
/* Helpers */

/**
 * @brief Get the outlet object
 *
 * @param x pointer to object struct
 * @return void*
 *
 * Returns a reference to the main object outlet
 */
void* get_outlet(t_py* x)
{
    return (void*)x->p_outlet_left;
}

/**
 * @brief Post msg to Max console.
 *
 * @param x pointer to object struct
 * @param fmt character string with format codes
 * @param ... other arguments
 *
 * This log function is a variadic function which does not `post` its message
 * if the object struct member `x->p_debug` is 0.
 *
 * WARNING: if PY_MAX_ELEMS is less than
 * the length of the log or err message, Max will crash.
 */
void py_log(t_py* x, char* fmt, ...)
{
    if (x->p_debug) {
        char msg[PY_MAX_ELEMS];

        va_list va;
        va_start(va, fmt);
        vsnprintf(msg, PY_MAX_ELEMS, fmt, va);
        va_end(va);

        post("[py %s]: %s", x->p_name->s_name, msg);
    }
}

/**
 * @brief Post error message to Max console.
 *
 * @param x pointer to object struct
 * @param fmt character string with format codes
 * @param ... other arguments
 */
void py_error(t_py* x, char* fmt, ...)
{
    char msg[PY_MAX_ELEMS];

    va_list va;
    va_start(va, fmt);
    vsnprintf(msg, PY_MAX_ELEMS, fmt, va);
    va_end(va);

    error("[py %s]: %s", x->p_name->s_name, msg);
}

/**
 * @brief Initialize python builtins
 *
 * @param x pointer to object struct
 *
 * Collects python builtin initialization steps. Meant to be called in
 * `py_init` which itself should be called inside `py_new`.
 */
void py_init_builtins(t_py* x)
{
    PyObject* p_name = NULL;
    PyObject* builtins = NULL;
    PyObject* p_code_obj = NULL;
    int err = -1;

    p_name = PyUnicode_FromString(x->p_name->s_name);
    if (p_name == NULL)
        goto error;

    builtins = PyEval_GetBuiltins();
    if (builtins == NULL)
        goto error;

    err = PyDict_SetItemString(builtins, "PY_OBJ_NAME", p_name);
    if (err == -1)
        goto error;

    err = PyDict_SetItemString(x->p_globals, "__builtins__", builtins);
    if (err == -1)
        goto error;

    p_code_obj = PyRun_String(PY_PRELUDE_MODULE,
        Py_file_input, x->p_globals, x->p_globals);

    if (p_code_obj == NULL) {
        py_error(x, "cannot import PY_PRELUDE_MODULE");
        goto error;
    }

    Py_XDECREF(p_name);
    Py_XDECREF(p_code_obj);
    return;

error:
    py_handle_error(x, "could not update object namespace with object name");
    Py_XDECREF(p_name);
}


/**
 * @brief Get the global registry object
 *
 * @return t_hashtab*
 *
 * This is only used in the api module
 */
t_hashtab* py_get_global_registry(void)
{
    return py_global_registry;
}

/**
 * @brief      Return a ref the t_py *x pointer via the global_ref
 *
 * @return     unitptr ref to the object struct
 * 
 * This is only used in the api module
 */
uintptr_t py_get_object_ref(void) {
    return py_global_obj_ref;
}


/**
 * @brief      Return path to external with optional subpath
 *
 * @param      c        t_class instance
 * @param      subpath  The subpath or NULL (if not)
 *
 * @return     path to external + (optional subpath)
 */
t_string* py_get_path_to_external(t_class* c, char* subpath)
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
    path_nameconform(external_path, conform_path, PATH_STYLE_MAX,
                     PATH_TYPE_BOOT);
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
t_string* py_get_path_to_package(t_class* c, char* subpath)
{
    char _dummy[MAX_PATH_CHARS];
    char externals_folder[MAX_PATH_CHARS];
    char package_folder[MAX_PATH_CHARS];

    t_string* result;
    t_string* external_path = py_get_path_to_external(c, NULL);

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
 * @brief Searches the Max filesystem context for a file given by a symbol
 *
 * @param x pointer to object struct
 * @param s symbol to be searched
 * @return t_max_err
 *
 * If successful, this function will set `x->p_code_filepath` with
 * the Max readable path of the found file.
 */
t_max_err py_locate_path_from_symbol(t_py* x, t_symbol* s)
{
    t_max_err ret = MAX_ERR_NONE;

    if (s == gensym("")) {
        x->p_code_filename[0] = 0;

        if (open_dialog(x->p_code_filename, &x->p_code_path,
                        &x->p_code_outtype, &x->p_code_filetype, 1))
            /* non-zero: cancelled */
            ret = MAX_ERR_GENERIC;
            goto finally;

    } else {

        strncpy_zero(x->p_code_filename, s->s_name, MAX_PATH_CHARS);

        if (locatefile_extended(x->p_code_filename, &x->p_code_path,
                                &x->p_code_outtype, &x->p_code_filetype, 1)) {
            // nozero: not found
            py_error(x, "can't find file %s", s->s_name);
            ret = MAX_ERR_GENERIC;
            goto finally;
        } else {
            x->p_code_pathname[0] = 0;
            ret = path_toabsolutesystempath(x->p_code_path, x->p_code_filename,
                                            x->p_code_pathname);
            if (ret != MAX_ERR_NONE) {
                py_error(x, "can't convert %s to absolutepath", s->s_name);
                goto finally;
            }
        }

        // success
        // set attribute from pathname symbol
        x->p_code_filepath = gensym(x->p_code_pathname);
        assert(ret == MAX_ERR_NONE);
    }

finally:
    return ret;
}


/**
 * @brief Update the dict with the filepath and autoload option.
 *
 * @param x pointer to object struct
 * @param dict pointer to dict instance
 */
void py_appendtodict(t_py* x, t_dictionary* dict)
{
    if (dict) {
        dictionary_appendsym(dict, gensym("file"), x->p_code_filepath);
        dictionary_appendlong(dict, gensym("autoload"), x->p_autoload);
    }
}


/**
 * @brief      replace a string with a string
 *
 * @param      orig  The original
 * @param      rep   what to replace
 * @param      with  what to replace with
 *
 * @return     a new string with replaced strings
 * 
 * NOTE: You must free the result if result is non-NULL.
 * 
 */
char *str_replace(char *orig, char *rep, char *with) {
    // from: https://stackoverflow.com/questions/779875
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = (int)strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = (int)strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}


/*--------------------------------------------------------------------------*/
/* Documentation */

/**
 * @brief Sets tool tips for external object inlets.
 *
 * @param x object instance
 * @param b notused (historical)
 * @param m type
 * @param a position
 * @param s name
 */
void py_assist(t_py* x, void* b, long m, long a, char* s)
{
    if (m == ASSIST_INLET) { // inlet
        snprintf_zero(s, PY_MAX_ELEMS, "I am inlet %ld", a);
    } else { // outlet
        snprintf_zero(s, PY_MAX_ELEMS, "I am outlet %ld", a);
    }
}


/**
 * @brief Output global object count.
 *
 * @param x pointer to object struct.
 */
void py_count(t_py* x)
{
    outlet_int(x->p_outlet_left, py_global_obj_count);
}


/**
 * @brief      join parent path to child subpath
 *
 * @param[out] destination  output destination path
 * @param[in]  path1        parent path
 * @param[in]  path2        child subpath
 */
void path_join(char* destination, const char* path1, const char* path2)
{
    //char filename[MAX_FILENAME_CHARS];
    //strncpy_zero(filename,str->s_name, MAX_FILENAME_CHARS);

    if(path1 == NULL && path2 == NULL) {
        strcpy(destination, "");
    }
    else if(path2 == NULL || strlen(path2) == 0) {
        strcpy(destination, path1);
    }
    else if(path1 == NULL || strlen(path1) == 0) {
        strcpy(destination, path2);
    }
    else {
        char directory_separator[] = "/";
#ifdef WIN32
        directory_separator[0] = '\\';
#endif
        const char *last_char = path1;
        while(*last_char != '\0')
            last_char++;
        int append_directory_separator = 0;
        if(strcmp(last_char, directory_separator) != 0) {
            append_directory_separator = 1;
        }
        strcpy(destination, path1);
        if(append_directory_separator)
            strcat(destination, directory_separator);
        strcat(destination, path2);
    }
}


/**
 * @brief      Displays info about the external
 *
 * @param      x     pointer to object struct.
 */
void py_info(t_py* x)
{
    char output_path[MAX_PATH_CHARS];

    short supportpath_id = path_getsupportpath();
    short tempfolder_id = path_tempfolder();
    short desktopfolder_id = path_desktopfolder();
    short userdocfolder_id = path_userdocfolder();
    short usermaxfolder_id = path_usermaxfolder();
    short defaultpath_id = path_getdefault();

    path_toabsolutesystempath(supportpath_id, "", output_path);
    post("supportpath: %s", output_path);

    path_toabsolutesystempath(tempfolder_id, "", output_path);
    post("tempfolder: %s", output_path);

    path_toabsolutesystempath(desktopfolder_id, "", output_path);
    post("desktopfolder: %s", output_path);

    path_toabsolutesystempath(userdocfolder_id, "", output_path);
    post("userdocfolder: %s", output_path);

    path_toabsolutesystempath(usermaxfolder_id, "", output_path);
    post("usermaxfolder: %s", output_path);

    path_toabsolutesystempath(defaultpath_id, "", output_path);
    post("defaultpath: %s", output_path);

    const char * external_path = string_getptr(
        py_get_path_to_external(py_class, NULL));

    post("externalpath: %s", external_path);

    // test new path finding
    char* package_path[MAX_PATH_CHARS];
    char* package_externals_path[MAX_PATH_CHARS];
    char* external_name[MAX_PATH_CHARS];
    char* externals_folder[MAX_PATH_CHARS];

    path_splitnames(external_path, (char*)package_externals_path,
                    (char*)external_name);
    post("package_externals_path: %s", package_externals_path);
    post("external_name: %s", external_name);

    path_splitnames((char*)package_externals_path, (char*)package_path,
                    (char*)externals_folder);
    post("package_path: %s", package_path);
    post("externals_folder: %s", externals_folder);

    // package mode
    const char* support_python_path = "support/python" PY_VER;

    char external_contents_path[MAX_PATH_CHARS];
    char external_resources_path[MAX_PATH_CHARS];
    char python_path[MAX_PATH_CHARS];

    path_join(external_contents_path, external_path, "Contents");
    path_join(external_resources_path, external_contents_path, "Resources");
    path_join(python_path, (char*)package_path, support_python_path);

    post("external_resources_path: %s", external_resources_path);
    post("python_path: %s", python_path);

}

/*--------------------------------------------------------------------------*/
/* Side-effects */

/**
 * @brief Output bang from left outlet.
 *
 * @param x pointer to object struct.
 */
void py_bang(t_py* x)
{
    // just a passthrough: bang out the left outlet
    outlet_bang(x->p_outlet_left);
}

/**
 * @brief Output bang from right outlet.
 *
 * @param x pointer to object struct.
 */
void py_bang_success(t_py* x)
{
    outlet_bang(x->p_outlet_right);
}

/**
 * @brief Output bang from middle outlet.
 *
 * @param x pointer to object struct.
 */
void py_bang_failure(t_py* x)
{
    outlet_bang(x->p_outlet_middle);
}

/*--------------------------------------------------------------------------*/
/* Time-based */

/**
 * @brief Schedule a python function call
 *
 * @param x pointer to object struct
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return t_max_err error code
 */
t_max_err py_sched(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    t_max_err ret = 0;

    // schedule a python call
    // [sched <time> func arg1 arg2 ... argN]
    float time = 0.0;

    // first atom in argv must be a float
    if (argv->a_type != A_FLOAT) {
        py_error(x, "first atom must be a float!");
        goto error;
    }

    if (argc < 2) {
        py_error(x, "need at least 2 args to schedule function calls");
        goto error;
    }

    if ((argv + 0)->a_type != A_FLOAT) {
        py_error(x, "1st arg of sched needs to be a float time in ms");
        goto error;
    }

    // argv+0 is the object name to send to
    time = atom_getfloat(argv);
    if (time == 0.0) {
        goto error;
    }

    // atom after the name of the time
    if ((argv + 1)->a_type != A_SYM) {
        py_error(x, "2nd elem of sched atom needs to be the name of the callable");
        goto error;
    }

    // address the minimum case: e.g a bang
    argc = argc - 1;
    argv = argv + 1;

    // success
    // reset it
    if (x->p_sched_atoms != NULL) {
        object_free(x->p_sched_atoms);
        x->p_sched_atoms = NULL;
    }

    x->p_sched_atoms = atomarray_new(argc, argv);
    if (x->p_sched_atoms == NULL) {
        py_error(x, "atom not scheduled");
        goto error;
    }
    clock_fdelay(x->p_clock, time);
    ret = MAX_ERR_NONE;
    goto finally;

error:
    py_error(x, "send failed");
    ret = MAX_ERR_GENERIC;

finally:
    return ret;
}

/**
 * @brief Wraps a scheduled python function call.
 *
 * @param x pointer to object struct
 * @return t_max_err error code
 */
t_max_err py_task(t_py* x)
{
    double time;
    long argc = 0;
    t_atom* argv = NULL;

    clock_getftime(&time);
    // also scheduler_gettime(&time);
    t_max_err err = atomarray_getatoms(x->p_sched_atoms, &argc, &argv);
    if (err != MAX_ERR_NONE) {
        py_error(x, "atomarray arg initialization failed");
        return MAX_ERR_GENERIC;
    }
    py_log(x, "%lx instance is executing at time %.2f", x, time);
    py_call(x, gensym(""), argc, argv);
    py_bang_success(x);
    return MAX_ERR_NONE;
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
void py_handle_error(t_py* x, char* fmt, ...)
{
    if (PyErr_Occurred()) {

        // build custom msg
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

        error("[py %s] %s: %s", x->p_name->s_name, msg, pvalue_str);
    }
}

/**
 * @brief Handler to output python float as max float
 *
 * @param x pointer to object struct
 * @param pfloat python float
 * @return t_max_err error code
 */
t_max_err py_handle_float_output(t_py* x, PyObject* pfloat)
{
    if (pfloat == NULL) {
        goto error;
    }

    if (PyFloat_Check(pfloat)) {
        float float_result = (float)PyFloat_AsDouble(pfloat);
        if (float_result == -1.0 && PyErr_Occurred()) {
            goto error;
        }

        outlet_float(x->p_outlet_left, float_result);
        py_bang_success(x);
    }
    Py_XDECREF(pfloat);
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "py_handle_float_output failed");
    Py_XDECREF(pfloat);
    py_bang_failure(x);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Handler to output python long as max int
 *
 * @param x pointer to object struct
 * @param plong python long
 * @return t_max_err error code
 */
t_max_err py_handle_long_output(t_py* x, PyObject* plong)
{
    if (plong == NULL) {
        goto error;
    }

    if (PyLong_Check(plong)) {
        long long_result = PyLong_AsLong(plong);
        if (long_result == -1 && PyErr_Occurred()) {
            goto error;
        }

        outlet_int(x->p_outlet_left, long_result);
        py_bang_success(x);
    }

    Py_XDECREF(plong);
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "py_handle_long_output failed");
    Py_XDECREF(plong);
    py_bang_failure(x);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Handler to output python string as max symbol
 *
 * @param x pointer to object struct
 * @param pstring python string
 * @return t_max_err error code
 */
t_max_err py_handle_string_output(t_py* x, PyObject* pstring)
{
    if (pstring == NULL) {
        goto error;
    }

    if (PyUnicode_Check(pstring)) {
        const char* unicode_result = PyUnicode_AsUTF8(pstring);
        if (unicode_result == NULL) {
            goto error;
        }
        outlet_anything(x->p_outlet_left, gensym(unicode_result), 0, NIL);
        py_bang_success(x);
    }

    Py_XDECREF(pstring);
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "py_handle_string_output failed");
    Py_XDECREF(pstring);
    py_bang_failure(x);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Handler to output python list as max list
 *
 * @param x pointer to object struct
 * @param plist python list
 * @return t_max_err error code
 */
t_max_err py_handle_list_output(t_py* x, PyObject* plist)
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
        py_log(x, "seq_size: %d", seq_size);

        if (seq_size == 0) {
            py_error(x, "cannot convert py list of length 0 to atoms");
            goto error;
        }

        if (seq_size > PY_MAX_ELEMS) {
            py_log(x, "dynamically increasing size of atom array");
            atoms = atom_dynamic_start(atoms_static, PY_MAX_ELEMS,
                                       seq_size + 1);
            is_dynamic = 1;

        } else {
            atoms = atoms_static;
        }

        if ((iter = PyObject_GetIter(plist)) == NULL) {
            goto error;
        }
        py_log(x, "seq_size2: %d", seq_size);

        while ((item = PyIter_Next(iter)) != NULL) {
            if (PyLong_Check(item)) {
                long long_item = PyLong_AsLong(item);
                if (long_item == -1 && PyErr_Occurred()) {
                    goto error;
                }
                atom_setlong(atoms + i, long_item);
                py_log(x, "%d long: %ld\n", i, long_item);
                i++;
            }

            if (PyFloat_Check(item)) {
                float float_item = PyFloat_AsDouble(item);
                if (float_item == -1.0 && PyErr_Occurred()) {
                    goto error;
                }
                atom_setfloat(atoms + i, float_item);
                py_log(x, "%d float: %f\n", i, float_item);
                i++;
            }

            if (PyUnicode_Check(item)) {
                const char* unicode_item = PyUnicode_AsUTF8(item);
                if (unicode_item == NULL) {
                    goto error;
                }
                atom_setsym(atoms + i, gensym(unicode_item));
                py_log(x, "%d unicode: %s\n", i, unicode_item);
                i++;
            }
            Py_DECREF(item);
        }

        outlet_list(x->p_outlet_left, NULL, i, atoms);
        py_bang_success(x);
        py_log(x, "end iter op: %d", i);

        if (is_dynamic) {
            py_log(x, "restoring to static atom array");
            atom_dynamic_end(atoms_static, atoms);
        }
    }

    Py_XDECREF(plist);
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "py_handle_list_output failed");
    Py_XDECREF(plist);
    py_bang_failure(x);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Handler to output python dict as max list
 *
 * @param x pointer to object struct
 * @param pdict python dict
 * @return t_max_err error code
 */
t_max_err py_handle_dict_output(t_py* x, PyObject* pdict)
{
    PyObject* pfun = NULL;
    PyObject* pval = NULL;

    if (pdict == NULL) {
        goto error;
    }

    if (PyDict_Check(pdict)) {

        // depends on definition in py_mod.h
        pfun = PyDict_GetItemString(x->p_globals, "out_dict");
        if (pfun == NULL) {
            py_error(x, "retrieving out_dict func from globals failed");
            goto error;
        }

        pval = PyObject_CallFunctionObjArgs(pfun, pdict, NULL);
        if (pval == NULL) {
            py_error(x, "out_dict call failed to retrieve result");
            goto error;
        }

        if (PyList_Check(pval)) {           // expecting a python list
            py_handle_list_output(x, pval); // this decrefs pval
            py_bang_success(x);
            return MAX_ERR_NONE;
        } else {
            py_error(x, "expected list output got something else");
            goto error;
        }
    }

error:
    py_handle_error(x, "py_handle_dict_output failed");
    Py_XDECREF(pval);
    // fail bang
    py_bang_failure(x);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Generic handler to output arbitrarily-typed python object as max object
 *
 * @param x pointer to object struct
 * @param pval python object
 * @return t_max_err error code
 */
t_max_err py_handle_output(t_py* x, PyObject* pval)
{
    if (pval == NULL) {
        py_error(x, "cannot handle NULL value");
        return MAX_ERR_GENERIC;
    }

    if (PyFloat_Check(pval)) {
        return py_handle_float_output(x, pval);
    }

    else if (PyLong_Check(pval)) {
        return py_handle_long_output(x, pval);
    }

    else if (PyUnicode_Check(pval)) {
        return py_handle_string_output(x, pval);
    }

    else if (PySequence_Check(pval) && !PyBytes_Check(pval)
             && !PyByteArray_Check(pval)) {
        return py_handle_list_output(x, pval);
    }

    else if (PyDict_Check(pval)) {
        return py_handle_dict_output(x, pval);
    }

    else if (pval == Py_None) {
        return MAX_ERR_GENERIC;
    }

    else {
        py_error(x, "cannot handle his type of value");
        return MAX_ERR_GENERIC;
    }
}

/*--------------------------------------------------------------------------*/
/* Translators */

/**
 * @brief Translates atom vector to python list
 *
 * @param x pointer to object struct
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param start_from index of vector to start from
 * @return PyObject* python list
 */
PyObject* py_atoms_to_list(t_py* x, long argc, t_atom* argv, int start_from)
{

    PyObject* plist = NULL; // python list

    if ((plist = PyList_New(0)) == NULL) {
        py_error(x, "could not create an empty python list");
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
            py_log(x, "cannot process unknown type");
            break;
        }
    }
    return plist;

error:
    py_error(x, "atom to list conversion failed");
    return NULL;
}

/*--------------------------------------------------------------------------*/
/* Core Methods */

/**
 * @brief Import a python module
 *
 * @param x pointer to object structure
 * @param s symbol of module to be imported
 * @return t_max_err error code
 */
t_max_err py_import(t_py* x, t_symbol* s)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    PyObject* x_module = NULL;

    if (s != gensym("")) {
        x_module = PyImport_ImportModule(s->s_name);
        // x_module borrrowed ref
        if (x_module == NULL) {
            goto error;
        }
        PyDict_SetItemString(x->p_globals, s->s_name, x_module);
        PyGILState_Release(gstate);
        py_bang_success(x);
        py_log(x, "imported: %s", s->s_name);
    }
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "import %s", s->s_name);
    PyGILState_Release(gstate);
    py_bang_failure(x);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Evaluate a max symbol as a python expression
 *
 * @param x pointer to object structure
 * @param s symbol of object to be evaluated
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return t_max_err error code
 */
t_max_err py_eval(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    char* py_argv = atom_getsym(argv)->s_name;
    py_log(x, "%s %s", s->s_name, py_argv);

    PyObject* pval = PyRun_String(py_argv, Py_eval_input, x->p_globals,
                                  x->p_globals);

    if (pval != NULL) {
        py_handle_output(x, pval);
        PyGILState_Release(gstate);
        return MAX_ERR_NONE;
    } else {
        py_handle_error(x, "eval %s", py_argv);
        PyGILState_Release(gstate);
        py_bang_failure(x);
        return MAX_ERR_GENERIC;
    }
}

/**
 * @brief Execute a max symbol as a line of python code
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return t_max_err error code
 */
t_max_err py_exec(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

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
    Py_DECREF(pval);
    PyGILState_Release(gstate);

    py_bang_success(x);
    py_log(x, "exec %s", py_argv);
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "exec %s", py_argv);
    Py_XDECREF(pval);
    PyGILState_Release(gstate);
    py_bang_failure(x);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Execute contents of a file (obtained from symbol) as python code
 *
 * @param x pointer to object structure
 * @param s symbol
 * @return t_max_err error code
 */
t_max_err py_execfile(t_py* x, t_symbol* s)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    PyObject* pval = NULL;
    FILE* fhandle = NULL;

    if (s != gensym("")) {
        // set x->p_code_filepath
        t_max_err err = py_locate_path_from_symbol(x, s);
        if (err != MAX_ERR_NONE) {
            py_error(x, "could not locate path from symbol");
            goto error;
        }
    }

    if (s == gensym("") || x->p_code_filepath == gensym("")) {
        py_error(x, "could not set filepath");
        goto error;
    }

    // assume x->p_code_filepath has be been set without errors

    py_log(x, "pathname: %s", x->p_code_filepath->s_name);
    fhandle = fopen(x->p_code_filepath->s_name, "r+");

    if (fhandle == NULL) {
        py_error(x, "could not open file");
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
    PyGILState_Release(gstate);
    py_bang_success(x);
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "execfile");
    Py_XDECREF(pval);
    PyGILState_Release(gstate);
    py_bang_failure(x);
    return MAX_ERR_GENERIC;
}

/*--------------------------------------------------------------------------*/
/* Extra Methods */


/**
 * @brief Converts an atom list to a python assignment
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return t_max_err error code
 *
 * The first item of the Max list must be a symbol. This is converted into a python variable
 * and the rest of the list is assignment to this variable in the object's python
 * namespace.
 */
t_max_err py_assign(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    char* varname = NULL;
    PyObject* list = NULL;
    int res = 0;

    if (s != gensym(""))
        py_log(x, "s: %s", s->s_name);

    // first atom in argv must be a symbol
    if (argv->a_type != A_SYM) {
        py_error(x, "first atom must be a symbol!");
        goto error;

    } else {
        varname = atom_getsym(argv)->s_name;
        py_log(x, "varname: %s", varname);
    }

    list = py_atoms_to_list(x, argc, argv, 1);
    if (list == NULL) {
        py_error(x, "atom to py list conversion failed");
        goto error;
    }

    if (PyList_Size(list) != argc - 1) {
        py_error(x, "PyList_Size(list) != argc - 1");
        goto error;
    } else {
        py_log(x, "length of list: %d", PyList_Size(list));
    }

    // finally, assign list to varname in object namespace
    py_log(x, "setting %s to list in namespace", varname);
    // following does not steal ref to list
    res = PyDict_SetItemString(x->p_globals, varname, list);
    if (res != 0) {
        py_error(x, "assign varname to list failed");
        goto error;
    }
    // Py_XDECREF(list); // causes a crash (because it still exists?)
    PyGILState_Release(gstate);
    py_bang_success(x);
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "assign %s", s->s_name);
    Py_XDECREF(list);
    PyGILState_Release(gstate);
    py_bang_failure(x);
    return MAX_ERR_GENERIC;
}


/**
 * @brief A helper function to evaluate Max text as a Python expression.
 *
 * @param x pointer to object structure
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param offset offset of atom vector from which to evaluate
 * @return t_max_err error code
 */
t_max_err py_eval_text(t_py* x, long argc, t_atom* argv, int offset)
{
    PyGILState_STATE gstate = PyGILState_Ensure();

    long textsize = 0;
    char* text = NULL;
    int is_eval = 1;
    PyObject* co = NULL;
    PyObject* pval = NULL;

    t_max_err err = atom_gettext(argc + offset, argv, &textsize, &text,
                                 OBEX_UTIL_ATOM_GETTEXT_DEFAULT);
    if (err == MAX_ERR_NONE && textsize && text) {
        py_log(x, ">>> %s", text);
    } else {
        goto error;
    }

    // char* new_text = str_replace(text, "\\,", ",");
    char* new_text = str_replace(text, "\\", "");

    // co = Py_CompileString(text, x->p_name->s_name, Py_eval_input);
    co = Py_CompileString(new_text, x->p_name->s_name, Py_eval_input);


    if (PyErr_ExceptionMatches(PyExc_SyntaxError)) {
        PyErr_Clear();
        co = Py_CompileString(new_text, x->p_name->s_name, Py_single_input);
        // co = Py_CompileString(text, x->p_name->s_name, Py_single_input);
        is_eval = 0;
    }

    free(new_text);

    if (co == NULL) { // can be eval-co or exec-co or NULL here
        goto error;
    }
    sysmem_freeptr(text);

    pval = PyEval_EvalCode(co, x->p_globals, x->p_globals);
    if (pval == NULL) {
        goto error;
    }
    Py_DECREF(co);

    if (!is_eval) {
        // bang for exec-type op
        PyGILState_Release(gstate);
        py_bang_success(x);
    } else {
        py_handle_output(x, pval);
        PyGILState_Release(gstate);
    }
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "python code evaluation failed");
    // fail bang
    PyGILState_Release(gstate);
    py_bang_failure(x);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Converts all of the atom to text and evaluate as python code.
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return t_max_err error code
 */
t_max_err py_code(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    return py_eval_text(x, argc, argv, 0);
}





/**
 * @brief Anything method converting all of the atom to text and evaluate as python code.
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return t_max_err error code
 */
t_max_err py_anything(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    t_atom atoms[PY_MAX_ELEMS];

    if (s == gensym("")) {
        return MAX_ERR_GENERIC;
    }

    // set '=' as shorthand for assign method
    if (s == gensym("=")) {
        py_assign(x, gensym(""), argc, argv);
        return MAX_ERR_NONE;
    }

    // set symbol as first atom in new atoms array
    atom_setsym(atoms, s);

    for (int i = 0; i < argc; i++) {
        switch ((argv + i)->a_type) {
        case A_FLOAT: {
            atom_setfloat((atoms + (i + 1)), atom_getfloat(argv + i));
            break;
        }
        case A_LONG: {
            atom_setlong((atoms + (i + 1)), atom_getlong(argv + i));
            break;
        }
        case A_SYM: {
            atom_setsym((atoms + (i + 1)), atom_getsym(argv + i));
            break;
        }
        default:
            py_log(x, "cannot process unknown type");
            break;
        }
    }

    return py_eval_text(x, argc, atoms, 1);
}


/*--------------------------------------------------------------------------*/
/* Wrapped Python Methods */


/**
 * @brief      Apply a pure python function to atoms as text
 *
 * @param      x            pointer to object structure
 * @param      pyfunc_name  python function name
 * @param      s            symbol
 * @param[in]  argc         atom argument count
 * @param      argv         atom argument vector
 *
 * @return     t_max_err error code
 */
t_max_err py_apply_pyfunc(t_py* x, char* pyfunc_name, t_symbol* s, long argc, t_atom* argv)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    long textsize = 0;
    char* text = NULL;
    t_max_err err;
    PyObject* pyfunc = NULL;
    PyObject* pval = NULL;
    PyObject* pstr = NULL;

    err = atom_gettext(argc, argv, &textsize, &text,
                       OBEX_UTIL_ATOM_GETTEXT_DEFAULT);
    if (err != MAX_ERR_NONE || !textsize || !text) {
        py_error(x, "atom -> text conversion failed");
        goto error;
    }

    pstr = PyUnicode_FromString(text);
    if (pstr == NULL) {
        py_error(x, "cstr -> pyunicode conversion failed");
        goto error;
    }

    sysmem_freeptr(text);

    // depends on definition in py_mod.h
    pyfunc = PyDict_GetItemString(x->p_globals, pyfunc_name);
    if (pyfunc == NULL) {
        py_error(x, "retrieving python func '%s' from globals failed", pyfunc_name);
        goto error;
    }

    pval = PyObject_CallFunctionObjArgs(pyfunc, pstr, NULL);

    if (pval != NULL) {

        if (!PyUnicode_Check(pval)) {
            py_handle_output(x, pval); // this decrefs pval
        } else {
            // special case strings, which will cause crash if handled
            // out of this methods's scope. (huge PITA to debug!)
            const char* unicode_result = PyUnicode_AsUTF8(pval);
            if (unicode_result == NULL) {
                goto error;
            }
            outlet_anything(x->p_outlet_left, gensym(unicode_result), 0, NIL);
            py_bang_success(x);
            Py_XDECREF(pval);
        }

        Py_XDECREF(pstr);
        PyGILState_Release(gstate);
        py_bang_success(x);
        return MAX_ERR_NONE;
    } else {
        goto error;
    }

error:
    py_handle_error(x, "%s call failed", pyfunc_name);
    Py_XDECREF(pstr);
    Py_XDECREF(pval);
    // fail bang
    PyGILState_Release(gstate);
    py_bang_failure(x);
    return MAX_ERR_GENERIC;
}


/**
 * @brief Pipe a max list through a functional pipeline
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return t_max_err error code
 */
t_max_err py_pipe(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    return py_apply_pyfunc(x, "pipe", s, argc, argv);
}


/**
 * @brief Applies a max list to a set of left fold functions
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return t_max_err error code
 * 
 * The first elem in the list is treated as the accumulator
 */
t_max_err py_fold(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    return py_apply_pyfunc(x, "fold", s, argc, argv);
}


/**
 * @brief Run shell command from Max list
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return t_max_err error code
 */
t_max_err py_shell(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    return py_apply_pyfunc(x, "shell", s, argc, argv);
}


/**
 * @brief Converts a Max list to call a python function with arguments
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return t_max_err error code
 */
t_max_err py_call(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    return py_apply_pyfunc(x, "call", s, argc, argv);
}


/*--------------------------------------------------------------------------*/
/* Interobject Methods */

/**
 * @brief Scan object registry and populate object IDs.
 *
 * @param x object instance
 */
void py_scan(t_py* x)
{
    long result = 0;

    hashtab_clear(py_global_registry);

    if (x->p_patcher == NULL) {
        post("p_patcher == NULL");
    } else {
        post("p_patcher != NULL");
    }

    if (x->p_patcher) {
        object_method(x->p_patcher, gensym("iterate"),
                      (method)py_scan_callback, x, PI_DEEP | PI_WANTBOX,
                      &result);
    } else {
        py_error(x, "scan failed");
    }
}

/**
 * @brief A help function used by scan to scan registry and retrieve object IDs.
 *
 * @param x object instance
 * @param box box type instance
 * @return long
 */
long py_scan_callback(t_py* x, t_object* box)
{
    t_rect jr;
    t_object* p;
    t_symbol* s;
    t_symbol* varname;
    t_object* obj;
    t_symbol* obj_id;

    jbox_get_patching_rect(box, &jr);
    p = jbox_get_patcher(box);
    varname = jbox_get_varname(box);
    obj = jbox_get_object(box);

    // STRANGE BUG: single quotes in py_log cause a crash but not with post!!
    // perhaps because post is a macro for object_post?
    if (varname && varname != gensym("")) {
        // post("XXXX -> '%s'", varname->s_name);
        py_log(x, "storing object %s in the global registry", varname->s_name);
        hashtab_store(py_global_registry, varname, obj);

        obj_id = jbox_get_id(box);
        s = jpatcher_get_name(p);

        object_post(
            (t_object*)x,
            "in patcher:%s, varname:%s id:%s box @ x %ld y %ld, w %ld, h %ld",
            s->s_name, varname->s_name, obj_id->s_name, (long)jr.x, (long)jr.y,
            (long)jr.width, (long)jr.height);
    }

    return 0;
}

/**
 * @brief Send a named object an arbitrary message.
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return t_max_err error code
 */
t_max_err py_send(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    t_object* obj = NULL;
    char* obj_name = NULL;
    t_symbol* msg_sym = NULL;
    t_max_err err = 0;

    if (argc < 2) {
        py_error(x, "need at least 2 args to send msg");
        goto error;
    }

    if ((argv + 0)->a_type != A_SYM) {
        py_error(
            x, "1st arg of send needs to be a symbol name of receiver object");
        goto error;
    }

    // argv+0 is the object name to send to
    obj_name = atom_getsym(argv)->s_name;
    if (obj_name == NULL) {
        goto error;
    }

    // if registry is empty, scan it
    if (hashtab_getsize(py_global_registry) == 0) {
        py_scan(x);
    }

    // // lookup name in registry
    err = hashtab_lookup(py_global_registry, gensym(obj_name), &obj);
    if (err != MAX_ERR_NONE || obj == NULL) {
        py_error(x, "no object found in the registry");
        goto error;
    }

    // atom after the name of the receiver
    switch ((argv + 1)->a_type) {
    case A_SYM: {
        msg_sym = atom_getsym(argv + 1);
        if (msg_sym == NULL) { // should check type here
            goto error;
        }
        // address the minimum case: e.g a bang
        if (argc - 2 == 0) { //
            argc = 0;
            argv = NULL;
        } else {
            argc = argc - 2;
            argv = argv + 2;
        }
        break;
    }
    case A_FLOAT: {
        msg_sym = gensym("float");
        if (msg_sym == NULL) { // should check type here
            goto error;
        }

        argc = argc - 1;
        argv = argv + 1;

        break;
    }
    case A_LONG: {
        msg_sym = gensym("int");
        if (msg_sym == NULL) { // should check type here
            goto error;
        }

        argc = argc - 1;
        argv = argv + 1;

        break;
    }
    default:
        py_log(x, "cannot process unknown type");
        break;
    }

    // methods to get method type
    t_messlist* messlist = object_mess((t_object*)obj, msg_sym);
    if (messlist) {
        post("messlist->m_sym  (name of msg): %s", messlist->m_sym->s_name);
        post("messlist->m_type (type of msg): %d", messlist->m_type[0]);
    }

    err = object_method_typed(obj, msg_sym, argc, argv, NULL);
    if (err) {
        py_error(x, "failed to send a message to object %s", obj_name);
        goto error;
    }

    // success
    return MAX_ERR_NONE;

error:
    py_error(x, "send failed");
    return MAX_ERR_GENERIC;
}

/*--------------------------------------------------------------------------*/
/* Code-editor Methods */

/**
 * @brief Event of double-clicking on external object launches code-editor UI
 *
 * @param x pointer to object structure
 *
 */
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

/**
 * @brief Read text file into code-editor.
 *
 * @param x pointer to object structure
 * @param s path to text file
 */
void py_read(t_py* x, t_symbol* s)
{
    defer((t_object*)x, (method)py_doread, s, 0, NULL);
}

/**
 * @brief Read function callback
 *
 * @param x pointer to object structure
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 */
void py_doread(t_py* x, t_symbol* s, long argc, t_atom* argv)
{
    t_max_err err;
    t_filehandle fh;

    py_locate_path_from_symbol(x, s);
    err = path_opensysfile(x->p_code_filename, x->p_code_path, &fh, READ_PERM);
    if (!err) {
        sysfile_readtextfile(fh, x->p_code, 0,
                             TEXT_LB_UNIX | TEXT_NULL_TERMINATE);
        sysfile_close(fh);
        x->p_code_size = sysmem_handlesize(x->p_code);
    }
}


/**
 * @brief Run python code stored in editor buffer
 *
 * @param x pointer to object structure
 */
void py_run(t_py* x)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    PyObject* pval = NULL;

    if ((*(x->p_code) != NULL) && (*(x->p_code)[0] == '\0'))
        // is empty string
        goto error;

    pval = PyRun_String(*(x->p_code), Py_file_input, x->p_globals,
                        x->p_globals);
    if (pval == NULL) {
        goto error;
    }

    // success cleanup
    Py_DECREF(pval);
    PyGILState_Release(gstate);
    py_bang_success(x);
    return;

error:
    py_handle_error(x, "run x->p_code failed");
    Py_XDECREF(pval);
    PyGILState_Release(gstate);
    py_bang_failure(x);
}


/**
 * @brief Event function to preserve text in buffer after editor is closed
 *
 * @param x pointer to object structure
 * @param text text to be saved to buffer
 * @param size size of text to be saved to buffer
 */
void py_edclose(t_py* x, char** text, long size)
{
    if (x->p_code)
        sysmem_freehandle(x->p_code);

    x->p_code = sysmem_newhandleclear(size + 1);
    sysmem_copyptr((char*)*text, *x->p_code, size);
    x->p_code_size = size + 1;
    x->p_code_editor = NULL;
    if (x->p_run_on_close) {
        py_run(x);
    }
}


/**
 * @brief Cnfigures behavior of system responding to editor window close
 *
 * @param x       pointer to object structure
 * @param s       custom save text (optional)
 * @param result  set values [0-4] to adjust what happens
 *                     how the system responds when the editor
 *                     window is closed.
 */
void py_okclose(t_py* x, char *s, short *result)
{
    // see: https://cycling74.com/forums/text-editor-without-dirty-bit
    py_log(x, "okclose: called -- run-on-close: %d", x->p_run_on_close);
    *result = 3; // don't put up a dialog
    // const char *string = "custom save text";
    // memcpy(s, string, strlen(string)+1);
}

/**
 * @brief Provides run-code-on-save functionality to code-editor
 *
 * @param x pointer to object structure
 * @param text text to be run and saved
 * @param size size of text to be run and saved
 * @return t_max_err error code
 */
t_max_err py_edsave(t_py* x, char** text, long size)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    PyObject* pval = NULL;

    if (x->p_run_on_save) {

        py_log(x, "run-on-save activated");

        pval = PyRun_String(*text, Py_file_input, x->p_globals, x->p_globals);
        if (pval == NULL) {
            py_error(x, "py_edsave: pval == NULL");
            goto error;
        }

        // success cleanup
        Py_DECREF(pval);
    }
    PyGILState_Release(gstate);
    py_log(x, "py_edsave: returning 0");
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "py_edsave with (possible) execution failed");
    Py_XDECREF(pval);
    PyGILState_Release(gstate);
    py_log(x, "py_edsave: returning 1");
    return MAX_ERR_GENERIC;
}

/**
 * @brief Combo function of `py_read <path> -> py_execfile <path>`
 *
 * @param x pointer to object structure
 * @param s path as symbol
 */
void py_load(t_py* x, t_symbol* s)
{
    py_read(x, s);
    py_execfile(x, s);
}



/*--------------------------------------------------------------------------*/
/* Max Datastructures Support */

/**
 * @brief      Determines if py table exists.
 *
 * @param      x           pointer to object structure
 * @param      table_name  The table name
 *
 * @return     True if py table exists, False otherwise.
 */
bool py_table_exists(t_py* x, char* table_name)
{
    long **storage, size;

    return (table_get(gensym(table_name), &storage, &size) == 0);
}

/**
 * @brief      Convert python list to Max table
 *
 * @param      x           pointer to object structure
 * @param      table_name  The table name
 * @param      plist       The python list
 *
 * @return     The t maximum error.
 */
t_max_err py_list_to_table(t_py* x, char* table_name, PyObject* plist)
{
    long **storage, size, value;

    Py_ssize_t len = 0;
    PyObject* elem = NULL;

    if (plist == NULL) {
        goto error;
    }

    if (!PyList_Check(plist)) {
        goto error;
    }

    len = PyList_Size(plist);


    if (table_get(gensym(table_name), &storage, &size)) {
        if (len > size)
            goto error;

        for(int i = 0; i < len; i++) {
            elem = PyList_GetItem(plist, i);
            value = PyLong_AsLong(elem);
            *((*storage)+i) = value;
            py_log(x, "storage[%d] = %d", i, value);
        }
    }
    Py_XDECREF(plist);
    Py_XDECREF(elem);
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "plist to table failed");
    Py_XDECREF(plist);
    Py_XDECREF(elem);
    return MAX_ERR_GENERIC;
}

/**
 * @brief      Convert Max table to python list
 *
 * @param      x           pointer to object structure
 * @param      table_name  The table name
 *
 * @return     python list
 */
PyObject* py_table_to_list(t_py* x, char* table_name)
{

    PyObject* plist = NULL;
    long **storage, size, value;

    if ((plist = PyList_New(0)) == NULL) {
        py_log(x, "could not create an empty python list");
        goto error;
    }

    if (table_get(gensym(table_name), &storage, &size) == 0) {
        for(int i = 0; i < size; i++) {
            value = *((*storage)+i);
            py_log(x, "storage[%d] = %d", i, value);
            PyObject* p_long = PyLong_FromLong(value);
            if (p_long == NULL) {
                goto error;
            }
            PyList_Append(plist, p_long);
            Py_DECREF(p_long);
        }
        return plist;
    }

error:
    py_error(x, "table to list conversion failed");
    Py_RETURN_NONE;
}


