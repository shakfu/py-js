/** @file py_interpreter.h
    @brief A single-header python3 c++ library for Max externals.

    Provides a `PythonInterpreter` implementation which can be dropped into any
    arbitrary Max external. 

    The latest code can be found in [py-js](https://github.com/shakfu/py-js)

    If PY_INTERPRETER_IMPLEMENTATION is defined before including the header,
    it will activate the implementation, otherwise the implementation
    will not be included, this file is treated as a header.

    Usage example:

        ```cpp
        #define PY_INTERPRETER_IMPLEMENTATION // <-- activate the implementation
        #include "py_interpreter.h"

        typedef struct my {
            t_object obj;
            PythonInterpreter* py; // <-- this is the key opaque type and instance
            void* outlet;
        } t_my;

        // then use the interpreter's methods as you like. For example:

        t_max_err my_import(t_my* x, t_symbol* s)
        {
            return x->py->import(s);
        }

        t_max_err my_eval(t_my* x, t_symbol* s, long argc, t_atom* argv)
        {
            return x->py->eval(s, argc, argv, x->outlet);
        }
        ```

    This library is placed in the public domain.

*/
// ===========================================================================
// HEADER


#ifndef PY_INTERPRETER_H
#define PY_INTERPRETER_H

#include "ext.h"
#include "ext_obex.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>

namespace pyjs
{

// ---------------------------------------------------------------------------
// macros

#define if_null_error(x) if(x == NULL) { goto error; }
#define _STR(x) #x
#define STR(x) _STR(x)
#define PY_VER STR(PY_MAJOR_VERSION) "." STR(PY_MINOR_VERSION)

// ---------------------------------------------------------------------------
// constants

#define PY_MAX_ELEMS 1024
#define PY_LOG_LEVEL PY_DEBUG

// ---------------------------------------------------------------------------
// enums


/**
 * @brief      specifies three logging levels
 */
enum log_level {
    PY_ERROR, PY_INFO, PY_DEBUG 
};

// ---------------------------------------------------------------------------
// classes

/**
 * @brief      This class describes a python interpreter.
 */
class PythonInterpreter
{
    private:
        t_symbol* p_name;           //!< unique python object name
        t_symbol* p_pythonpath;     //!< path to python directory
        t_symbol* p_source_name;    //!< base name of python file to execfile
        t_symbol* p_source_path;    //!< full path to python file to execfile
        log_level p_log_level;      //!< object-level log level (error, info, debug)
        PyObject* p_globals;        //!< per object 'globals' python namespace

    public:
        PythonInterpreter(t_class* c);
        ~PythonInterpreter();

        // helpers
        void log_debug(char* fmt, ...);
        void log_info(char* fmt, ...);
        void log_error(char* fmt, ...);
        void print_atom(int argc, t_atom* argv);

        // python helpers
        void handle_error(char* fmt, ...);
        t_max_err syspath_append(char* path);

        // python <-> atom translation
        PyObject* atoms_to_plist_with_offset(long argc, t_atom* argv, int start_from);
        PyObject* atoms_to_plist(long argc, t_atom* argv); //+
        t_max_err plist_to_atoms(PyObject* seq, int* argc, t_atom** argv); //+
        PyObject* atoms_to_ptuple(int argc, t_atom* argv); //+

        PyObject* atom_to_pobject(t_atom* atom); // used by atoms_to_ptuple
        t_max_err pobject_to_atom(PyObject* value, t_atom* atom); // used by plist_to_atoms

        // python value -> atom -> output
        t_max_err handle_float_output(void* outlet, PyObject* pval);
        t_max_err handle_long_output(void* outlet, PyObject* pval);
        t_max_err handle_string_output(void* outlet, PyObject* pval);
        t_max_err handle_list_output(void* outlet, PyObject* pval);
        t_max_err handle_dict_output(void* outlet, PyObject* pval);
        t_max_err handle_output(void* outlet, PyObject* pval);

        // core message method helpers
        t_max_err import_module(char* module);
        PyObject* eval_pcode(char* pcode);
        t_max_err exec_pcode(char* pcode);
        t_max_err execfile_path(char* path);

        // core message methods
        t_max_err import(t_symbol* s);
        t_max_err eval(t_symbol* s, void* outlet);
        t_max_err exec(t_symbol* s);
        t_max_err execfile(t_symbol* s);

        // extra message method helpers
        PyObject* eval_text(char* text);
        t_max_err eval_text_to_outlet(long argc, t_atom* argv, int offset, void* outlet);

        // extra message methods
        t_max_err call(t_symbol* s, long argc, t_atom* argv, void* outlet);
        t_max_err assign(t_symbol* s, long argc, t_atom* argv);
        t_max_err code(t_symbol* s, long argc, t_atom* argv, void* outlet);
        t_max_err anything(t_symbol* s, long argc, t_atom* argv, void* outlet);
        t_max_err pipe(t_symbol* s, long argc, t_atom* argv, void* outlet);

        // datastructures
        bool table_exists(char* table_name);
        t_max_err plist_to_table(char* table_name, PyObject* pval);
        PyObject* table_to_plist(char* table_name);

