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
        void syspath_append(const char* path);

        // python <-> atom translation
        PyObject* atoms_to_plist_with_offset(const atoms& args, int start_from);
        PyObject* atoms_to_plist(const atoms& args);
        atoms plist_to_atoms(PyObject* plist);
        PyObject* atoms_to_ptuple(const atoms& args);

        PyObject* atom_to_pobject(const atom& arg); // used by atoms_to_ptuple
        atom pobject_to_atom(PyObject* value); // used by plist_to_atoms

        // python value -> atom -> output
        void handle_float_output(outlet<> *output, PyObject* pfloat);
        void handle_long_output(outlet<> *output, PyObject* pval);
        void handle_string_output(outlet<> *output, PyObject* pstring);
        void handle_list_output(outlet<> *output, PyObject* pval);
        void handle_dict_output(outlet<> *output, PyObject* pval);
        void handle_output(outlet<> *output, PyObject* pval);

        // core message method helpers
        void import_module(const char* module);
        PyObject* eval_pcode(const char* pcode);
        void exec_pcode(const char* pcode);
        void execfile_path(const char* path);

        // core message methods
        void import(const char* module);
        void import(const atoms& args);
        void eval(const char* code, outlet<> *output);
        void eval(const atoms& args, outlet<> *output);
        void exec(const char* code);
        void exec(const atoms& args);
        void execfile(const char* path);
        void execfile(const atoms& args);

        // extra message method helpers
        PyObject* eval_text(char* text);
        void eval_text_to_outlet(const atoms& args, int offset, outlet<> *output);

        // extra message methods
        void call(const atoms& args, outlet<> *output);
        void assign(const atoms& args);
        void code(const atoms& args, outlet<> *output);
        void anything(symbol s, const atoms& args, outlet<> *output);
        void pipe(const atoms& args, outlet<> *output);

        // datastructures
        bool table_exists(const char* table_name);
        void plist_to_table(const char* table_name, PyObject* pval);
        PyObject* table_to_plist(const char* table_name);

        // path helpers
        bool locate_path(const char* path);
        std::string get_path_to_external(c74::max::t_class* c, const char* subpath);
        std::string get_path_to_package(c74::max::t_class* c, const char* subpath);

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
    this->p_log_level = log_level::PY_DEBUG;

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

void PythonInterpreter::log_debug(char* fmt, ...)
{
    if (this->p_log_level >= log_level::PY_DEBUG) {
        char msg[PY_MAX_ELEMS];

        va_list va;
        va_start(va, fmt);
        vsnprintf(msg, PY_MAX_ELEMS, fmt, va);
        va_end(va);

        c74::max::post("[py debug %s]: %s", (const char*)this->p_name, msg);
    }
}


void PythonInterpreter::log_info(char* fmt, ...)
{
    if (this->p_log_level >= log_level::PY_INFO) {
        char msg[PY_MAX_ELEMS];

        va_list va;
        va_start(va, fmt);
        vsnprintf(msg, PY_MAX_ELEMS, fmt, va);
        va_end(va);

        c74::max::post("[py info %s]: %s", (const char*)this->p_name, msg);
    }
}


void PythonInterpreter::log_error(char* fmt, ...)
{
    if (this->p_log_level >= log_level::PY_ERROR) {
        char msg[PY_MAX_ELEMS];

        va_list va;
        va_start(va, fmt);
        vsnprintf(msg, PY_MAX_ELEMS, fmt, va);
        va_end(va);

        c74::max::error("[py error %s]: %s", (const char*)this->p_name, msg);
    }
}


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

        c74::max::error((char*)"[py %s] %s: %s", (const char*)this->p_name, msg, pvalue_str);
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


void PythonInterpreter::syspath_append(const char* path)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

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
    this->handle_error((char*)"syspath append failed");

finally:
    Py_XDECREF(expanded_path);
    Py_XDECREF(pre_expanded);
    Py_XDECREF(os_path_expandvars);
    Py_XDECREF(os_path);
    Py_XDECREF(os);
    PyGILState_Release(gstate);
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


atom PythonInterpreter::pobject_to_atom(PyObject* value)
{
    atom result = symbol();

    if (value == Py_True)
        result = 1;
    else if (value == Py_False)
        result = 0;
    else if (PyFloat_Check(value))
        result = (double)PyFloat_AsDouble(value);
    else if (PyLong_Check(value))
        result = (long)PyLong_AsLong(value);
    else if (PyUnicode_Check(value))
        result = symbol(PyUnicode_AsUTF8(value));
    // else if (value == Py_None);
    return result;
}


