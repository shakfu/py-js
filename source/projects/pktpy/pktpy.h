#ifndef PKTPY_INTERPRETER_H
#define PKTPY_INTERPRETER_H

#include "ext.h"
#include "ext_obex.h"

#include <fstream>
#include <wordexp.h>

#define PK_USER_CONFIG_H
#include "pocketpy.h"

using namespace pkpy;


// ---------------------------------------------------------------------------
// HEADER

/**
 * @brief      constants
 */
#define PY_MAX_ELEMS 1024
#define PY_LOG_LEVEL DEBUG


/**
 * @brief      specifies three logging levels
 */
enum log_level { ERROR, INFO, DEBUG };


/**
 * @brief      This class describes a pocketpy interpreter.
 */
class PktpyInterpreter {
private:
    t_symbol* name;        //!< unique python object name
    t_symbol* pythonpath;  //!< path to python directory
    log_level loglevel;    //!< object-level log level (error, info, debug)

public:
    VM* vm;                //!< pocketpy vm instance
    t_symbol* source_name; //!< base name of python file to execfile
    t_symbol* source_path; //!< full path to python file to execfile
    short     path_code;    

    PktpyInterpreter();
    ~PktpyInterpreter();

    // logging methods
    void log_debug(char* fmt, ...);
    void log_info(char* fmt, ...);
    void log_error(char* fmt, ...);
    void print_atom(int argc, t_atom* argv);

    // error handling methods


    // path handling methods
    // t_max_err syspath_append(char* path);
    t_max_err set_working_directory(char* path);
    t_max_err set_scripts_path(t_class* klass);
    t_max_err locate_path_from_symbol(t_symbol* s);
    t_symbol* get_path_to_external(t_class* klass);

    // simple python <-> atom type translation helpers
    PyObject* atom_to_pobject(t_atom* atom);                  // used by atoms_to_ptuple
    t_max_err pobject_to_atom(PyObject* value, t_atom* atom); // used by plist_to_atoms

    // complex python <-> atom type translation helpers
    List atoms_to_plist_with_offset(long argc, t_atom* argv, int start_from);
    List atoms_to_plist(long argc, t_atom* argv);
    t_max_err plist_to_atoms(List seq, int* argc, t_atom** argv);
    // Tuple atoms_to_ptuple(int argc, t_atom* argv);

    // python value -> atom -> outlet
    t_max_err handle_pyvar_output(PyObject* pval, void* outlet);
    t_max_err handle_plist_output(List plist, void* outlet);
    // t_max_err handle_dict_output(PyObject* pval, void* outlet);

    // core message method helpers
    t_max_err eval_pcode(char* pcode, void* outlet);
    t_max_err exec_pcode(char* pcode);
    t_max_err execfile_path(char* path);
    
    // anything message method helpers
    PyObject* eval_text(char* text);
    t_max_err eval_text_to_outlet(long argc, t_atom* argv, int offset, void* outlet);

    // core message methods
    t_max_err eval(t_symbol* s, long argc, t_atom* argv, void* outlet);
    t_max_err exec(t_symbol* s, long argc, t_atom* argv);
    t_max_err anything(t_symbol* s, long argc, t_atom* argv, void* outlet);
    t_max_err execfile(t_symbol* s);
};

// ---------------------------------------------------------------------------
// constructor / destructor

/**
 * @brief      Constructs a new PktpyInterpreter instance.
 */
PktpyInterpreter::PktpyInterpreter()
{
    this->name = symbol_unique();
    this->pythonpath = gensym("");
    this->source_name = gensym("");
    this->source_path = gensym("");
    this->path_code = 0;
    this->loglevel = log_level::PY_LOG_LEVEL;
    this->vm = new VM(false); // vm->enable_os = false
    this->vm->_stdout = [](VM* vm, const Str& s) {};
    this->vm->_stderr = [](VM* vm, const Str& s) {};

    // this->vm->_stdout = [](VM* vm, const Str& s) { 
    //     const char* stdout_output = s.c_str_dup();
    //     post((char*)stdout_output);
    // };
    // this->vm->_stderr = [](VM* vm, const Str& s) { 
    //     const char* stderr_output = s.c_str_dup();
    //     error((char*)stderr_output);
    // };
}


