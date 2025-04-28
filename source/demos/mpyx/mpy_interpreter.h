#ifndef PY_INTERPRETER_H
#define PY_INTERPRETER_H

#include "c74_min.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>

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
        // void print_atom(const atoms& args);

        // python helpers
        void handle_error(char* fmt, ...);
        c74::max::t_max_err syspath_append(char* path);


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


// ===========================================================================

#endif // PY_INTERPRETER_IMPLEMENTATION

} // namespace pyjs