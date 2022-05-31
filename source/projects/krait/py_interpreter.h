/** \file py_interpreter.h
    \brief A single-header python3 library for Max externals.

    Provides a `PythonInterpreter` implementation which can be dropped into any
    arbitrary Max external. 

    The latest code can be found in [py-js](https://github.com/shakfu/py-js)

    If PY_INTERPRETER_IMPLEMENTATION is defined before including the header,
    it will activate the implementation, otherwise the implementation
    will not be included, this file is treated as a header.

    Usage example:

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


class PythonInterpreter
{
    private:
        t_symbol* p_name;                       //!< unique python object name
        t_symbol* p_pythonpath;                 //!< path to python directory
        t_bool p_debug;                         //!< bool to switch per-object debug state
        t_fourcc p_code_filetype;               //!< filetype four char code of 'TEXT'
        t_fourcc p_code_outtype;                //!< filetype four char code of 'TEXT'
        char p_code_filename[MAX_PATH_CHARS];   //!< file name field
        char p_code_pathname[MAX_PATH_CHARS];   //!< file path field
        short p_code_path;                      //!< short code for max file system
        t_symbol* p_code_filepath;              //!< filepath to python file to execfile
        PyObject* p_globals;                    //!< per object 'globals' python namespace

    public:
        PythonInterpreter();
        ~PythonInterpreter();

        // helpers
        void log_debug(char* fmt, ...);
        void log_error(char* fmt, ...);
        void handle_error(char* fmt, ...);

        t_max_err locate_path_from_symbol(t_symbol* s);
        t_max_err eval_text_to_outlet(long argc, t_atom* argv, int offset, void* outlet);
        PyObject* eval_pcode_to_pval(char* pcode);

        // py <-> atom translation
        PyObject* atoms_to_plist_with_offset(long argc, t_atom* argv, int start_from);
        PyObject* atoms_to_plist(long argc, t_atom* argv); //+
        t_max_err plist_to_atoms(PyObject* seq, int* argc, t_atom** argv); //+
        PyObject* atoms_to_ptuple(int argc, t_atom* argv); //+

        PyObject* atom_to_pobject(t_atom* atom); // used by atoms_to_ptuple
        t_max_err pobject_to_atom(PyObject* value, t_atom* atom); // used by plist_to_atoms

        // translation -> output        
        t_max_err handle_float_output(void* outlet, PyObject* pval);
        t_max_err handle_long_output(void* outlet, PyObject* pval);
        t_max_err handle_string_output(void* outlet, PyObject* pval);
        t_max_err handle_list_output(void* outlet, PyObject* pval);
        t_max_err handle_dict_output(void* outlet, PyObject* pval);
        t_max_err handle_output(void* outlet, PyObject* pval);

        // core
        t_max_err import(t_symbol* s);
        t_max_err eval(t_symbol* s, long argc, t_atom* argv, void* outlet);
        t_max_err exec(t_symbol* s, long argc, t_atom* argv);
        t_max_err execfile(t_symbol* s);

        // extra
        t_max_err call(t_symbol* s, long argc, t_atom* argv, void* outlet);
        t_max_err assign(t_symbol* s, long argc, t_atom* argv);
        t_max_err code(t_symbol* s, long argc, t_atom* argv, void* outlet);
        t_max_err anything(t_symbol* s, long argc, t_atom* argv, void* outlet);
        t_max_err pipe(t_symbol* s, long argc, t_atom* argv, void* outlet);
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
// constants

#define PY_MAX_ATOMS 128
#define PY_MAX_LOG_CHAR 500
#define PY_MAX_ERR_CHAR PY_MAX_LOG_CHAR
#define PY_DEBUG 1


// ---------------------------------------------------------------------------
// constructor / destructor methods

/**
 * @brief      Constructs a new PythonInterpreter instance.
 */