/**
 * @brief      PktpyInterpreter destructor method.
 */
PktpyInterpreter::~PktpyInterpreter() { delete(this->vm); }


// ---------------------------------------------------------------------------
// logging methods


/**
 * @brief Post msg to Max console.
 *
 * @param fmt character string with format codes
 * @param ... other arguments
 */
void PktpyInterpreter::log_debug(char* fmt, ...)
{
    if (this->loglevel >= log_level::DEBUG) {
        char msg[PY_MAX_ELEMS];

        va_list va;
        va_start(va, fmt);
        vsnprintf(msg, PY_MAX_ELEMS, fmt, va);
        va_end(va);

        post("[pktpy debug %s]: %s", this->name->s_name, msg);
    }
}


/**
 * @brief Post msg to Max console.
 *
 * @param fmt character string with format codes
 * @param ... other arguments
 */
void PktpyInterpreter::log_info(char* fmt, ...)
{
    if (this->loglevel >= log_level::INFO) {
        char msg[PY_MAX_ELEMS];

        va_list va;
        va_start(va, fmt);
        vsnprintf(msg, PY_MAX_ELEMS, fmt, va);
        va_end(va);

        post("[pktpy info %s]: %s", this->name->s_name, msg);
    }
}


/**
 * @brief Post error message to Max console.
 *
 * @param fmt character string with format codes
 * @param ... other arguments
 */
void PktpyInterpreter::log_error(char* fmt, ...)
{
    if (this->loglevel >= log_level::ERROR) {
        char msg[PY_MAX_ELEMS];

        va_list va;
        va_start(va, fmt);
        vsnprintf(msg, PY_MAX_ELEMS, fmt, va);
        va_end(va);

        error((char*)"[pktpy error %s]: %s", this->name->s_name, msg);
    }
}

/**
 * @brief      Prints an atom array to the console.
 *
 * @param[in]  argc  The count of arguments
 * @param      argv  The arguments atom array
 */
void PktpyInterpreter::print_atom(int argc, t_atom* argv)
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

// ---------------------------------------------------------------------------
// error handling methods


// ---------------------------------------------------------------------------
// path handling methods

/**
 * @brief      Sets the working directory.
 *
 * @param      path  The path
 *
 * @return     t_max_err
 */
t_max_err PktpyInterpreter::set_working_directory(char* path)
{
    std::filesystem::path p(path);
    if (std::filesystem::exists(p)) {
        std::filesystem::current_path(p);
        return MAX_ERR_NONE;
    }
    return MAX_ERR_GENERIC;
}


t_max_err PktpyInterpreter::set_scripts_path(t_class* klass)
{
    char external_path[MAX_PATH_CHARS];
    char external_name[MAX_PATH_CHARS];
    char conform_path[MAX_PATH_CHARS];

    short path_id = class_getpath(klass);
    snprintf_zero(external_name, PY_MAX_ELEMS, "%s.mxo", klass->c_sym->s_name);
    path_toabsolutesystempath(path_id, external_name, external_path);
    path_nameconform(external_path, conform_path, PATH_STYLE_NATIVE,
                     PATH_TYPE_TILDE);

    std::filesystem::path p (external_path);
    std::filesystem::path scripts = p.parent_path().parent_path() / "source/projects/pktpy/scripts";
    if (std::filesystem::exists(scripts)) {
        std::filesystem::current_path(scripts);
        return MAX_ERR_NONE;
    }
    return MAX_ERR_GENERIC;
}


/**
 * @brief Searches the Max filesystem context for a file given by a symbol
 *
 * @param s symbol to be searched
 * @return t_max_err
 */