        // path helpers
        t_max_err locate_path_from_symbol(t_symbol* s);
        t_string* get_path_to_package(t_class* c, char* subpath);
        t_string* get_path_to_external(t_class* c, char* subpath);
};

#endif /* PY_INTERPRETER_H */

// ===========================================================================
// IMPLEMENTATION

#ifdef PY_INTERPRETER_IMPLEMENTATION

/*
    py_interpreter.h
    
    single-header library providing minimal python3 services for Max externals.
*/


// ---------------------------------------------------------------------------
// constructor / destructor methods

/**
 * @brief      Constructs a new PythonInterpreter instance.
 */
PythonInterpreter::PythonInterpreter(t_class* c)
{
    this->p_name = symbol_unique();
    this->p_pythonpath = gensym("");
    this->p_source_name = gensym("");
    this->p_source_path = gensym("");
    this->p_log_level = log_level::PY_LOG_LEVEL;

    // python init
    wchar_t* python_home = NULL;

    if (c) { // special-case pythonhome config, only makes sense if c not NULL

#if defined(__APPLE__) && defined(BUILD_STATIC)
    const char* resources_path = string_getptr(
        this->get_path_to_external(c, "/Contents/Resources"));
    python_home = Py_DecodeLocale(resources_path, NULL);
#endif

#if defined(__APPLE__) && defined(BUILD_SHARED_PKG)
    const char* package_path = string_getptr(
        this->get_path_to_package(c, "/support/python" PY_VER));
    python_home = Py_DecodeLocale(package_path, NULL);
#endif

    } // end special-case python-home config

#if PY_VERSION_HEX < 0x0308000
    if (python_home != NULL) {
        Py_SetPythonHome(python_home);
        PyMem_RawFree(python_home);
    }
    Py_Initialize();
#else
    PyConfig config;
    PyConfig_InitPythonConfig(&config);
    config.parse_argv = 0; // Disable parsing command line arguments
    config.isolated = 0;   // default is disabled
    config.home = python_home;

    PyStatus status = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(status)) {
        PyConfig_Clear(&config);
        this->log_error((char*)"could not initialize python");
    }
    PyConfig_Clear(&config);
#endif

    PyObject* main_mod = PyImport_AddModule(this->p_name->s_name); // borrowed
    this->p_globals = PyModule_GetDict(main_mod); // borrowed reference

    PyObject* py_name = NULL;
    PyObject* builtins = NULL;

    py_name = PyUnicode_FromString(this->p_name->s_name);
    builtins = PyEval_GetBuiltins();
    PyDict_SetItemString(builtins, "PY_OBJ_NAME", py_name);
    PyDict_SetItemString(this->p_globals, "__builtins__", builtins);
    Py_XDECREF(py_name);
}


/**
 * @brief      PythonInterpreter destructor method.
 */
PythonInterpreter::~PythonInterpreter()
{
    Py_XDECREF(this->p_globals);
    Py_FinalizeEx();
}


// ---------------------------------------------------------------------------
// helper methods


/**
 * @brief Post msg to Max console.
 *
 * @param fmt character string with format codes
 * @param ... other arguments
 */
void PythonInterpreter::log_debug(char* fmt, ...)
{
    if (this->p_log_level >= log_level::PY_DEBUG) {
        char msg[PY_MAX_ELEMS];

        va_list va;
        va_start(va, fmt);
        vsnprintf(msg, PY_MAX_ELEMS, fmt, va);
        va_end(va);

        post("[py debug %s]: %s", this->p_name->s_name, msg);
    }
}


/**
 * @brief Post msg to Max console.
 *
 * @param fmt character string with format codes
 * @param ... other arguments
 */
void PythonInterpreter::log_info(char* fmt, ...)
{
    if (this->p_log_level >= log_level::PY_INFO) {
        char msg[PY_MAX_ELEMS];

        va_list va;
        va_start(va, fmt);
        vsnprintf(msg, PY_MAX_ELEMS, fmt, va);
        va_end(va);

        post("[py info %s]: %s", this->p_name->s_name, msg);
    }
}


/**
 * @brief Post error message to Max console.
 *
 * @param fmt character string with format codes
 * @param ... other arguments
 */
void PythonInterpreter::log_error(char* fmt, ...)
{
    if (this->p_log_level >= log_level::PY_ERROR) {
        char msg[PY_MAX_ELEMS];

        va_list va;
        va_start(va, fmt);
        vsnprintf(msg, PY_MAX_ELEMS, fmt, va);
        va_end(va);

        error((char*)"[py error %s]: %s", this->p_name->s_name, msg);
    }
}


/**
 * @brief Generic python error handler
 *
 * @param fmt format string
 * @param ... other args
 */
void PythonInterpreter::handle_error(char* fmt, ...)
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

        error((char*)"[py %s] %s: %s", this->p_name->s_name, msg, pvalue_str);
    }
}

void PythonInterpreter::print_atom(int argc, t_atom* argv)
{
    for (int i = 0; i < argc; i++) {
        switch ((argv + i)->a_type) {
        case A_FLOAT: {
            this->log_info((char*)"(%d) float: %f", i,
                           atom_getfloat(argv + i));
            break;
        }
        case A_LONG: {
            this->log_info((char*)"(%d) long: %d", i, atom_getlong(argv + i));
            break;
        }
        case A_SYM: {
            this->log_info((char*)"(%d) symbol: %s", i,
                           atom_getsym(argv + i)->s_name);
            break;
        }
        default:
            this->log_debug((char*)"cannot process unknown type");
            break;
        }
    }
}


/**
 * @brief Append string to python sys.path
 * 
 * @param path 
 * @return t_max_err 
 */