PyObject* PythonInterpreter::atoms_to_plist_with_offset(const atoms& args, int start_from)
{
    PyObject* plist = NULL; // python list

    if ((plist = PyList_New(0)) == NULL) {
        this->log_error((char*)"could not create an empty python list");
        goto error;
    }

    for (int i = start_from; i < (int)args.size(); i++) {
        switch (args[i].a_type) {
        case c74::max::A_FLOAT: {
            PyObject* p_float = PyFloat_FromDouble((double)args[i]);
            if (p_float == NULL) {
                goto error;
            }
            PyList_Append(plist, p_float);
            Py_DECREF(p_float);
            break;
        }
        case c74::max::A_LONG: {
            PyObject* p_long = PyLong_FromLong((long)args[i]);
            if (p_long == NULL) {
                goto error;
            }
            PyList_Append(plist, p_long);
            Py_DECREF(p_long);
            break;
        }
        case c74::max::A_SYM: {
            PyObject* p_str = PyUnicode_FromString(std::string(args[i]).c_str());
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


PyObject* PythonInterpreter::atoms_to_plist(const atoms& args)
{
    return this->atoms_to_plist_with_offset(args, 0);
}


PyObject* PythonInterpreter::atoms_to_ptuple(const atoms& args)
{
    PyObject* ptuple = PyTuple_New(args.size());
    for (int i = 0; i < (int)args.size(); i++) {
        PyObject* value = this->atom_to_pobject(args[i]);
        PyTuple_SetItem(ptuple, i, value); // pass value ref to the tuple
    }
    return ptuple;
}


atoms PythonInterpreter::plist_to_atoms(PyObject* plist)
{
    Py_ssize_t len = 0;
    Py_ssize_t i;
    PyObject* elem;
    atoms result;

    if (plist == NULL) {
        goto error;
    }

    if (PyList_Check(plist)) {
        len = PyList_Size(plist);
        result.reserve(len);
        for (i = 0; i < len; i++) {
            elem = PyList_GetItem(plist, i);
            result[i] = this->pobject_to_atom(elem);
        }
        return result;
    }

error:
    this->handle_error((char*)"plist_to_atoms failed");
    Py_XDECREF(plist);
    return {};
}


// ---------------------------------------------------------------------------
// output methods


void PythonInterpreter::handle_float_output(outlet<> *output, PyObject* pfloat)
{
    if (pfloat == NULL) {
        goto error;
    }

    if (PyFloat_Check(pfloat)) {
        float float_result = (float)PyFloat_AsDouble(pfloat);
        if (float_result == -1.0 && PyErr_Occurred()) {
            goto error;
        }
        output->send(float_result);
    }
    Py_XDECREF(pfloat);
    return;

error:
    this->handle_error((char*)"handle_float_output failed");
    Py_XDECREF(pfloat);
}


void PythonInterpreter::handle_long_output(outlet<> *output, PyObject* plong)
{
    if (plong == NULL) {
        goto error;
    }

    if (PyLong_Check(plong)) {
        long long_result = PyLong_AsLong(plong);
        if (long_result == -1 && PyErr_Occurred()) {
            goto error;
        }
        output->send(long_result);
    }

    Py_XDECREF(plong);
    return;

error:
    this->handle_error((char*)"handle_long_output failed");
    Py_XDECREF(plong);
}


void PythonInterpreter::handle_string_output(outlet<> *output, PyObject* pstring)
{
    if (pstring == NULL) {
        goto error;
    }

    if (PyUnicode_Check(pstring)) {
        const char* unicode_result = PyUnicode_AsUTF8(pstring);
        if (unicode_result == NULL) {
            goto error;
        }
        output->send(symbol(unicode_result));
    }

    Py_XDECREF(pstring);
    return;

error:
    this->handle_error((char*)"handle_string_output failed");
    Py_XDECREF(pstring);
}


void PythonInterpreter::handle_list_output(outlet<> *output, PyObject* plist)
{
    if (plist == NULL) {
        goto error;
    }

    if (PySequence_Check(plist) && !PyUnicode_Check(plist)
        && !PyBytes_Check(plist) && !PyByteArray_Check(plist)) {
        PyObject* iter = NULL;
        PyObject* item = NULL;
        int i = 0;

        Py_ssize_t seq_size = PySequence_Length(plist);
        this->log_debug((char*)"seq_size: %d", seq_size);

        atoms result(seq_size);

        if (seq_size == 0) {
            this->log_error((char*)"cannot convert py list of length 0 to atoms");
            goto error;
        }

        if ((iter = PyObject_GetIter(plist)) == NULL) {
            goto error;
        }

        while ((item = PyIter_Next(iter)) != NULL) {
            if (PyLong_Check(item)) {
                long long_item = PyLong_AsLong(item);
                if (long_item == -1 && PyErr_Occurred()) {
                    goto error;
                }
                result[i] = long_item;
                this->log_debug((char*)"%d long: %ld\n", i, long_item);
                i++;
            }

            if (PyFloat_Check(item)) {
                float float_item = PyFloat_AsDouble(item);
                if (float_item == -1.0 && PyErr_Occurred()) {
                    goto error;
                }
                result[i] = float_item;
                this->log_debug((char*)"%d float: %f\n", i, float_item);
                i++;
            }

            if (PyUnicode_Check(item)) {
                const char* unicode_item = PyUnicode_AsUTF8(item);
                if (unicode_item == NULL) {
                    goto error;
                }
                result[i] = symbol(unicode_item);
                this->log_debug((char*)"%d unicode: %s\n", i, unicode_item);
                i++;
            }
            Py_DECREF(item);
        }
        output->send(result);
    }

    Py_XDECREF(plist);
    return;

error:
    this->handle_error((char*)"handle_list_output failed");
    Py_XDECREF(plist);
}


void PythonInterpreter::handle_dict_output(outlet<> *output, PyObject* pdict)
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
            this->handle_list_output(output, pval); // this decrefs pval
            Py_XDECREF(pfun_co);
            return;
        } else {
            this->log_error((char*)"expected list output got something else");
            goto error;
        }
    }

error:
    this->handle_error((char*)"handle_dict_output failed");
    Py_XDECREF(pfun_co);
    Py_XDECREF(pval);
}


void PythonInterpreter::handle_output(outlet<> *output, PyObject* pval)
{
    if (pval == NULL) {
        this->log_error((char*)"cannot handle NULL value");
        return;
    }

    if (PyFloat_Check(pval)) {
        return this->handle_float_output(output, pval);
    }

    else if (PyLong_Check(pval)) {
        return this->handle_long_output(output, pval);
    }

    else if (PyUnicode_Check(pval)) {
        return this->handle_string_output(output, pval);
    }

    else if (PySequence_Check(pval) && !PyBytes_Check(pval)
             && !PyByteArray_Check(pval)) {
        return this->handle_list_output(output, pval);
    }

    else if (PyDict_Check(pval)) {
        return this->handle_dict_output(output, pval);
    }

    else if (pval == Py_None) {
        return;
    }

    else {
        this->log_error((char*)"cannot handle this type of value");
    }
}


// ---------------------------------------------------------------------------------------
// CORE METHOD HELPERS


void PythonInterpreter::import_module(const char* module)
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
    return;

error:
    this->handle_error((char*)"import %s", module);
    PyGILState_Release(gstate);
}