t_max_err PktpyInterpreter::locate_path_from_symbol(t_symbol* s)
{
    t_fourcc filetype = FOUR_CHAR_CODE('TEXT');
    t_fourcc outtype = 0;
    // short path_code = 0;
    char filename[MAX_PATH_CHARS];
    char pathname[MAX_PATH_CHARS];
    t_max_err ret = MAX_ERR_NONE;

    if (s == gensym("")) { // if no arg supplied to ask for file
        filename[0] = 0;

        if (open_dialog(filename, &this->path_code, &outtype, &filetype, 1))
            // non-zero: cancelled
            ret = MAX_ERR_GENERIC;
        goto finally;

    } else {
        // tilde expansion in path
        wordexp_t exp_result;
        wordexp(s->s_name, &exp_result, 0);
        // must copy symbol before calling locatefile_extended
        strncpy_zero(filename, exp_result.we_wordv[0], MAX_PATH_CHARS);
        wordfree(&exp_result);

        if (locatefile_extended(filename, &this->path_code, &outtype, &filetype,
                                1)) {
            // nozero: not found
            this->log_error((char*)"can't find file %s", s->s_name);
            ret = MAX_ERR_GENERIC;
            goto finally;
        } else {
            pathname[0] = 0;
            ret = path_toabsolutesystempath(this->path_code, filename, pathname);
            if (ret != MAX_ERR_NONE) {
                this->log_error((char*)"can't convert %s to absolutepath",
                                s->s_name);
                goto finally;
            }
        }

        // success
        // set attribute from pathname symbol
        this->log_debug((char*)"filename: %s", filename);
        this->log_debug((char*)"pathname: %s", pathname);
        this->source_name = gensym(filename);
        this->source_path = gensym(pathname);
        assert(ret == MAX_ERR_NONE);
    }

finally:
    return ret;
}


/**
 * @brief      Gets the path to external.
 *
 * @param      klass a pointer to a t_class instance
 *
 * @return     The path to external as a t_symbol pointer
 */
t_symbol* PktpyInterpreter::get_path_to_external(t_class* klass)
{
    char external_path[MAX_PATH_CHARS];
    char external_name[MAX_PATH_CHARS];
    char conform_path[MAX_PATH_CHARS];

    short path_id = class_getpath(klass);
    snprintf_zero(external_name, PY_MAX_ELEMS, "%s.mxo", klass->c_sym->s_name);
    path_toabsolutesystempath(path_id, external_name, external_path);
    path_nameconform(external_path, conform_path, PATH_STYLE_NATIVE,
                     PATH_TYPE_TILDE);
    this->log_debug((char*)
        "path_id: %d, external_name: %s, external_path: %s conform_path: %s",
        path_id, external_name, external_path, conform_path);
    return gensym(external_path);
}

// ---------------------------------------------------------------------------
// simple type translation methods

/**
 * @brief      Converts max atom to python object
 *
 * @param      atom  The atom
 *
 * @return     python object (int, float, string)
 */
PyObject* PktpyInterpreter::atom_to_pobject(t_atom* atom)
{
    this->log_debug((char*)"py_atom_to_py_object start");

    switch (atom->a_type) {

    case A_LONG:
        this->log_debug((char*)"int: %i", atom_getlong(atom));
        return py_var(this->vm, atom_getlong(atom));

    case A_FLOAT:
        this->log_debug((char*)"float: %f", atom_getfloat(atom));
        return py_var(this->vm, atom_getfloat(atom));

    case A_SYM:
        this->log_debug((char*)"symbol: %s", atom_getsym(atom)->s_name);
        return py_var(this->vm, atom_getsym(atom)->s_name);

    case A_NOTHING:
        return this->vm->None;

    default:
        // FIXME: should be this->log_warning
        this->log_error(
            (char*)"Warning: type %d unsupported for conversion to Python.",
            atom->a_type);
        return this->vm->None;
    }
}