t_max_err PythonInterpreter::syspath_append(char* path)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    t_max_err err = MAX_ERR_NONE;
    PyObject* os = NULL;
    PyObject* os_path = NULL;
    PyObject* os_path_expandvars = NULL;
    PyObject* pre_expanded = NULL;
    PyObject* expanded_path = NULL;
    PyObject* sys_path = NULL;
    const char* expanded_path_cstr = NULL;

    if (path == NULL) {
        goto error;
    }

    os = PyImport_ImportModule("os"); // new ref
    if (os == NULL) {
        goto error;
    }
    
    os_path = PyObject_GetAttrString(os, "path"); // new ref
    if (os_path == NULL) {
        goto error;
    }

    os_path_expandvars = PyObject_GetAttrString(os_path, "expandvars"); // new ref.
    if (os_path_expandvars == NULL) {
        goto error;
    }
    
    pre_expanded = PyUnicode_FromString(path); // new ref
    if (pre_expanded == NULL) {
        goto error;
    }

    expanded_path = PyObject_CallFunctionObjArgs(os_path_expandvars, pre_expanded, NULL);
    if (expanded_path == NULL) {
        goto error;
    }

    expanded_path_cstr = PyUnicode_AsUTF8(expanded_path);
    if (expanded_path_cstr == NULL) {
        goto error;
    }
    this->log_debug((char*)"expanded string: %s", expanded_path_cstr);

    sys_path = PySys_GetObject((char*)"path"); // borrowed ref
    if (sys_path == NULL) {
        goto error;
    }
    PyList_Append(sys_path, expanded_path);
    goto finally;

error:
    err = MAX_ERR_GENERIC;
    this->handle_error((char*)"syspath append failed");

finally:
    Py_XDECREF(expanded_path);
    Py_XDECREF(pre_expanded);
    Py_XDECREF(os_path_expandvars);
    Py_XDECREF(os_path);
    Py_XDECREF(os);
    PyGILState_Release(gstate);
    return err;
}

// ---------------------------------------------------------------------------
// translation methods

/**
 * @brief      Converts max atom to python object
 *
 * @param      atom  The atom
 *
 * @return     python object (int, float, string)
 */
PyObject* PythonInterpreter::atom_to_pobject(t_atom* atom)
{
    this->log_debug((char*)"py_atom_to_py_object start");

    switch (atom->a_type) {

    case A_LONG:
        this->log_debug((char*)"int: %i", atom_getlong(atom));
        return PyLong_FromLong(atom_getlong(atom));

    case A_FLOAT:
        this->log_debug((char*)"float: %f", atom_getfloat(atom));
        return PyFloat_FromDouble(atom_getfloat(atom));

    case A_SYM:
        this->log_debug((char*)"symbol: %s", atom_getsym(atom)->s_name);
        return PyUnicode_FromString(atom_getsym(atom)->s_name);

    case A_NOTHING:
        Py_RETURN_NONE;

    default:
        // FIXME: should be this->log_warning
        this->log_error((char*)"Warning: type %d unsupported for conversion to Python.",
             atom->a_type);
        Py_RETURN_NONE;
    }
}


/**
 * @brief      Converts a python object to a max atom 
 *
 * @param      value  Python value
 * @param[out] atom   Max atom
 */
t_max_err PythonInterpreter::pobject_to_atom(PyObject* value, t_atom* atom)
{
    t_max_err err = MAX_ERR_NONE;

    if (value == Py_True)
        atom_setlong(atom, 1);
    else if (value == Py_False)
        atom_setlong(atom, 0);
    else if (PyFloat_Check(value))
        atom_setfloat(atom, (double)PyFloat_AsDouble(value));
    else if (PyLong_Check(value))
        atom_setlong(atom, (long)PyLong_AsLong(value));
    else if (PyUnicode_Check(value))
        atom_setsym(atom, gensym(PyUnicode_AsUTF8(value)));
    else {
        // FIXME: should this not return an 'error' t_symbol
        this->log_error((char*)"Warning: python type unsupported for conversion to max t_atom.");
        // atom_setsym(atom, gensym("error"));
        err = MAX_ERR_GENERIC;
    }
    return err;
}


/**
 * @brief Translates atom vector to python list
 *
 * @param argc atom argument count
 * @param argv atom argument vector
 *
 * @return PyObject* list
 */
PyObject* PythonInterpreter::atoms_to_plist(long argc, t_atom* argv)
{
    return this->atoms_to_plist_with_offset(argc, argv, 0);
}

/**
 * @brief Translates atom vector to python list
 *
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param start_from index of vector to start from
 *
 * @return PyObject* python list
 */
PyObject* PythonInterpreter::atoms_to_plist_with_offset(long argc,
                                                          t_atom* argv,
                                                          int start_from)
{

    PyObject* plist = NULL; // python list

    if ((plist = PyList_New(0)) == NULL) {
        this->log_error((char*)"could not create an empty python list");
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
            this->log_debug((char*)"cannot process unknown type");
            break;
        }
    }
    return plist;

error:
    this->log_error((char*)"atom to list conversion failed");
    return NULL;
}

/**
 * @brief      Populates in-place an empty atom list with the contents of a
 * python list
 *
 * @param      seq   The python list
 * @param      argc  The count of arguments
 * @param[out] argv  The arguments array
 * 
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::plist_to_atoms(PyObject* plist, int* argc,
                                            t_atom** argv)
{
    Py_ssize_t len = 0;
    Py_ssize_t i;
    PyObject* elem;

    if (plist == NULL) {
        goto error;
    }

    // FIXME: possible bug here. Check!
    if (PyList_Check(plist)) {
        len = PyList_Size(plist);
        *argv = (t_atom*)malloc(len * sizeof(t_atom));
        for (i = 0; i < len; i++) {
            elem = PyList_GetItem(plist, i);
            this->pobject_to_atom(elem, (*argv) + i);
        }
        *argc = (int)len;
        return MAX_ERR_NONE;
    }

error:
    this->handle_error((char*)"plist_to_atoms failed");
    Py_XDECREF(plist);
    return MAX_ERR_GENERIC;
}


/**
 * @brief      Converts an atom vector to a Python tuple
 *
 * @param[in]  argc  The length of the atom vector
 * @param      argv  The max atom vector
 *
 * @return     Python tuple object
 */