PyObject* PythonInterpreter::eval_pcode(const char* pcode)
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


void PythonInterpreter::exec_pcode(const char* pcode)
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
    return;

error:
    this->handle_error((char*)"exec %s", pcode);
    Py_XDECREF(pval);
    PyGILState_Release(gstate);
}


void PythonInterpreter::execfile_path(const char* path)
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
    return;

error:
    this->handle_error((char*)"execfile");
    Py_XDECREF(pval);
    PyGILState_Release(gstate);
}


// ---------------------------------------------------------------------------------------
// CORE METHODS


void PythonInterpreter::import(const char* module)
{
    return this->import_module(module);
}

void PythonInterpreter::import(const atoms& args)
{
    if (args.size() == 0 || args[0].a_type != c74::max::A_SYM ) {
        this->log_error((char*)"no import module provided");
        return;
    }
    const char* module = (const char*)symbol(args[0]);
    return this->import_module(module);
}


void PythonInterpreter::eval(const char* code, outlet<> *output)
{
    PyObject* pval = this->eval_pcode(code);
    this->handle_output(output, pval);
}

void PythonInterpreter::eval(const atoms& args, outlet<> *output)
{
    if (args.size() == 0 || args[0].a_type != c74::max::A_SYM ) {
        this->log_error((char*)"no eval args provided");
        return;
    }
    const char* code = (const char*)symbol(args[0]);
    this->eval(code, output);
}