/**
 * @brief      Converts a primitive python object to a max atom
 *             used by `plist_to_atoms`
 *
 * @param      value  Python value
 * @param[out] atom   Max atom
 */
t_max_err PktpyInterpreter::pobject_to_atom(PyObject* value, t_atom* atom)
{
    t_max_err err = MAX_ERR_NONE;

    if (is_type(value, this->vm->tp_int)) {
        int int_value = py_cast<int>(this->vm, value);
        atom_setlong(atom, int_value);
    }

    else if (is_type(value, this->vm->tp_float)) {
        double float_value = py_cast<float>(this->vm, value);
        atom_setfloat(atom, float_value);
    }

    else if (is_type(value, this->vm->tp_bool)) {
        bool bool_value = py_cast<bool>(this->vm, value);
        atom_setlong(atom, bool_value);
    }

    else if (is_type(value, this->vm->tp_str)) {
        Str str_value = py_cast<Str>(this->vm, value);
        const char* cstr = str_value.c_str_dup();
        atom_setsym(atom, gensym(cstr));
    }

    else {
        // FIXME: should this not return an 'error' t_symbol
        this->log_error((char*)"Warning: python type unsupported for "
                               "conversion to max t_atom.");
        atom_setsym(atom, gensym("error"));
        err = MAX_ERR_GENERIC;
    }
    return err;
}


// ---------------------------------------------------------------------------
// complex type translation methods


/**
 * @brief Translates atom vector to pocketpy list
 *
 * @param argc atom argument count
 * @param argv atom argument vector
 *
 * @return pocketpy List
 */
List PktpyInterpreter::atoms_to_plist(long argc, t_atom* argv)
{
    return this->atoms_to_plist_with_offset(argc, argv, 0);
}


/**
 * @brief Translates atom vector to pocketpy list
 *
 * @param argc atom argument count
 * @param argv atom argument vector
 * @param start_from index of vector to start from
 *
 * @return pocketpy List
 */
List PktpyInterpreter::atoms_to_plist_with_offset(long argc, t_atom* argv,
                                                  int start_from)
{

    List plist; // pocketpy list

    for (int i = start_from; i < argc; i++) {
        switch ((argv + i)->a_type) {
        case A_FLOAT: {
            double c_float = atom_getfloat(argv + i);
            PyObject* p_float = py_var(this->vm, c_float);
            plist.push_back(p_float);
            break;
        }
        case A_LONG: {
            PyObject* p_int = py_var(this->vm, atom_getlong(argv + i));
            plist.push_back(p_int);
            break;
        }
        case A_SYM: {
            PyObject* p_str = py_var(this->vm, atom_getsym(argv + i)->s_name);
            plist.push_back(p_str);
            break;
        }
        default:
            this->log_debug((char*)"cannot process unknown type");
            break;
        }
    }
    return plist;
}


/**
 * @brief      Populates in-place an empty atom list with the contents of a
 *             python list
 *
 * @param      seq   The python list
 * @param      argc  The count of arguments
 * @param[out] argv  The arguments array
 *
 * @return t_max_err error code
 */
t_max_err PktpyInterpreter::plist_to_atoms(List plist, int* argc,
                                           t_atom** argv)
{
    int len = plist.size();

    *argv = (t_atom*)malloc(len * sizeof(t_atom));
    for (int i = 0; i < len; i++) {
        PyObject* elem = plist[i];
        this->pobject_to_atom(elem, (*argv) + i);
    }
    *argc = (int)len;
    return MAX_ERR_NONE;
}


// ---------------------------------------------------------------------------
// output methods


/**
 * @brief Handler to output pktpy List as max list
 *
 * @param plist pktpy List
 * @return t_max_err error code
 */