PyObject* PythonInterpreter::atoms_to_ptuple(int argc, t_atom* argv)
{
    PyObject* ptuple = PyTuple_New(argc);
    int i;
    for (i = 0; i < argc; i++) {
        PyObject* value = this->atom_to_pobject(&argv[i]);
        PyTuple_SetItem(ptuple, i, value); // pass value ref to the tuple
    }
    return ptuple;
}


// ---------------------------------------------------------------------------
// output methods

/**
 * @brief Handler to output python float as max float
 *
 * @param pfloat python float
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::handle_float_output(void* outlet, PyObject* pfloat)
{
    if (pfloat == NULL) {
        goto error;
    }

    if (PyFloat_Check(pfloat)) {
        float float_result = (float)PyFloat_AsDouble(pfloat);
        if (float_result == -1.0 && PyErr_Occurred()) {
            goto error;
        }
        outlet_float(outlet, float_result);
    }
    Py_XDECREF(pfloat);
    return MAX_ERR_NONE;

error:
    this->handle_error((char*)"handle_float_output failed");
    Py_XDECREF(pfloat);
    return MAX_ERR_GENERIC;
}



/**
 * @brief Handler to output python long as max int
 *
 * @param plong python long
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::handle_long_output(void* outlet, PyObject* plong)
{
    if (plong == NULL) {
        goto error;
    }

    if (PyLong_Check(plong)) {
        long long_result = PyLong_AsLong(plong);
        if (long_result == -1 && PyErr_Occurred()) {
            goto error;
        }
        outlet_int(outlet, long_result);
    }

    Py_XDECREF(plong);
    return MAX_ERR_NONE;

error:
    this->handle_error((char*)"handle_long_output failed");
    Py_XDECREF(plong);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Handler to output python string as max symbol
 *
 * @param pstring python string
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::handle_string_output(void* outlet, PyObject* pstring)
{
    if (pstring == NULL) {
        goto error;
    }

    if (PyUnicode_Check(pstring)) {
        const char* unicode_result = PyUnicode_AsUTF8(pstring);
        if (unicode_result == NULL) {
            goto error;
        }
        outlet_anything(outlet, gensym(unicode_result), 0, (t_atom*)NIL);
    }

    Py_XDECREF(pstring);
    return MAX_ERR_NONE;

error:
    this->handle_error((char*)"handle_string_output failed");
    Py_XDECREF(pstring);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Handler to output python list as max list
 *
 * @param plist python list
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::handle_list_output(void* outlet, PyObject* plist)
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
        this->log_debug((char*)"seq_size: %d", seq_size);

        if (seq_size == 0) {
            this->log_error((char*)"cannot convert py list of length 0 to atoms");
            goto error;
        }

        if (seq_size > PY_MAX_ELEMS) {
            this->log_debug((char*)"dynamically increasing size of atom array");
            atoms = atom_dynamic_start(atoms_static, PY_MAX_ELEMS,
                                       seq_size + 1);
            is_dynamic = 1;

        } else {
            atoms = atoms_static;
        }

        if ((iter = PyObject_GetIter(plist)) == NULL) {
            goto error;
        }
        this->log_debug((char*)"seq_size2: %d", seq_size);

        while ((item = PyIter_Next(iter)) != NULL) {
            if (PyLong_Check(item)) {
                long long_item = PyLong_AsLong(item);
                if (long_item == -1 && PyErr_Occurred()) {
                    goto error;
                }
                atom_setlong(atoms + i, long_item);
                this->log_debug((char*)"%d long: %ld\n", i, long_item);
                i++;
            }

            if (PyFloat_Check(item)) {
                float float_item = PyFloat_AsDouble(item);
                if (float_item == -1.0 && PyErr_Occurred()) {
                    goto error;
                }
                atom_setfloat(atoms + i, float_item);
                this->log_debug((char*)"%d float: %f\n", i, float_item);
                i++;
            }

            if (PyUnicode_Check(item)) {
                const char* unicode_item = PyUnicode_AsUTF8(item);
                if (unicode_item == NULL) {
                    goto error;
                }
                atom_setsym(atoms + i, gensym(unicode_item));
                this->log_debug((char*)"%d unicode: %s\n", i, unicode_item);
                i++;
            }
            Py_DECREF(item);
        }

        outlet_list(outlet, NULL, i, atoms);
        this->log_debug((char*)"end iter op: %d", i);

        if (is_dynamic) {
            this->log_debug((char*)"restoring to static atom array");
            atom_dynamic_end(atoms_static, atoms);
        }
    }

    Py_XDECREF(plist);
    return MAX_ERR_NONE;

error:
    this->handle_error((char*)"handle_list_output failed");
    Py_XDECREF(plist);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Handler to output python dict as max list
 *
 * @param pdict python dict
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::handle_dict_output(void* outlet, PyObject* pdict)
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
                               Py_single_input, this->p_globals, this->p_globals);

        if (pfun_co == NULL) {
            this->log_error((char*)"out_dict function code object is NULL");
            goto error;
        }

        pfun = PyDict_GetItemString(this->p_globals, "__py_maxmsp_out_dict");
        if (pfun == NULL) {
            this->log_error((char*)"retrieving out_dict func from globals failed");
            goto error;
        }

        pval = PyObject_CallFunctionObjArgs(pfun, pdict, NULL);
        if (pval == NULL) {
            this->log_error((char*)"out_dict call failed to retrieve result");
            goto error;
        }

        if (PyList_Check(pval)) {                   // expecting a python list
            this->handle_list_output(outlet, pval); // this decrefs pval
            Py_XDECREF(pfun_co);
            return MAX_ERR_NONE;
        } else {
            this->log_error((char*)"expected list output got something else");
            goto error;
        }
    }

error:
    this->handle_error((char*)"handle_dict_output failed");
    Py_XDECREF(pfun_co);
    Py_XDECREF(pval);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Generic handler to output arbitrarily-typed python object as max
 * object
 *
 * @param pval python object
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::handle_output(void* outlet, PyObject* pval)
{
    if (pval == NULL) {
        this->log_error((char*)"cannot handle NULL value");
        return MAX_ERR_GENERIC;
    }

    if (PyFloat_Check(pval)) {
        return this->handle_float_output(outlet, pval);
    }

    else if (PyLong_Check(pval)) {
        return this->handle_long_output(outlet, pval);
    }

    else if (PyUnicode_Check(pval)) {
        return this->handle_string_output(outlet, pval);
    }

    else if (PySequence_Check(pval) && !PyBytes_Check(pval)
             && !PyByteArray_Check(pval)) {
        return this->handle_list_output(outlet, pval);
    }

    else if (PyDict_Check(pval)) {
        return this->handle_dict_output(outlet, pval);
    }

    else if (pval == Py_None) {
        return MAX_ERR_GENERIC;
    }

    else {
        this->log_error((char*)"cannot handle his type of value");
        return MAX_ERR_GENERIC;
    }
}

// ---------------------------------------------------------------------------------------
// CORE METHOD HELPERS


/**
 * @brief Import a python module
 *
 * @param module to be imported as cstring
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::import_module(char* module)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    PyObject* pmodule = NULL;

    if (module == NULL) {
        goto error;
    }

    pmodule = PyImport_ImportModule(module);
    if (pmodule == NULL) {
        goto error;
    }
        
    PyDict_SetItemString(this->p_globals, module, pmodule);
    PyGILState_Release(gstate);
    this->log_debug((char*)"imported: %s", module);
    return MAX_ERR_NONE;

error:
    this->handle_error((char*)"import %s", module);
    PyGILState_Release(gstate);
    return MAX_ERR_GENERIC;
}


/**
 * @brief      eval python code from cstring
 *
 * @param      python code in cstring
 *
 * @return     result of python evaluation
 */