void PythonInterpreter::exec(const char* code)
{
    return this->exec_pcode(code);
}

void PythonInterpreter::exec(const atoms& args)
{
    if (args.size() == 0 || args[0].a_type != c74::max::A_SYM ) {
        this->log_error((char*)"no exec args provided");
        return;
    }
    const char* code = (const char*)symbol(args[0]);
    this->exec_pcode(code);
}


void PythonInterpreter::execfile(const char* path)
{
    if (path != NULL) {
        // set path to this->p_source_path
        if (!this->locate_path(path)) {
            this->log_error((char*)"could not locate path from symbol");
            return;
        }
    }

    if (path == NULL || this->p_source_path.empty()) {
        this->log_error((char*)"could not set filepath");
        return;
    }

    // assume this->p_source_path has be been set without errors
    this->execfile_path((const char*)this->p_source_path);
}

void PythonInterpreter::execfile(const atoms& args)
{
    if (args.size() == 0 || args[0].a_type != c74::max::A_SYM ) {
        this->log_error((char*)"no execfile path provided");
        return;
    }
    const char* path = (const char*)symbol(args[0]);
    this->execfile(path);
}

// ---------------------------------------------------------------------------------------
// EXTRA METHODS HELPERS


PyObject* PythonInterpreter::eval_text(char* text)
{
    PyGILState_STATE gstate = PyGILState_Ensure();

    int is_eval = 1;
    PyObject* co = NULL;
    PyObject* pval = NULL;
    const char * name = this->p_name;

    co = Py_CompileString(text, name, Py_eval_input);

    if (PyErr_ExceptionMatches(PyExc_SyntaxError)) {
        PyErr_Clear();
        co = Py_CompileString(text, name, Py_single_input);
        is_eval = 0;
    }

    if (co == NULL) { // can be eval-co or exec-co or NULL here
        c74::max::sysmem_freeptr(text);
        goto error;
    }
    c74::max::sysmem_freeptr(text);

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


void PythonInterpreter::eval_text_to_outlet(const atoms& args, int offset, outlet<> *output)
{
    long textsize = 0;
    char* text = NULL;

    c74::max::t_max_err err = c74::max::atom_gettext((long)args.size() + (long)offset,
        (c74::max::t_atom*)&args[0], &textsize, &text,
        c74::max::OBEX_UTIL_ATOM_GETTEXT_DEFAULT);

    if (err == c74::max::MAX_ERR_NONE && textsize && text) {
        this->log_debug((char*)">>> %s", text);
        PyObject* pval = this->eval_text(text);
        if (pval != NULL) {
            this->handle_output(output, pval);
            return;
        }
    }
    this->log_error((char*)"eval_text_to_outlet failed");
}


// ---------------------------------------------------------------------------------------
// EXTRA METHODS


void PythonInterpreter::call(const atoms& args, outlet<> *output)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    const char* callable_name = NULL;
    PyObject* py_argslist = NULL;
    PyObject* pval = NULL;
    PyObject* py_callable = NULL;
    // python list
    PyObject* py_args = NULL; // python tuple

    // first atom in argv must be a symbol
    if (args[0].a_type != c74::max::A_SYM) {
        this->log_error((char*)"first atom must be a symbol!");
        goto error;
    }

    callable_name = (const char*)symbol(args[0]);
    this->log_debug((char*)"callable_name: %s", callable_name);

    py_callable = PyRun_String(callable_name, Py_eval_input, this->p_globals,
                               this->p_globals);
    if (py_callable == NULL) {
        this->log_error((char*)"could not evaluate %s", callable_name);
        goto error;
    }

    py_argslist = this->atoms_to_plist_with_offset(args, 1);
    if (py_argslist == NULL) {
        this->log_error((char*)"atom to py list conversion failed");
        goto error;
    }

    this->log_debug((char*)"length of argc:%ld list: %d", args.size(),
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
    this->handle_output(output, pval);
    // success cleanup
    Py_XDECREF(py_callable);
    Py_XDECREF(py_argslist);
    PyGILState_Release(gstate);
    return;

error:
    this->handle_error((char*)"call method");
    // cleanup
    Py_XDECREF(py_callable);
    Py_XDECREF(py_argslist);
    Py_XDECREF(pval);
    PyGILState_Release(gstate);
}


void PythonInterpreter::assign(const atoms& args)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    const char* varname = NULL;
    PyObject* list = NULL;
    int res;

    // first atom in argv must be a symbol
    if (args[0].a_type != c74::max::A_SYM) {
        this->log_error((char*)"first atom must be a symbol!");
        goto error;
    } 
    
    varname = (const char*)symbol(args[0]);
    this->log_debug((char*)"varname: %s", varname);

    list = this->atoms_to_plist_with_offset(args, 1);
    if (list == NULL) {
        this->log_error((char*)"atom to py list conversion failed");
        goto error;
    }

    if (PyList_Size(list) != (long)args.size() - 1) {
        this->log_error((char*)"PyList_Size(list) != argc - 1");
        goto error;
    }
    this->log_debug((char*)"length of list: %d", PyList_Size(list));
    

    // finally, assign list to varname in object namespace
    this->log_debug((char*)"setting %s to list in namespace", varname);
    // following does not steal ref to list
    res = PyDict_SetItemString(this->p_globals, varname, list);
    if (res != 0) {
        this->log_error((char*)"assign varname to list failed");
        goto error;
    }
    PyGILState_Release(gstate);
    return;

error:
    this->handle_error((char*)"assign method");
    Py_XDECREF(list);
    PyGILState_Release(gstate);
}


