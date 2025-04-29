#ifndef PY_INTERPRETER_H
#define PY_INTERPRETER_H

#include "c74_min.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

using namespace c74::min;

namespace pyjs
{

// ---------------------------------------------------------------------------
// macros

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
        symbol p_name;           //!< unique python object name
        symbol p_pythonpath;     //!< path to python directory
        symbol p_source_name;    //!< base name of python file to execfile
        symbol p_source_path;    //!< full path to python file to execfile
        log_level p_log_level;   //!< object-level log level (error, info, debug)
        PyObject* p_globals;     //!< per object 'globals' python namespace

    public:
        PythonInterpreter(c74::max::t_class* c);
        ~PythonInterpreter();

        // helpers
        void log_debug(char* fmt, ...);
        void log_info(char* fmt, ...);
        void log_error(char* fmt, ...);
        void print_atom(const atoms& args);

        // python helpers
        void handle_error(char* fmt, ...);
        c74::max::t_max_err syspath_append(char* path);

        // python <-> atom translation
        
        // PyObject* atoms_to_plist_with_offset(long argc, t_atom* argv, int start_from);
        // PyObject* atoms_to_plist(long argc, t_atom* argv); //+
        // t_max_err plist_to_atoms(PyObject* seq, int* argc, t_atom** argv); //+
        // PyObject* atoms_to_ptuple(int argc, t_atom* argv); //+

        PyObject* atom_to_pobject(const atom& arg); // used by atoms_to_ptuple
        c74::max::t_max_err pobject_to_atom(PyObject* value, atom& arg); // used by plist_to_atoms

        // // python value -> atom -> output
        // t_max_err handle_float_output(void* outlet, PyObject* pval);
        // t_max_err handle_long_output(void* outlet, PyObject* pval);
        // t_max_err handle_string_output(void* outlet, PyObject* pval);
        // t_max_err handle_list_output(void* outlet, PyObject* pval);
        // t_max_err handle_dict_output(void* outlet, PyObject* pval);
        // t_max_err handle_output(void* outlet, PyObject* pval);

        // // core message method helpers
        // t_max_err import_module(char* module);
        // PyObject* eval_pcode(char* pcode);
        // t_max_err exec_pcode(char* pcode);
        // t_max_err execfile_path(char* path);

        // // core message methods
        // t_max_err import(t_symbol* s);
        // t_max_err eval(t_symbol* s, void* outlet);
        // t_max_err exec(t_symbol* s);
        // t_max_err execfile(t_symbol* s);

        // // extra message method helpers
        // PyObject* eval_text(char* text);
        // t_max_err eval_text_to_outlet(long argc, t_atom* argv, int offset, void* outlet);

        // // extra message methods
        // t_max_err call(t_symbol* s, long argc, t_atom* argv, void* outlet);
        // t_max_err assign(t_symbol* s, long argc, t_atom* argv);
        // t_max_err code(t_symbol* s, long argc, t_atom* argv, void* outlet);
        // t_max_err anything(t_symbol* s, long argc, t_atom* argv, void* outlet);
        // t_max_err pipe(t_symbol* s, long argc, t_atom* argv, void* outlet);

        // // datastructures
        // bool table_exists(char* table_name);
        // t_max_err plist_to_table(char* table_name, PyObject* pval);
        // PyObject* table_to_plist(char* table_name);