PyObject* PythonInterpreter::eval_pcode(char* pcode)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    PyObject* pval = PyRun_String(pcode,
        Py_eval_input, this->p_globals, this->p_globals);

    if (pval != NULL) {
        PyGILState_Release(gstate);
        return pval;
    } else {
        this->handle_error((char*)"failed python code eval: %s", pcode);
        PyGILState_Release(gstate);
        Py_RETURN_NONE;
    }
}


/**
 * @brief Execute a line of python code
 *
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return t_max_err error code
 */ 
t_max_err PythonInterpreter::exec_pcode(char* pcode)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    PyObject* pval = PyRun_String(pcode,
        Py_single_input, this->p_globals, this->p_globals);

    if (pval == NULL) {
        goto error;
    }
    Py_XDECREF(pval);
    PyGILState_Release(gstate);

    this->log_debug((char*)"exec %s", pcode);
    return MAX_ERR_NONE;

error:
    this->handle_error((char*)"exec %s", pcode);
    Py_XDECREF(pval);
    PyGILState_Release(gstate);
    return MAX_ERR_GENERIC;
}


/**
 * @brief Execute contents of a file (obtained from symbol) as python code
 *
 * @param path cstring of path to execfile
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::execfile_path(char* path)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    PyObject* pval = NULL;
    FILE* fhandle = NULL;

    if (path == NULL) {
        goto error;
    }

    fhandle = fopen(path, "r+");

    if (fhandle == NULL) {
        this->log_error((char*)"could not open file");
        goto error;
    }

    pval = PyRun_File(fhandle, path, Py_file_input, this->p_globals, this->p_globals);

    if (pval == NULL) {
        fclose(fhandle);
        goto error;
    }

    // success cleanup
    fclose(fhandle);
    Py_DECREF(pval);
    PyGILState_Release(gstate);
    return MAX_ERR_NONE;

error:
    this->handle_error((char*)"execfile");
    Py_XDECREF(pval);
    PyGILState_Release(gstate);
    return MAX_ERR_GENERIC;
}


/**
 * @brief A helper function to evaluate Max text as a Python expression.
 *
 * @param text c-string
 *
 * @return PyObject* pointer to python object
 */
PyObject* PythonInterpreter::eval_text(char* text)
{
    PyGILState_STATE gstate = PyGILState_Ensure();

    int is_eval = 1;
    PyObject* co = NULL;
    PyObject* pval = NULL;

    co = Py_CompileString(text, this->p_name->s_name, Py_eval_input);

    if (PyErr_ExceptionMatches(PyExc_SyntaxError)) {
        PyErr_Clear();
        co = Py_CompileString(text, this->p_name->s_name, Py_single_input);
        is_eval = 0;
    }

    if (co == NULL) { // can be eval-co or exec-co or NULL here
        sysmem_freeptr(text);
        goto error;
    }
    sysmem_freeptr(text);

    pval = PyEval_EvalCode(co, this->p_globals, this->p_globals);
    if (pval == NULL) {
        goto error;
    }
    Py_DECREF(co);

    if (!is_eval) {
        PyGILState_Release(gstate);
    }

    return pval;

error:
    this->handle_error((char*)"python code evaluation failed");
    PyGILState_Release(gstate);
    Py_RETURN_NONE;
}