void PythonInterpreter::code(const atoms& args, outlet<> *output)
{
    this->eval_text_to_outlet(args, 0, output);
}


void PythonInterpreter::anything(symbol s, const atoms& args, outlet<> *output)
{
    atoms result(PY_MAX_ELEMS);

    const char* pythonpath = NULL;
    int argc = (int)args.size();

    if (s.empty()) {
        this->log_error((char*)"no selector provided");
        return;
    }

    // set '=' as shorthand for assign method
    else if (s == symbol("=")) {
        return this->assign(args);
    }

    // check for properties
    else if (s == symbol("pythonpath")) {
        if (argc == 0) {
            this->log_info((char*)"property pythonpath: %s", 
                (const char*)this->p_pythonpath);
            return;
        }
        
        if (argc == 1) {
            if (args[0].a_type == c74::max::A_SYM) {
                pythonpath = (const char*)symbol(args[0]);
                this->log_info((char*)"setting pythonpath to %s", pythonpath);
                this->p_pythonpath = symbol(pythonpath);
                this->syspath_append((const char*)pythonpath);
                return;
            }
        }
        this->log_error((char*)"could not get/set pythonpath");
        return;
    }

    else if (s == symbol("log_level")) {
        if (argc == 0) {
            this->log_info((char*)"property log_level: %d", 
                            this->p_log_level);
            return;
        }

        if (argc == 1) {
            if (args[0].a_type == c74::max::A_LONG) {
                long level = (long)args[0];
                this->log_info((char*)"setting log_level to %d", level);
                this->p_log_level = (log_level)level;
                return;
            }
        }
        this->log_error((char*)"ncould not get/set log_level");
        return;
    }
    
    else {

        // set symbol as first atom in new atoms array
        result[0] = s;

        for (int i = 0; i < argc; i++) {
            switch (args[i].a_type) {
            case c74::max::A_FLOAT: {
                result[i+1] = (double)args[i];
                break;
            }
            case c74::max::A_LONG: {
                result[i+1] = (long)args[i];
                break;
            }
            case c74::max::A_SYM: {
                result[i+1] = symbol(args[i]);
                break;
            }
            default:
                this->log_debug((char*)"cannot process unknown type");
                break;
            }
        }

        this->eval_text_to_outlet(args, 1, output);
    }
}