t_max_err PktpyInterpreter:: handle_plist_output(List plist, void* outlet)
{
    t_atom atoms_static[PY_MAX_ELEMS];
    t_atom* atoms = NULL;
    int is_dynamic = 0;
    int i = 0;

    int seq_size = plist.size();
    this->log_debug((char*)"seq_size: %d", seq_size);

    if (seq_size == 0) {
        this->log_error((char*)"cannot convert py list of length 0 to atoms");
        return MAX_ERR_GENERIC;
    }

    if (seq_size > PY_MAX_ELEMS) {
        this->log_debug((char*)"dynamically increasing size of atom array");
        atoms = atom_dynamic_start(atoms_static, PY_MAX_ELEMS, seq_size + 1);
        is_dynamic = 1;

    } else {
        atoms = atoms_static;
    }

    this->log_debug((char*)"seq_size2: %d", seq_size);

    for (PyObject* obj : plist) {
        if (is_int(obj)) {
            int int_obj = py_cast<int>(this->vm, obj);
            atom_setlong(atoms + i, int_obj);
            this->log_debug((char*)"%d long: %ld\n", i, int_obj);
            i += 1;
        }

        if (is_float(obj)) {
            float float_obj = py_cast<float>(this->vm, obj);
            atom_setfloat(atoms + i, float_obj);
            this->log_debug((char*)"%d float: %f\n", i, float_obj);
            i += 1;
        }

        if (is_type(obj, this->vm->tp_str)) {
            Str str_obj = py_cast<Str>(this->vm, obj);
            const char* cstr = str_obj.c_str_dup();
            atom_setsym(atoms + i, gensym(cstr));
            this->log_debug((char*)"%d string: %s\n", i, cstr);
            i += 1;
        }
    }

    outlet_list(outlet, NULL, i, atoms);
    this->log_debug((char*)"end iter op: %d", i);

    if (is_dynamic) {
        this->log_debug((char*)"restoring to static atom array");
        atom_dynamic_end(atoms_static, atoms);
    }
    return MAX_ERR_NONE;
}



/**
 * @brief Generic handler to output arbitrarily-typed python object as max
 * object
 *
 * @param pval python object
 * @return t_max_err error code
 */
t_max_err PktpyInterpreter::handle_pyvar_output(PyObject* pval, void* outlet)
{
    if (is_float(pval)) {
        float float_result = py_cast<float>(this->vm, pval);
        outlet_float(outlet, float_result);
        return MAX_ERR_NONE;
    }

    else if (is_int(pval)) {
        float long_result = py_cast<int>(this->vm, pval);
        outlet_int(outlet, long_result);
        return MAX_ERR_NONE;
    }

    else if (is_type(pval, this->vm->tp_str)) {
        Str str_result = py_cast<Str>(this->vm, pval);
        const char* cstr = str_result.c_str_dup();
        outlet_anything(outlet, gensym(cstr), 0, (t_atom*)NIL);
        return MAX_ERR_NONE;
    }

    else if (is_type(pval, this->vm->tp_list)) {
        List plist = py_cast<List>(this->vm, pval);
        return handle_plist_output(plist, outlet);
    }

    // else if (PyDict_Check(pval)) {
    //     return this->handle_dict_output(outlet, pval);
    // }

    else if (pval == this->vm->None) {
        return MAX_ERR_GENERIC;
    }

    else {
        this->log_error((char*)"cannot handle his type of value");
        return MAX_ERR_GENERIC;
    }
}

// ---------------------------------------------------------------------------
// core message method helpers


/**
 * @brief      'eval' a string of python code and convert it into
 *             max atoms and send it the specified outlet.
 *
 * @param      pcode   python code in c string form
 * @param      outlet  the outlet
 *
 * @return     The t maximum error.
 */