// ---------------------------------------------------------------------------------------
// CORE METHODS


/**
 * @brief Import a python module
 *
 * @param s symbol of module to be imported
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::import(t_symbol* s)
{
    if (s == gensym("")) {
        return MAX_ERR_GENERIC;
    }
    return this->import_module(s->s_name);
}


/**
 * @brief Evaluate a max symbol as a python expression
 *
 * @param s symbol of code to be evaluated
 * @param outlet object outlet
 *
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::eval(t_symbol* s, void* outlet)
{
    PyObject* pval = this->eval_pcode(s->s_name);

    if (pval == NULL) {
        return MAX_ERR_GENERIC;
    }

    this->handle_output(outlet, pval);
    return MAX_ERR_NONE;
}


/**
 * @brief Execute a max symbol as a line of python code
 *
 * @param s symbol of code to be executed
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::exec(t_symbol* s)
{
    return this->exec_pcode(s->s_name);
}


/**
 * @brief Execute contents of a file (obtained from symbol) as python code
 *
 * @param s symbol
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::execfile(t_symbol* s)
{

    if (s != gensym("")) {
        // set this->p_source_path
        t_max_err err = this->locate_path_from_symbol(s);
        if (err != MAX_ERR_NONE) {
            this->log_error((char*)"could not locate path from symbol");
            return err;
        }
    }

    if (s == gensym("") || this->p_source_path == gensym("")) {
        this->log_error((char*)"could not set filepath");
        return MAX_ERR_GENERIC;
    }

    // assume this->p_source_path has be been set without errors

    return this->execfile_path(this->p_source_path->s_name);

}


// ---------------------------------------------------------------------------------------
// EXTRA METHODS HELPERS


/**
 * @brief A helper function to evaluate Max text as a Python expression.
 *
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param offset offset of atom vector from which to evaluate
 * @param outlet object outlet
 *
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::eval_text_to_outlet(long argc, t_atom* argv, int offset, void* outlet)
{
    long textsize = 0;
    char* text = NULL;

    t_max_err err = atom_gettext(argc + offset, argv, &textsize, &text,
                                 OBEX_UTIL_ATOM_GETTEXT_DEFAULT);
    if (err == MAX_ERR_NONE && textsize && text) {
        this->log_debug((char*)">>> %s", text);
        PyObject* pval = this->eval_text(text);
        if (pval != NULL) {
            this->handle_output(outlet, pval);
            return MAX_ERR_NONE;
        }
    }
    return MAX_ERR_GENERIC;
}




// ---------------------------------------------------------------------------------------
// EXTRA METHODS

/**
 * @brief Converts a Max list to call a python function with arguments
 *
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param outlet object outlet
 *
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::call(t_symbol* s, long argc, t_atom* argv, void* outlet)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    char* callable_name = NULL;
    PyObject* py_argslist = NULL;
    PyObject* pval = NULL;
    PyObject* py_callable = NULL;
    // python list
    PyObject* py_args = NULL; // python tuple

    // first atom in argv must be a symbol
    if (argv->a_type != A_SYM) {
        this->log_error((char*)"first atom must be a symbol!");
        goto error;

    } else {
        callable_name = atom_getsym(argv)->s_name;
        this->log_debug((char*)"callable_name: %s", callable_name);
    }

    py_callable = PyRun_String(callable_name, Py_eval_input, this->p_globals,
                               this->p_globals);
    if (py_callable == NULL) {
        this->log_error((char*)"could not evaluate %s", callable_name);
        goto error;
    }

    py_argslist = this->atoms_to_plist_with_offset(argc, argv, 1);
    if (py_argslist == NULL) {
        this->log_error((char*)"atom to py list conversion failed");
        goto error;
    }

    this->log_debug((char*)"length of argc:%ld list: %d", argc,
           PyList_Size(py_argslist));

    // convert py_args to tuple
    py_args = PyList_AsTuple(py_argslist);
    if (py_args == NULL) {
        this->log_error((char*)"unable to convert args list to tuple");
        goto error;
    }

    pval = PyObject_CallObject(py_callable, py_args);
    if (!PyErr_ExceptionMatches(PyExc_TypeError)) {
        if (pval == NULL) {
            this->log_error((char*)"unable to apply callable(*args)");
            goto error;
        }
        goto handle_output;
    }
    PyErr_Clear();

    pval = PyObject_CallFunctionObjArgs(py_callable, py_argslist, NULL);
    if (pval == NULL) {
        this->log_error((char*)"could not retrieve result of callable(list)");
        goto error;
    }
    goto handle_output; // this is redundant but safer in case code is added

handle_output:
    this->handle_output(outlet, pval);
    // success cleanup
    Py_XDECREF(py_callable);
    Py_XDECREF(py_argslist);
    PyGILState_Release(gstate);
    return MAX_ERR_NONE;

error:
    this->handle_error((char*)"method %s", s->s_name);
    // cleanup
    Py_XDECREF(py_callable);
    Py_XDECREF(py_argslist);
    Py_XDECREF(pval);
    PyGILState_Release(gstate);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Converts an atom list to a python assignment
 *
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return t_max_err error code
 *
 * The first item of the Max list must be a symbol. This is converted into a
 * python variable and the rest of the list is assignment to this variable in
 * the object's python namespace.
 */