        // // path helpers
        // t_max_err locate_path_from_symbol(t_symbol* s);
        std::string get_path_to_external(c74::max::t_class* c, char* subpath);
        std::string get_path_to_package(c74::max::t_class* c, char* subpath);

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
PythonInterpreter::PythonInterpreter(c74::max::t_class* c)
{
    this->p_name = c74::max::symbol_unique();
    this->p_pythonpath = "";
    this->p_source_name = "";
    this->p_source_path = "";
    this->p_log_level = log_level::PY_LOG_LEVEL;

    // python init
    wchar_t* python_home = NULL;

    if (c) { // special-case pythonhome config, only makes sense if c not NULL

#if defined(__APPLE__) && defined(BUILD_STATIC)
    const char* resources_path = this->get_path_to_external(c, "/Contents/Resources").c_str();
    python_home = Py_DecodeLocale(resources_path, NULL);
#endif

#if defined(__APPLE__) && defined(BUILD_SHARED_PKG)
    const char* package_path = this->get_path_to_package(c, "/support/python" PY_VER).c_str();
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

    PyObject* main_mod = PyImport_AddModule((const char*)this->p_name); // borrowed
    this->p_globals = PyModule_GetDict(main_mod); // borrowed reference

    PyObject* py_name = NULL;
    PyObject* builtins = NULL;

    py_name = PyUnicode_FromString((const char*)this->p_name);
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

        c74::max::post("[py debug %s]: %s", this->p_name, msg);
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

        c74::max::post("[py info %s]: %s", this->p_name, msg);
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

        c74::max::error("[py error %s]: %s", this->p_name, msg);
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

        c74::max::error((char*)"[py %s] %s: %s", this->p_name, msg, pvalue_str);
    }
}


void PythonInterpreter::print_atom(const atoms& args)
{
    for (int i = 0; i < (int)args.size(); ++i) {
        switch (args[i].a_type) {
        case c74::max::A_FLOAT: {
            this->log_info((char*)"(%d) float: %f", i, (double)args[i]);
            break;
        }
        case c74::max::A_LONG: {
            this->log_info((char*)"(%d) long: %d", i, (long)args[i]);
            break;
        }
        case c74::max::A_SYM: {
            this->log_info((char*)"(%d) symbol: %s", i, std::string(args[i]).c_str());
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
c74::max::t_max_err PythonInterpreter::syspath_append(char* path)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    c74::max::t_max_err err = c74::max::MAX_ERR_NONE;
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
    err = c74::max::MAX_ERR_GENERIC;
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


PyObject* PythonInterpreter::atom_to_pobject(const atom& arg)
{
    this->log_debug((char*)"py_atom_to_py_object start");

    switch (arg.a_type) {

    case c74::max::A_LONG:
        this->log_debug((char*)"int: %i", (long)arg);
        return PyLong_FromLong((long)arg);

    case c74::max::A_FLOAT:
        this->log_debug((char*)"float: %f", (double)arg);
        return PyFloat_FromDouble((double)arg);

    case c74::max::A_SYM:
        this->log_debug((char*)"symbol: %s", std::string(arg).c_str());
        return PyUnicode_FromString(std::string(arg).c_str());

    case c74::max::A_NOTHING:
        Py_RETURN_NONE;

    default:
        // FIXME: should be this->log_warning
        this->log_error((char*)"Warning: type %d unsupported for conversion to Python.",
             arg.a_type);
        Py_RETURN_NONE;
    }
}



c74::max::t_max_err PythonInterpreter::pobject_to_atom(PyObject* value, atom& arg)
{
    c74::max::t_max_err err = c74::max::MAX_ERR_NONE;

    if (value == Py_True)
        arg = 1;
        // c74::max::atom_setlong(arg, 1);
    else if (value == Py_False)
        arg = 0;
        // c74::max::atom_setlong(arg, 0);
    else if (PyFloat_Check(value))
        arg = (double)PyFloat_AsDouble(value);
        // c74::max::atom_setfloat(arg, (double)PyFloat_AsDouble(value));
    else if (PyLong_Check(value))
        arg = (long)PyLong_AsLong(value);
        // c74::max::atom_setlong(arg, (long)PyLong_AsLong(value));
    else if (PyUnicode_Check(value))
        arg = symbol(PyUnicode_AsUTF8(value));
        // c74::max::atom_setsym(arg, symbol(PyUnicode_AsUTF8(value)));
    else {
        this->log_error((char*)"Warning: python type unsupported for conversion to max t_atom.");
        err = c74::max::MAX_ERR_GENERIC;
    }
    return err;
}





std::string get_path_to_external(c74::max::t_class* c, char* subpath)
{
    char external_path[c74::max::MAX_PATH_CHARS];
    char external_name[c74::max::MAX_PATH_CHARS];
    char conform_path[c74::max::MAX_PATH_CHARS];
    short path_id = c74::max::class_getpath(c);
    fs::path result;

#ifdef __APPLE__
    const char* ext_filename = "%s.mxo";
#else
    const char* ext_filename = "%s.mxe64";
#endif
    c74::max::snprintf_zero(external_name, c74::max::MAX_FILENAME_CHARS, ext_filename, c->c_sym->s_name);
    c74::max::path_toabsolutesystempath(path_id, external_name, external_path);
    c74::max::path_nameconform(external_path, conform_path, c74::max::PATH_STYLE_NATIVE,
                     c74::max::PATH_TYPE_TILDE);
    result = fs::path(external_path);
    if (subpath != NULL) {
        result /= subpath;
    }
    return result.string();
}



std::string get_path_to_package(c74::max::t_class* c, char* subpath)
{
    fs::path external_path = fs::path(get_path_to_external(c, NULL));
    fs::path externals_folder = external_path.parent_path();
    fs::path package_folder = externals_folder.parent_path();

    if (subpath != NULL) {
        package_folder /= subpath;
    }

    return package_folder.string();
}

// ===========================================================================

#endif // PY_INTERPRETER_IMPLEMENTATION

} // namespace pyjs