t_max_err PktpyInterpreter::eval_pcode(char* pcode, void* outlet)
{
    PyObject* result = this->vm->exec(pcode, "<eval>", EVAL_MODE);

    if (result != NULL) {

        this->log_debug((char*)"eval %s", pcode);

        if (is_type(result, this->vm->tp_int)) {
            int int_result = py_cast<int>(this->vm, result);
            outlet_int(outlet, int_result);
        }

        else if (is_type(result, this->vm->tp_float)) {
            double float_result = py_cast<float>(this->vm, result);
            outlet_float(outlet, float_result);
        }

        else if (is_type(result, this->vm->tp_bool)) {
            bool bool_result = py_cast<bool>(this->vm, result);
            outlet_int(outlet, bool_result);
        }

        else if (is_type(result, this->vm->tp_str)) {
            Str str_result = py_cast<Str>(this->vm, result);
            const char* cstr = str_result.c_str_dup();
            outlet_anything(outlet, gensym(cstr), 0, (t_atom*)NIL);
        }

        else if (is_type(result, this->vm->tp_list)) {
            List list_result = py_cast<List>(this->vm, result);
            this->handle_plist_output(list_result, outlet);
            // outlet_anything(outlet, gensym("list"), 0, (t_atom*)NIL);
        }

        else if (is_type(result, this->vm->tp_tuple)) {
            Tuple tuple_result = py_cast<Tuple>(this->vm, result);
            outlet_anything(outlet, gensym("tuple"), 0, (t_atom*)NIL);
        }

        // else if (is_type(result, this->vm->tp_slice)) {
        //     Slice slice_result = py_cast<Slice>(this->vm, result);
        //     outlet_anything(outlet, gensym("slice"), 0, (t_atom*)NIL);
        // }

        // else if (is_type(result, this->vm->tp_range)) {
        //     Range range_result = py_cast<Range>(this->vm, result);
        //     outlet_anything(outlet, gensym("range"), 0, (t_atom*)NIL);
        // }

        else if (is_type(result, this->vm->tp_exception)) {
            Exception exception_result = py_cast<Exception>(this->vm,
                                                            result);
            outlet_anything(outlet, gensym("star_wrapper"), 0, (t_atom*)NIL);
        }

        else if (is_type(result, this->vm->tp_star_wrapper)) {
            StarWrapper star_wrapper_result = py_cast<StarWrapper>(this->vm,
                                                                   result);
            outlet_anything(outlet, gensym("star_wrapper"), 0, (t_atom*)NIL);
        }

        else if (is_type(result, this->vm->tp_function)) {
            Function func_result = py_cast<Function>(this->vm, result);
            outlet_anything(outlet, gensym("function"), 0, (t_atom*)NIL);
        }

        else if (is_type(result, this->vm->tp_native_func)) {
            NativeFunc func_result = py_cast<NativeFunc>(this->vm, result);
            outlet_anything(outlet, gensym("native_function"), 0,
                            (t_atom*)NIL);
        }

        else if (is_type(result, this->vm->tp_module)) {
            outlet_anything(outlet, gensym("module"), 0, (t_atom*)NIL);
        }

        else if (result == this->vm->None) {
            this->log_debug((char*)"eval None");
        }

        else {
            this->log_debug((char*)"Type Check not Implemented");
        }
        return MAX_ERR_NONE;
    }
    this->log_error((char*)"eval %s", pcode);
    return MAX_ERR_GENERIC;
}


/**
 * @brief Execute a line of python code
 *
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * @return t_max_err error code
 */ 
t_max_err PktpyInterpreter::exec_pcode(char* pcode)
{
    if (this->vm->exec(pcode, "main.py", EXEC_MODE) == NULL) {
        this->log_error((char*)"exec %s", pcode);            
        return MAX_ERR_GENERIC;
    }

    this->log_debug((char*)"exec %s", pcode);
    return MAX_ERR_NONE;
}


/**
 * @brief Execute contents of a file (obtained from symbol) as python code
 *
 * @param path cstring of path to execfile
 * @return t_max_err error code
 */
t_max_err PktpyInterpreter::execfile_path(char* path)
{
    if (path == NULL) {
        return MAX_ERR_GENERIC;
    }

    std::string str_path(path);
    std::ifstream ifs(str_path);
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    this->vm->exec(buffer.str(), "main.py", EXEC_MODE);

    return MAX_ERR_NONE;
}