t_max_err PythonInterpreter::assign(t_symbol* s, long argc, t_atom* argv)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    char* varname = NULL;
    PyObject* list = NULL;
    int res;

    if (s != gensym(""))
        this->log_debug((char*)"s: %s", s->s_name);

    // first atom in argv must be a symbol
    if (argv->a_type != A_SYM) {
        this->log_error((char*)"first atom must be a symbol!");
        goto error;

    } else {
        varname = atom_getsym(argv)->s_name;
        this->log_debug((char*)"varname: %s", varname);
    }

    list = this->atoms_to_plist_with_offset(argc, argv, 1);
    if (list == NULL) {
        this->log_error((char*)"atom to py list conversion failed");
        goto error;
    }

    if (PyList_Size(list) != argc - 1) {
        this->log_error((char*)"PyList_Size(list) != argc - 1");
        goto error;
    } else {
        this->log_debug((char*)"length of list: %d", PyList_Size(list));
    }

    // finally, assign list to varname in object namespace
    this->log_debug((char*)"setting %s to list in namespace", varname);
    // following does not steal ref to list
    res = PyDict_SetItemString(this->p_globals, varname, list);
    if (res != 0) {
        this->log_error((char*)"assign varname to list failed");
        goto error;
    }
    PyGILState_Release(gstate);
    return MAX_ERR_NONE;

error:
    this->handle_error((char*)"assign %s", s->s_name);
    Py_XDECREF(list);
    PyGILState_Release(gstate);
    return MAX_ERR_GENERIC;
}


/**
 * @brief Converts all of the atom to text and evaluate as python code.
 *
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param outlet object outlet
 *
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::code(t_symbol* s, long argc, t_atom* argv, void* outlet)
{
    return this->eval_text_to_outlet(argc, argv, 0, outlet);
}


/**
 * @brief Anything method converting all of the atom to text and evaluate as
 * python code.
 *
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param outlet object outlet
 *
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::anything(t_symbol* s, long argc, t_atom* argv,
                      void* outlet)
{
    t_atom atoms[PY_MAX_ELEMS];

    if (s == gensym("")) {
        return MAX_ERR_GENERIC;
    }

    // set '=' as shorthand for assign method
    else if (s == gensym("=")) {
        return this->assign(gensym(""), argc, argv);
    }

    // check for properties
    else if (s == gensym("pythonpath")) {
        if (argc == 0) {
            this->log_info((char*)"property pythonpath: %s",
                            this->p_pythonpath->s_name);
            return MAX_ERR_NONE;
        }
        
        if (argc == 1) {
            if ((argv)->a_type == A_SYM) {
                this->log_info((char*)"setting pythonpath to %s",
                               atom_getsym(argv)->s_name);
                this->p_pythonpath = gensym(atom_getsym(argv)->s_name);
                return this->syspath_append(this->p_pythonpath->s_name);
            }
        }
        return MAX_ERR_GENERIC;
    }

    else if (s == gensym("log_level")) {
        if (argc == 0) {
            this->log_info((char*)"property log_level: %d", 
                            this->p_log_level);
            return MAX_ERR_NONE;
        }

        if (argc == 1) {
            if ((argv)->a_type == A_LONG) {
                this->log_info((char*)"setting log_level to %d", atom_getlong(argv));
                this->p_log_level = (log_level)atom_getlong(argv);
                return MAX_ERR_NONE;
            }
        }
        return MAX_ERR_GENERIC;
    }
    
    else {

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
                this->log_debug((char*)"cannot process unknown type");
                break;
            }
        }

        return this->eval_text_to_outlet(argc, atoms, 1, outlet);
    }
}

/**
 * @brief Create a function python pipeline from a Max list
 *
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param outlet object outlet
 *
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::pipe(t_symbol* s, long argc, t_atom* argv, void* outlet)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    long textsize = 0;
    char* text = NULL;
    t_max_err err;
    PyObject* pipe_pre = NULL;
    PyObject* pipe_fun = NULL;
    PyObject* pval = NULL;
    PyObject* pstr = NULL;

    err = atom_gettext(argc, argv, &textsize, &text,
                       OBEX_UTIL_ATOM_GETTEXT_DEFAULT);
    if (err != MAX_ERR_NONE || !textsize || !text) {
        this->log_error((char*)"atom -> text conversion failed");
        goto error;
    }

    pipe_pre = PyRun_String(
        "def __py_maxmsp_pipe(arg):\n"
        "\targs = arg.split()\n"
        "\tval = eval(args[0], locals(), globals())\n"
        "\tfuncs = [eval(f, locals(), globals()) for f in args[1:]]\n"
        "\tfor f in funcs:\n"
        "\t\tval = f(val)\n"
        "\treturn val\n",
        Py_single_input, this->p_globals, this->p_globals);

    if (pipe_pre == NULL) {
        this->log_error((char*)"pipe func is NULL");
        goto error;
    }

    pstr = PyUnicode_FromString(text);
    if (pstr == NULL) {
        this->log_error((char*)"cstr -> pyunicode conversion failed");
        goto error;
    }

    sysmem_freeptr(text);

    pipe_fun = PyDict_GetItemString(this->p_globals, "__py_maxmsp_pipe");
    if (pipe_fun == NULL) {
        this->log_error((char*)"retrieving pipe func from globals failed");
        goto error;
    }

    pval = PyObject_CallFunctionObjArgs(pipe_fun, pstr, NULL);

    if (pval != NULL) {

        if (!PyUnicode_Check(pval)) {
            this->handle_output(outlet, pval); // this decrefs pval
        } else {
            // special case strings, which will cause crash if handled
            // out of this methods's scope. (huge PITA to debug!)
            const char* unicode_result = PyUnicode_AsUTF8(pval);
            if (unicode_result == NULL) {
                goto error;
            }
            outlet_anything(outlet, gensym(unicode_result), 0, (t_atom*)NIL);
            Py_XDECREF(pval);
        }

        Py_XDECREF(pipe_pre);
        Py_XDECREF(pstr);
        PyGILState_Release(gstate);
        return MAX_ERR_NONE;
    } else {
        goto error;
    }

error:
    this->handle_error((char*)"pipe failed");
    Py_XDECREF(pipe_pre);
    Py_XDECREF(pstr);
    Py_XDECREF(pval);
    PyGILState_Release(gstate);
    return MAX_ERR_GENERIC;
}

// ---------------------------------------------------------------------------------------
// datastructure support

bool PythonInterpreter::table_exists(char* table_name)
{
    long **storage, size;

    return (table_get(gensym(table_name), &storage, &size) == 0);
}

t_max_err PythonInterpreter::plist_to_table(char* table_name, PyObject* plist)
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
            this->log_debug((char*)"storage[%d] = %d", i, value);
        }
    }
    Py_XDECREF(plist);
    Py_XDECREF(elem);
    return MAX_ERR_NONE;

error:
    this->handle_error((char*)"plist to table failed");
    Py_XDECREF(plist);
    Py_XDECREF(elem);
    return MAX_ERR_GENERIC;
}


PyObject* PythonInterpreter::table_to_plist(char* table_name) {

    PyObject* plist = NULL;
    long **storage, size, value;

    if ((plist = PyList_New(0)) == NULL) {
        this->log_error((char*)"could not create an empty python list");
        goto error;
    }

    if (table_get(gensym(table_name), &storage, &size) == 0) {
        for(int i = 0; i < size; i++) {
            value = *((*storage)+i);
            this->log_debug((char*)"storage[%d] = %d", i, value);
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
    this->log_error((char*)"table to list conversion failed");
    Py_RETURN_NONE;
}

// ---------------------------------------------------------------------------------------
// path helpers

/**
 * @brief Searches the Max filesystem context for a file given by a symbol
 *
 * @param s symbol to be searched
 * @return t_max_err
 */