void PythonInterpreter::pipe(const atoms& args, outlet<> *output)
{
    atoms result;
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    long textsize = 0;
    char* text = NULL;
    c74::max::t_max_err err;
    PyObject* pipe_pre = NULL;
    PyObject* pipe_fun = NULL;
    PyObject* pval = NULL;
    PyObject* pstr = NULL;

    err = c74::max::atom_gettext((long)args.size(), (c74::max::t_atom*)&args[0],
        &textsize, &text, c74::max::OBEX_UTIL_ATOM_GETTEXT_DEFAULT);
    if (err != c74::max::MAX_ERR_NONE || !textsize || !text) {
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

    c74::max::sysmem_freeptr(text);

    pipe_fun = PyDict_GetItemString(this->p_globals, "__py_maxmsp_pipe");
    if (pipe_fun == NULL) {
        this->log_error((char*)"retrieving pipe func from globals failed");
        goto error;
    }

    pval = PyObject_CallFunctionObjArgs(pipe_fun, pstr, NULL);
    if (pval == NULL) {
        this->log_error((char*)"call python function failed");
        goto error;
    }

    if (!PyUnicode_Check(pval)) {
        this->handle_output(output, pval); // this decrefs pval
    } else {
        // special case strings, which will cause crash if handled
        // out of this methods's scope. (huge PITA to debug!)
        const char* unicode_result = PyUnicode_AsUTF8(pval);
        if (unicode_result == NULL) {
            goto error;
        }
        result.push_back(symbol(unicode_result));
        output->send(result);
        // outlet_anything(outlet, gensym(unicode_result), 0, (t_atom*)NIL);
        Py_XDECREF(pval);
    }

    Py_XDECREF(pipe_pre);
    Py_XDECREF(pstr);
    PyGILState_Release(gstate);
    return;


error:
    this->handle_error((char*)"pipe failed");
    Py_XDECREF(pipe_pre);
    Py_XDECREF(pstr);
    Py_XDECREF(pval);
    PyGILState_Release(gstate);
}


// ---------------------------------------------------------------------------------------
// datastructure support


bool PythonInterpreter::table_exists(const char* table_name)
{
    long **storage, size;

    return (c74::max::table_get(symbol(table_name), &storage, &size) == 0);
}


void PythonInterpreter::plist_to_table(const char* table_name, PyObject* plist)
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

    if (c74::max::table_get(symbol(table_name), &storage, &size)) {
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
    return;

error:
    this->handle_error((char*)"plist to table failed");
    Py_XDECREF(plist);
    Py_XDECREF(elem);
}


PyObject* PythonInterpreter::table_to_plist(const char* table_name)
{
    PyObject* plist = NULL;
    long **storage, size, value;

    if ((plist = PyList_New(0)) == NULL) {
        this->log_error((char*)"could not create an empty python list");
        goto error;
    }

    if (c74::max::table_get(symbol(table_name), &storage, &size) == 0) {
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

// ----------------------------------------------------------------------------
// path helpers


bool PythonInterpreter::locate_path(const char* path)
{
    c74::max::t_fourcc filetype = FOUR_CHAR_CODE('TEXT');
    c74::max::t_fourcc outtype = 0;
    short path_code = 0;
    char filename[c74::max::MAX_PATH_CHARS];
    char pathname[c74::max::MAX_PATH_CHARS];
    bool ret = false;
    // c74::max::t_max_err ret = c74::max::MAX_ERR_NONE;

    if (path == NULL) { // if no arg supplied to ask for file
        filename[0] = 0;

        if (c74::max::open_dialog(filename, &path_code,
                        &outtype, &filetype, 1))
            // non-zero: cancelled
        goto finally;

    } else {
        // must copy symbol before calling locatefile_extended
        c74::max::strncpy_zero(filename, path, c74::max::MAX_PATH_CHARS);
        if (c74::max::locatefile_extended(filename, &path_code,
                                &outtype, &filetype, 1)) {
            // nozero: not found
            this->log_error((char*)"can't find file %s", path);
            goto finally;
        } else {
            ret = true;
            pathname[0] = 0;
            ret = c74::max::path_toabsolutesystempath(path_code, filename, pathname);
            if (ret != c74::max::MAX_ERR_NONE) {
                this->log_error((char*)"can't convert %s to absolutepath", path);
                ret = false;
                goto finally;
            }
        }

        // success
        // set attribute from pathname symbol
        this->log_debug((char*)"filename: %s", filename);
        this->log_debug((char*)"pathname: %s", pathname);
        this->p_source_name = symbol(filename);
        this->p_source_path = symbol(pathname);
        assert(ret);
    }

finally:
    return ret;
}



std::string get_path_to_external(c74::max::t_class* c, const char* subpath)
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



std::string get_path_to_package(c74::max::t_class* c, const char* subpath)
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