PythonInterpreter::PythonInterpreter()
{
    this->p_name = symbol_unique();
    this->p_pythonpath = gensym("");
    this->p_debug = PY_DEBUG;

    this->p_code_filetype = FOUR_CHAR_CODE('TEXT');
    this->p_code_outtype = 0;
    this->p_code_filename[0] = 0;
    this->p_code_pathname[0] = 0;
    this->p_code_path = 0;
    this->p_code_filepath = gensym("");

    // python init

    Py_Initialize();

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
    if (this->p_debug) {
        char msg[PY_MAX_LOG_CHAR];

        va_list va;
        va_start(va, fmt);
        vsprintf(msg, fmt, va);
        va_end(va);

        post("[py %s]: %s", this->p_name->s_name, msg);
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
    char msg[PY_MAX_ERR_CHAR];

    va_list va;
    va_start(va, fmt);
    vsprintf(msg, fmt, va);
    va_end(va);

    error((char*)"[py %s]: %s", this->p_name->s_name, msg);
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
        char msg[PY_MAX_ERR_CHAR];

        va_list va;
        va_start(va, fmt);
        vsprintf(msg, fmt, va);
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


/**
 * @brief Searches the Max filesystem context for a file given by a symbol
 *
 * @param s symbol to be searched
 * @return t_max_err
 */
t_max_err PythonInterpreter::locate_path_from_symbol(t_symbol* s)
{
    t_max_err ret = 0;

    if (s == gensym("")) { // if no arg supplied ask for file
        this->p_code_filename[0] = 0;

        if (open_dialog(this->p_code_filename, &this->p_code_path,
                        &this->p_code_outtype, &this->p_code_filetype, 1))
            // non-zero: cancelled
            ret = MAX_ERR_GENERIC;
        goto finally;

    } else {
        // must copy symbol before calling locatefile_extended
        strncpy_zero(this->p_code_filename, s->s_name, MAX_PATH_CHARS);
        if (locatefile_extended(this->p_code_filename, &this->p_code_path,
                                &this->p_code_outtype, &this->p_code_filetype, 1)) {
            // nozero: not found
            this->log_error((char*)"can't find file %s", s->s_name);
            ret = MAX_ERR_GENERIC;
            goto finally;
        } else {
            this->p_code_pathname[0] = 0;
            ret = path_toabsolutesystempath(this->p_code_path, this->p_code_filename,
                                            this->p_code_pathname);
            if (ret != MAX_ERR_NONE) {
                this->log_error((char*)"can't convert %s to absolutepath", s->s_name);
                goto finally;
            }
        }

        // success
        // set attribute from pathname symbol
        this->p_code_filepath = gensym(this->p_code_pathname);
        assert(ret == MAX_ERR_NONE);
    }

finally:
    return ret;
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
        atom_setfloat(atom, (float)PyFloat_AsDouble(value));
    else if (PyLong_Check(value))
        atom_setlong(atom, (float)PyLong_AsLong(value));
    else if (PyUnicode_Check(value))
        atom_setsym(atom, gensym(PyUnicode_AsUTF8(value)));
    else {
        // FIXME: should this not return an 'error' t_symbol
        this->log_error((char*)"Warning: python type unsupported for conversion to max t_atom.");
        atom_setsym(atom, gensym("error"));
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

        t_atom atoms_static[PY_MAX_ATOMS];
        t_atom* atoms = NULL;
        int is_dynamic = 0;

        Py_ssize_t seq_size = PySequence_Length(plist);
        this->log_debug((char*)"seq_size: %d", seq_size);

        if (seq_size == 0) {
            this->log_error((char*)"cannot convert py list of length 0 to atoms");
            goto error;
        }

        if (seq_size > PY_MAX_ATOMS) {
            this->log_debug((char*)"dynamically increasing size of atom array");
            atoms = atom_dynamic_start(atoms_static, PY_MAX_ATOMS,
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
// CORE METHODS


/**
 * @brief Import a python module
 *
 * @param s symbol of module to be imported
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::import(t_symbol* s)
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
        PyDict_SetItemString(this->p_globals, s->s_name, x_module);
        PyGILState_Release(gstate);
        this->log_debug((char*)"imported: %s", s->s_name);
    }
    return MAX_ERR_NONE;

error:
    this->handle_error((char*)"import %s", s->s_name);
    PyGILState_Release(gstate);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Evaluate a max symbol as a python expression
 *
 * @param s symbol of object to be evaluated
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param outlet object outlet
 *
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::eval(t_symbol* s, long argc, t_atom* argv, void* outlet)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    char* py_argv = atom_getsym(argv)->s_name;
    this->log_debug((char*)"%s %s", s->s_name, py_argv);

    PyObject* pval = PyRun_String(py_argv, Py_eval_input, this->p_globals,
                                  this->p_globals);

    if (pval != NULL) {
        this->handle_output(outlet, pval);
        PyGILState_Release(gstate);
        return MAX_ERR_NONE;
    } else {
        this->handle_error((char*)"eval %s", py_argv);
        PyGILState_Release(gstate);
        return MAX_ERR_GENERIC;
    }
}

/**
 * @brief Execute a max symbol as a line of python code
 *
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::exec(t_symbol* s, long argc, t_atom* argv)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    char* py_argv = NULL;
    PyObject* pval = NULL;

    py_argv = atom_getsym(argv)->s_name;
    if (py_argv == NULL) {
        goto error;
    }

    pval = PyRun_String(py_argv, Py_single_input, this->p_globals, this->p_globals);
    if (pval == NULL) {
        goto error;
    }
    Py_DECREF(pval);
    PyGILState_Release(gstate);

    this->log_debug((char*)"exec %s", py_argv);
    return MAX_ERR_NONE;

error:
    this->handle_error((char*)"exec %s", py_argv);
    Py_XDECREF(pval);
    PyGILState_Release(gstate);
    return MAX_ERR_GENERIC;
}

/**
 * @brief Execute contents of a file (obtained from symbol) as python code
 *
 * @param s symbol
 * @return t_max_err error code
 */
t_max_err PythonInterpreter::execfile(t_symbol* s)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    PyObject* pval = NULL;
    FILE* fhandle = NULL;

    if (s != gensym("")) {
        // set this->p_code_filepath
        t_max_err err = this->locate_path_from_symbol(s);
        if (err != MAX_ERR_NONE) {
            this->log_error((char*)"could not locate path from symbol");
            goto error;
        }
    }

    if (s == gensym("") || this->p_code_filepath == gensym("")) {
        this->log_error((char*)"could not set filepath");
        goto error;
    }

    // assume this->p_code_filepath has be been set without errors

    this->log_debug((char*)"pathname: %s", this->p_code_filepath->s_name);
    fhandle = fopen(this->p_code_filepath->s_name, "r+");

    if (fhandle == NULL) {
        this->log_error((char*)"could not open file");
        goto error;
    }

    pval = PyRun_File(fhandle, this->p_code_filepath->s_name, Py_file_input,
                      this->p_globals, this->p_globals);
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
    PyGILState_STATE gstate = PyGILState_Ensure();

    long textsize = 0;
    char* text = NULL;
    int is_eval = 1;
    PyObject* co = NULL;
    PyObject* pval = NULL;


    t_max_err err = atom_gettext(argc + offset, argv, &textsize, &text,
                                 OBEX_UTIL_ATOM_GETTEXT_DEFAULT);
    if (err == MAX_ERR_NONE && textsize && text) {
        this->log_debug((char*)">>> %s", text);
    } else {
        goto error;
    }

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
        // bang for exec-type op
        PyGILState_Release(gstate);
    } else {
        this->handle_output(outlet, pval);
    }
    return MAX_ERR_NONE;

error:
    this->handle_error((char*)"python code evaluation failed");
    PyGILState_Release(gstate);
    return MAX_ERR_GENERIC;
}


PyObject* PythonInterpreter::eval_pcode_to_pval(char* pcode)
{
    PyObject* pval = PyRun_String(pcode,
        Py_eval_input, this->p_globals, this->p_globals);

    if (pval == NULL) {
        this->log_error((char*)"python code evaluation result is NULL");
        goto error;
    }
    return pval;  

error:
    this->handle_error((char*)"failed evaluation");
    Py_XDECREF(pval);
    Py_RETURN_NONE;
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
    t_atom atoms[PY_MAX_ATOMS];

    if (s == gensym("")) {
        return MAX_ERR_GENERIC;
    }

    // set '=' as shorthand for assign method
    if (s == gensym("=")) {
        this->assign(gensym(""), argc, argv);
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
            this->log_debug((char*)"cannot process unknown type");
            break;
        }
    }

    return this->eval_text_to_outlet(argc, atoms, 1, outlet);
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

// ===========================================================================

#endif // PY_INTERPRETER_IMPLEMENTATION

} // namespace pyjs