t_max_err PythonInterpreter::locate_path_from_symbol(t_symbol* s)
{
    t_fourcc filetype = FOUR_CHAR_CODE('TEXT');
    t_fourcc outtype = 0;
    short path_code = 0;
    char filename[MAX_PATH_CHARS];
    char pathname[MAX_PATH_CHARS];
    t_max_err ret = MAX_ERR_NONE;

    if (s == gensym("")) { // if no arg supplied to ask for file
        filename[0] = 0;

        if (open_dialog(filename, &path_code,
                        &outtype, &filetype, 1))
            // non-zero: cancelled
            ret = MAX_ERR_GENERIC;
        goto finally;

    } else {
        // must copy symbol before calling locatefile_extended
        strncpy_zero(filename, s->s_name, MAX_PATH_CHARS);
        if (locatefile_extended(filename, &path_code,
                                &outtype, &filetype, 1)) {
            // nozero: not found
            this->log_error((char*)"can't find file %s", s->s_name);
            ret = MAX_ERR_GENERIC;
            goto finally;
        } else {
            pathname[0] = 0;
            ret = path_toabsolutesystempath(path_code, filename, pathname);
            if (ret != MAX_ERR_NONE) {
                this->log_error((char*)"can't convert %s to absolutepath", s->s_name);
                goto finally;
            }
        }

        // success
        // set attribute from pathname symbol
        this->log_debug((char*)"filename: %s", filename);
        this->log_debug((char*)"pathname: %s", pathname);
        this->p_source_name = gensym(filename);
        this->p_source_path = gensym(pathname);
        assert(ret == MAX_ERR_NONE);
    }

finally:
    return ret;
}


/**
 * @brief  Return path to external with optional subpath
 *
 * @param  c        t_class instance
 * @param  subpath  The subpath or NULL (if not)
 *
 * @return path to external + (optional subpath)
 */
t_string* PythonInterpreter::get_path_to_external(t_class* c, char* subpath)
{
    char external_path[MAX_PATH_CHARS];
    char external_name[MAX_PATH_CHARS];
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
    result = string_new(external_path);
    if (subpath != NULL) {
        string_append(result, subpath);
    }
    return result;
}

/**
 * @brief  Return path to package with optional subpath
 *
 * @param  c        t_class instance
 * @param  subpath  The subpath or NULL (if not)
 *
 * @return path to package + (optional subpath)
 */
t_string* PythonInterpreter::get_path_to_package(t_class* c, char* subpath)
{
    char _dummy[MAX_PATH_CHARS];
    char externals_folder[MAX_PATH_CHARS];
    char package_folder[MAX_PATH_CHARS];

    t_string* result;
    t_string* external_path = this->get_path_to_external(c, NULL);

    const char* ext_path_c = string_getptr(external_path);

    path_splitnames(ext_path_c, externals_folder, _dummy); // ignore filename
    path_splitnames(externals_folder, package_folder,
                    _dummy); // ignore filename

    result = string_new((char*)package_folder);

    if (subpath != NULL) {
        string_append(result, subpath);
    }

    return result;
}


// ===========================================================================

#endif // PY_INTERPRETER_IMPLEMENTATION

} // namespace pyjs