// ---------------------------------------------------------------------------
// core message method helpers (quiet)

/**
 * @brief A helper function to evaluate Max text as a Python expression.
 * 
 * @return PyObject* python object
 */
PyObject* PktpyInterpreter::eval_text(char* text)
{
    PyObject* result = this->vm->exec(text, "<eval>", EVAL_MODE);
 
    if (result == NULL) {
        if (this->vm->exec(text, "main.py", EXEC_MODE) != NULL) {
            return this->vm->None;
        }
    } else {
        return result;
    }
    return NULL;
}


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
t_max_err PktpyInterpreter::eval_text_to_outlet(long argc, t_atom* argv,
                                                int offset, void* outlet)
{
    long textsize = 0;
    char* text = NULL;

    t_max_err err = atom_gettext(argc + offset, argv, &textsize, &text,
                                 OBEX_UTIL_ATOM_GETTEXT_DEFAULT);
    if (err == MAX_ERR_NONE && textsize && text) {
        this->log_debug((char*)">>> %s", text);
        PyObject* pval = this->eval_text(text);
        if (pval != NULL) {
            this->handle_pyvar_output(pval, outlet);
            return MAX_ERR_NONE;
        }
    }
    return MAX_ERR_GENERIC;
}


// ---------------------------------------------------------------------------
// core message methods

/**
 * @brief Evaluate a max symbol as a python expression
 *
 * @param s symbol of object to be evaluated
 * @param argc atom argument count
 * @param argv atom argument vector
 *
 * @return t_max_err error code
 */
t_max_err PktpyInterpreter::eval(t_symbol* s, long argc, t_atom* argv,
                                 void* outlet)
{
    char* pcode = atom_getsym(argv)->s_name;

    if (pcode == NULL) {
        return MAX_ERR_GENERIC;
    }

    return this->eval_pcode(pcode, outlet);
}


/**
 * @brief Execute a max symbol as a line of python code
 *
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 *
 * @return t_max_err error code
 */
t_max_err PktpyInterpreter::exec(t_symbol* s, long argc, t_atom* argv)
{
    char* pcode = atom_getsym(argv)->s_name;
    if (pcode == NULL) {
        return MAX_ERR_GENERIC;
    }

    return this->exec_pcode(pcode);
}


/**
 * @brief Execute contents of a file (obtained from symbol) as python code
 *
 * @param s symbol
 * @return t_max_err error code
 */
t_max_err PktpyInterpreter::execfile(t_symbol* s)
{

    if (s != gensym("")) {
        // set this->source_path
        t_max_err err = this->locate_path_from_symbol(s);
        if (err != MAX_ERR_NONE) {
            this->log_error((char*)"could not locate path from symbol");
            return err;
        }
    }

    if (s == gensym("") || this->source_path == gensym("")) {
        this->log_error((char*)"could not set filepath");
        return MAX_ERR_GENERIC;
    }

    // assume this->source_path has be been set without errors

    return this->execfile_path(this->source_path->s_name);

}


/**
 * @brief      Try to eval a string of python code, if it succeeds convert
 *             it to atoms and send it out the outlet, otherwise, try to exec it
 *
 * @param      s       symbol
 * @param[in]  argc    The count of arguments
 * @param      argv    The arguments array
 * @param      outlet  The outlet
 *
 * @return     The t maximum error.
 */
t_max_err PktpyInterpreter::anything(t_symbol* s, long argc, t_atom* argv,
                                     void* outlet)
{
    t_atom atoms[PY_MAX_ELEMS];
    long textsize = 0;
    char* text = NULL;
    int is_eval = 1;


    if (s == gensym("")) {
        return MAX_ERR_GENERIC;
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
            this->log_error((char*)"cannot process unknown type");
            break;
        }
    }

    return this->eval_text_to_outlet(argc, atoms, 1, outlet);

}


#endif /* PKTPY_INTERPRETER_H */
