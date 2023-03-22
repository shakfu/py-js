#include "ext.h"
#include "ext_obex.h"

#include "pocketpy.h"



using namespace pkpy;

// #ifndef PKTPY_INTERPRETER_H
// #define PKTPY_INTERPRETER_H

/**
 * @brief      specifies three logging levels
 */
enum log_level {
    ERROR, INFO, DEBUG 
};

#define PY_MAX_ELEMS 1024
#define PY_LOG_LEVEL DEBUG

/**
 * @brief      This class describes a pocketpy interpreter.
 */
class PktpyInterpreter
{
    private:
        t_symbol* p_name;           //!< unique python object name
        t_symbol* p_pythonpath;     //!< path to python directory
        t_symbol* p_source_name;    //!< base name of python file to execfile
        t_symbol* p_source_path;    //!< full path to python file to execfile
        log_level p_log_level;      //!< object-level log level (error, info, debug)
        VM* p_vm;                   //!< pocketpy vm instance

    public:
        PktpyInterpreter();
        ~PktpyInterpreter();

        // helpers
        void log_debug(char* fmt, ...);
        void log_info(char* fmt, ...);
        void log_error(char* fmt, ...);
        void print_atom(int argc, t_atom* argv);

        // python helpers
        // void handle_error(char* fmt, ...);
        t_max_err syspath_append(char* path);

        // python <-> atom translation
        List atoms_to_plist_with_offset(long argc, t_atom* argv, int start_from);
        List atoms_to_plist(long argc, t_atom* argv); //+
        t_max_err plist_to_atoms(List seq, int* argc, t_atom** argv); //+
        // Tuple atoms_to_ptuple(int argc, t_atom* argv); //+

        PyVar atom_to_pobject(t_atom* atom); // used by atoms_to_ptuple
        t_max_err pobject_to_atom(PyVar value, t_atom* atom); // used by plist_to_atoms

        // python value -> atom -> output
        t_max_err handle_float_output(void* outlet, PyVar pval);
        t_max_err handle_long_output(void* outlet, PyVar pval);
        t_max_err handle_string_output(void* outlet, PyVar pval);
        t_max_err handle_list_output(void* outlet, PyVar pval);
        // t_max_err handle_dict_output(void* outlet, PyVar pval);
        t_max_err handle_output(void* outlet, PyVar pval);

        // core message method helpers
        PyVar eval_pcode(char* pcode);
        t_max_err exec_pcode(char* pcode);
        // t_max_err execfile_path(char* path);
        t_max_err locate_path_from_symbol(t_symbol* s);

        // core message methods
        t_max_err eval(t_symbol* s, long argc, t_atom* argv, void* outlet);
        t_max_err exec(t_symbol* s, long argc, t_atom* argv);
        // t_max_err execfile(t_symbol* s);

        // extra message method helpers
        PyVar eval_text(char* text);
        t_max_err eval_text_to_outlet(long argc, t_atom* argv, int offset, void* outlet);

        // extra message methods
        // t_max_err call(t_symbol* s, long argc, t_atom* argv, void* outlet);
        // t_max_err assign(t_symbol* s, long argc, t_atom* argv);
        t_max_err anything(t_symbol* s, long argc, t_atom* argv, void* outlet);
        // t_max_err pipe(t_symbol* s, long argc, t_atom* argv, void* outlet);
};

// ---------------------------------------------------------------------------
// constructor / destructor methods

/**
 * @brief      Constructs a new PktpyInterpreter instance.
 */
PktpyInterpreter::PktpyInterpreter()
{
    this->p_name = symbol_unique();
    this->p_pythonpath = gensym("");
    this->p_source_name = gensym("");
    this->p_source_path = gensym("");
    this->p_log_level = log_level::PY_LOG_LEVEL;
    this->p_vm = new VM(true);
}


/**
 * @brief      PktpyInterpreter destructor method.
 */
PktpyInterpreter::~PktpyInterpreter()
{
    pkpy_delete(this->p_vm);
}



// ---------------------------------------------------------------------------
// helper methods


/**
 * @brief Post msg to Max console.
 *
 * @param fmt character string with format codes
 * @param ... other arguments
 */
void PktpyInterpreter::log_debug(char* fmt, ...)
{
    if (this->p_log_level >= log_level::DEBUG) {
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
void PktpyInterpreter::log_info(char* fmt, ...)
{
    if (this->p_log_level >= log_level::INFO) {
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
void PktpyInterpreter::log_error(char* fmt, ...)
{
    if (this->p_log_level >= log_level::ERROR) {
        char msg[PY_MAX_ELEMS];

        va_list va;
        va_start(va, fmt);
        vsnprintf(msg, PY_MAX_ELEMS, fmt, va);
        va_end(va);

        error((char*)"[py error %s]: %s", this->p_name->s_name, msg);
    }
}


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


// ---------------------------------------------------------------------------
// translation methods

/**
 * @brief      Converts max atom to python object
 *
 * @param      atom  The atom
 *
 * @return     python object (int, float, string)
 */
PyVar PktpyInterpreter::atom_to_pobject(t_atom* atom)
{
    this->log_debug((char*)"py_atom_to_py_object start");

    switch (atom->a_type) {

    case A_LONG:
        this->log_debug((char*)"int: %i", atom_getlong(atom));
        return py_var<int>(this->p_vm, atom_getlong(atom));

    case A_FLOAT:
        this->log_debug((char*)"float: %f", atom_getfloat(atom));
        return py_var<float>(this->p_vm, atom_getfloat(atom));

    case A_SYM:
        this->log_debug((char*)"symbol: %s", atom_getsym(atom)->s_name);
        return py_var<Str>(this->p_vm, atom_getsym(atom)->s_name);

    case A_NOTHING:
        return this->p_vm->None;

    default:
        // FIXME: should be this->log_warning
        this->log_error((char*)"Warning: type %d unsupported for conversion to Python.",
             atom->a_type);
        return this->p_vm->None;
    }
}


/**
 * @brief      Converts a python object to a max atom 
 *
 * @param      value  Python value
 * @param[out] atom   Max atom
 */
t_max_err PktpyInterpreter::pobject_to_atom(PyVar value, t_atom* atom)
{
    t_max_err err = MAX_ERR_NONE;

    // if (value == Py_True)
    //     atom_setlong(atom, 1);
    // else if (value == Py_False)
    //     atom_setlong(atom, 0);
    // else if (PyFloat_Check(value))
    //     atom_setfloat(atom, (float)PyFloat_AsDouble(value));
    // else if (PyLong_Check(value))
    //     atom_setlong(atom, (float)PyLong_AsLong(value));
    // else if (PyUnicode_Check(value))
    //     atom_setsym(atom, gensym(PyUnicode_AsUTF8(value)));

    if (is_type(value, this->p_vm->tp_int)) {
        int int_value = py_cast<int>(this->p_vm, value);
        atom_setlong(atom, int_value);
    }
    
    else if (is_type(value, this->p_vm->tp_float)) {
        double float_value = py_cast<float>(this->p_vm, value);
        atom_setfloat(atom, float_value);
    }

    else if (is_type(value, this->p_vm->tp_bool)) {
        bool bool_value = py_cast<bool>(this->p_vm, value);
        atom_setlong(atom, bool_value);
    }

    else if (is_type(value, this->p_vm->tp_str)) {
        Str str_value = py_cast<Str>(this->p_vm, value);
        const char* cstr = strdup(str_value.c_str());
        atom_setsym(atom, gensym(cstr));
    }

    else {
        // FIXME: should this not return an 'error' t_symbol
        this->log_error((char*)"Warning: python type unsupported for conversion to max t_atom.");
        atom_setsym(atom, gensym("error"));
        err = MAX_ERR_GENERIC;
    }
    return err;
}


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
            PyVar p_float = py_var<float>(this->p_vm, c_float);
            plist.push_back(p_float);
            break;
        }
        case A_LONG: {
            PyVar p_int = py_var<int>(this->p_vm, atom_getlong(argv + i));
            plist.push_back(p_int);
            break;
        }
        case A_SYM: {
            PyVar p_str = py_var<Str>(this->p_vm,
                atom_getsym(argv + i)->s_name);
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
 * python list
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
        PyVar elem = plist[i];
        this->pobject_to_atom(elem, (*argv) + i);
    }
    *argc = (int)len;
    return MAX_ERR_NONE;

}


// ---------------------------------------------------------------------------
// output methods

/**
 * @brief Handler to output python float as max float
 *
 * @param pfloat python float
 * @return t_max_err error code
 */
t_max_err PktpyInterpreter::handle_float_output(void* outlet, PyVar pval)
{
    if (is_float(pval)) {
        float float_result = py_cast<float>(this->p_vm, pval);
        outlet_float(outlet, float_result);
    }
    return MAX_ERR_NONE;
}



/**
 * @brief Handler to output python long as max int
 *
 * @param plong python long
 * @return t_max_err error code
 */
t_max_err PktpyInterpreter::handle_long_output(void* outlet, PyVar pval)
{
    if (is_int(pval)) {
        float long_result = py_cast<int>(this->p_vm, pval);
        outlet_int(outlet, long_result);
    }
    return MAX_ERR_NONE;
}

/**
 * @brief Handler to output python string as max symbol
 *
 * @param pstring python string
 * @return t_max_err error code
 */
t_max_err PktpyInterpreter::handle_string_output(void* outlet, PyVar pval)
{
    if (is_type(pval, this->p_vm->tp_str)) {
        Str str_result = py_cast<Str>(this->p_vm, pval);
        const char* cstr = strdup(str_result.c_str());
        outlet_anything(outlet, gensym(cstr), 0, (t_atom*)NIL);
    }
    return MAX_ERR_NONE;
}

/**
 * @brief Handler to output python list as max list
 *
 * @param plist python list
 * @return t_max_err error code
 */
t_max_err PktpyInterpreter::handle_list_output(void* outlet, PyVar pval)
{
    if (is_type(pval, this->p_vm->tp_list)) {

        List plist = py_cast<List>(this->p_vm, pval);

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

        for (PyVar obj : plist) {
            if (is_int(obj)) {
                int int_obj = py_cast<int>(this->p_vm, obj);
                atom_setlong(atoms + i, int_obj);
                this->log_debug((char*)"%d long: %ld\n", i, int_obj);
                i += 1;
            }

            if (is_float(obj)) {
                float float_obj = py_cast<float>(this->p_vm, obj);
                atom_setfloat(atoms + i, float_obj);
                this->log_debug((char*)"%d float: %f\n", i, float_obj);
                i += 1;
            }

            if (is_type(obj, this->p_vm->tp_str)) {
                Str str_obj = py_cast<Str>(this->p_vm, obj);
                const char* cstr = strdup(str_obj.c_str());
                atom_setsym(atoms + i, gensym(cstr));
                this->log_debug((char*)"%d unicode: %s\n", i, cstr);
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
    return MAX_ERR_GENERIC;
}


/**
 * @brief Generic handler to output arbitrarily-typed python object as max
 * object
 *
 * @param pval python object
 * @return t_max_err error code
 */
t_max_err PktpyInterpreter::handle_output(void* outlet, PyVar pval)
{
    if (is_float(pval)) {
        return this->handle_float_output(outlet, pval);
    }

    else if (is_int(pval)) {
        return this->handle_long_output(outlet, pval);
    }

    else if (is_type(pval, this->p_vm->tp_str)) {
        return this->handle_string_output(outlet, pval);
    }

    else if (is_type(pval, this->p_vm->tp_list)) {
        return this->handle_list_output(outlet, pval);
    }

    // else if (PyDict_Check(pval)) {
    //     return this->handle_dict_output(outlet, pval);
    // }

    else if (pval == this->p_vm->None) {
        return MAX_ERR_GENERIC;
    }

    else {
        this->log_error((char*)"cannot handle his type of value");
        return MAX_ERR_GENERIC;
    }
}

//#endif /* PKTPY_INTERPRETER_H */






typedef struct _pktpy {
    t_object ob;
    t_symbol* name;
    void* outlet;
    VM* py; // PocketPy VM object is named `py` for familiarity
} t_pktpy;


BEGIN_USING_C_LINKAGE

void* pktpy_new(t_symbol* s, long argc, t_atom* argv);
void pktpy_free(t_pktpy* x);
void pktpy_assist(t_pktpy* x, void* b, long m, long a, char* s);

// attr getters / setters
// t_max_err pktpy_name_get(t_pktpy* x, t_object* attr, long* argc, t_atom** argv);
// t_max_err pktpy_name_set(t_pktpy* x, t_object* attr, long argc, t_atom* argv);


// basic methods
void pktpy_bang(t_pktpy*);

// core methods
t_max_err pktpy_eval(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
t_max_err pktpy_exec(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
// t_max_err pktpy_execfile(t_pktpy* x, t_symbol* s);

// extra py methods
// t_max_err pktpy_call(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
// t_max_err pktpy_assign(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
t_max_err pktpy_anything(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
// t_max_err pktpy_pipe(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);

// utilities
t_symbol* pktpy_get_path_to_external(t_pktpy* x);
// int add100(int a);


END_USING_C_LINKAGE


static t_class* pktpy_class = NULL;

void ext_main(void* r)
{
    t_class* c;

    c = class_new("pktpy", 
            (method)pktpy_new,
            (method)pktpy_free,
            (long)sizeof(t_pktpy), 
            (method)NULL, /* leave NULL!! */
            A_GIMME,
            0L);

    class_addmethod(c, (method)pktpy_assist,     "assist",   A_CANT,     0);

    class_addmethod(c, (method)pktpy_bang,       "bang",                 0);

    class_addmethod(c, (method)pktpy_eval,       "eval",     A_GIMME,    0);
    class_addmethod(c, (method)pktpy_exec,       "exec",     A_GIMME,    0);
    // class_addmethod(c, (method)pktpy_execfile,   "execfile", A_DEFSYM,   0);

    // class_addmethod(c, (method)pktpy_assign,     "assign",   A_GIMME,    0);
    // class_addmethod(c, (method)pktpy_call,       "call",     A_GIMME,    0);
    // class_addmethod(c, (method)pktpy_pipe,       "pipe",     A_GIMME,    0);
    class_addmethod(c, (method)pktpy_anything,   "anything", A_GIMME,    0);

    CLASS_ATTR_LABEL(c, "name", 0,  "unique object id");
    CLASS_ATTR_SYM(c, "name", 0,   t_pktpy, name);
    CLASS_ATTR_BASIC(c, "name", 0);
    // CLASS_ATTR_ACCESSORS(c, "name", NULL, pktpy_name_set);


    class_register(CLASS_BOX, c); /* CLASS_NOBOX */
    pktpy_class = c;

    post("I am the pktpy object");
}


t_symbol* pktpy_get_path_to_external(t_pktpy* x)
{
    char external_path[MAX_PATH_CHARS];
    char external_name[MAX_PATH_CHARS];
    char conform_path[MAX_PATH_CHARS];

    short path_id = class_getpath(pktpy_class);
    snprintf_zero(external_name, PY_MAX_ELEMS, "%s.mxo", pktpy_class->c_sym->s_name);
    path_toabsolutesystempath(path_id, external_name, external_path);
    path_nameconform(external_path, conform_path, PATH_STYLE_NATIVE, PATH_TYPE_TILDE);
    // post("path_id: %d, external_name: %s, external_path: %s conform_path: %s", 
    //     path_id, external_name, external_path, conform_path);
    return gensym(external_path);
}




void pktpy_assist(t_pktpy* x, void* b, long m, long a, char* s)
{
    if (m == ASSIST_INLET) { // inlet
        snprintf(s, PY_MAX_ELEMS, "I am inlet %ld", a);
    } else { // outlet
        snprintf(s, PY_MAX_ELEMS, "I am outlet %ld", a);
    }
}



// example function to be wrapped by NativeProxyFunc
int add100(int a){
    return a + 100;
}

void add_custom_builtins(t_pktpy* x)
{
        // builtins
        x->py->bind_builtin_func<1>("add10", [](VM* vm, Args& args) {
            i64 a = CAST(i64, args[0]);
            return VAR(a + 10);
        });

        // example of wrapping a max api function
        // bind: void *outlet_int(t_outlet *x, t_atom_long n);
        // >>> out_int(10) -> sends 10 out of outlet
        x->py->bind_builtin_func<1>("out_int", [x](VM* vm, Args& args) {
            i64 a = CAST(i64, args[0]);
            outlet_int(x->outlet, a);
            return vm->None;
        });

        // wrap exist function pktpy_get_path_to_external
        x->py->bind_builtin_func<0>("location", [x](VM* vm, Args& args) {
            t_symbol* sym = pktpy_get_path_to_external(x);
            outlet_anything(x->outlet, sym, 0, (t_atom*)NIL);
            return vm->None;
        });

        // example of wrapping function using NativeProxyFunc
        // It can be constructed from a function pointer,
        // a lambda function, or a std::function.
        x->py->bind_builtin_func<1>("add100", NativeProxyFunc(&add100));
}



void pktpy_free(t_pktpy* x)
{
    pkpy_delete(x->py);
}

void* pktpy_new(t_symbol* s, long argc, t_atom* argv)
{
    t_pktpy* x = NULL;
    long i;

    if ((x = (t_pktpy*)object_alloc(pktpy_class))) {
        object_post((t_object*)x, "a new %s object was instantiated: %p",
                    s->s_name, x);
        object_post((t_object*)x, "it has %ld arguments", argc);

        for (i = 0; i < argc; i++) {
            if ((argv + i)->a_type == A_LONG) {
                object_post((t_object*)x, "arg %ld: long (%ld)", i,
                            atom_getlong(argv + i));
            } else if ((argv + i)->a_type == A_FLOAT) {
                object_post((t_object*)x, "arg %ld: float (%f)", i,
                            atom_getfloat(argv + i));
            } else if ((argv + i)->a_type == A_SYM) {
                object_post((t_object*)x, "arg %ld: symbol (%s)", i,
                            atom_getsym(argv + i)->s_name);
            } else {
                object_error((t_object*)x, "forbidden argument");
            }
        }

        x->name = gensym("");
        x->outlet = bangout((t_object*)x);
        x->py = new VM(true);    // instanciate the PocketPy VM

        // custom builtins
        add_custom_builtins(x);

        attr_args_process(x, argc, argv);
    }
    return (x);
}



void pktpy_bang(t_pktpy* x)
{
    outlet_bang(x->outlet);
}


/**
 * @brief Execute a max symbol as a line of python code
 *
 * @param x instance of t_pktpy struct
 * @param s symbol
 * @param argc atom argument count
 * @param argv atom argument vector
 * 
 * @return t_max_err error code
 */
t_max_err pktpy_exec(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
{
    char* pcode = atom_getsym(argv)->s_name;
    if (pcode == NULL) {
        return MAX_ERR_GENERIC;
    }

    if(x->py->exec(pcode, "main.py", EXEC_MODE) == NULL) {
        return MAX_ERR_GENERIC;
    }
    return MAX_ERR_NONE;
}


/**
 * @brief Handler to output pktpy List as max list
 *
 * @param plist pktpy List
 * @return t_max_err error code
 */
t_max_err handle_list_output(t_pktpy* x, List plist)
{
    t_atom atoms_static[PY_MAX_ELEMS];
    t_atom* atoms = NULL;
    int is_dynamic = 0;
    int i = 0;

    int seq_size = plist.size();
    // this->log_debug((char*)"seq_size: %d", seq_size);

    if (seq_size == 0) {
        // this->log_error((char*)"cannot convert py list of length 0 to atoms");
        return MAX_ERR_GENERIC;
    }

    if (seq_size > PY_MAX_ELEMS) {
        // this->log_debug((char*)"dynamically increasing size of atom array");
        atoms = atom_dynamic_start(atoms_static, PY_MAX_ELEMS,
                                   seq_size + 1);
        is_dynamic = 1;

    } else {
        atoms = atoms_static;
    }

    // this->log_debug((char*)"seq_size2: %d", seq_size);

    for (PyVar obj : plist) {
        if (is_int(obj)) {
            int int_obj = py_cast<int>(x->py, obj);
            atom_setlong(atoms + i, int_obj);
            // this->log_debug((char*)"%d long: %ld\n", i, long_item);
            i += 1;
        }

        if (is_float(obj)) {
            float float_obj = py_cast<float>(x->py, obj);
            atom_setfloat(atoms + i, float_obj);
            // this->log_debug((char*)"%d float: %f\n", i, float_item);
            i += 1;
        }

        if (is_type(obj, x->py->tp_str)) {
            Str str_obj = py_cast<Str>(x->py, obj);
            const char* cstr = strdup(str_obj.c_str());
            atom_setsym(atoms + i, gensym(cstr));
            // this->log_debug((char*)"%d unicode: %s\n", i, unicode_item);
            i += 1;
        }
    }

    outlet_list(x->outlet, NULL, i, atoms);
    // this->log_debug((char*)"end iter op: %d", i);

    if (is_dynamic) {
        // this->log_debug((char*)"restoring to static atom array");
        atom_dynamic_end(atoms_static, atoms);
    }
    return MAX_ERR_NONE;
}



t_max_err pktpy_eval_text(t_pktpy* x, char* text)
{
    PyVar result = x->py->exec(text, "<eval>", EVAL_MODE);

    if (result != NULL) {

        if (is_type(result, x->py->tp_int)) {
            int int_result = py_cast<int>(x->py, result);
            outlet_int(x->outlet, int_result);
        }
        
        else if (is_type(result, x->py->tp_float)) {
            double float_result = py_cast<float>(x->py, result);
            outlet_float(x->outlet, float_result);
        }

        else if (is_type(result, x->py->tp_bool)) {
            bool bool_result = py_cast<bool>(x->py, result);
            outlet_int(x->outlet, bool_result);
        }

        else if (is_type(result, x->py->tp_str)) {
            Str str_result = py_cast<Str>(x->py, result);
            const char* cstr = strdup(str_result.c_str());
            outlet_anything(x->outlet, gensym(cstr), 0, (t_atom*)NIL);
        }

        else if (is_type(result, x->py->tp_list)) {
            List list_result = py_cast<List>(x->py, result);
            handle_list_output(x, list_result);
            // outlet_anything(x->outlet, gensym("list"), 0, (t_atom*)NIL);
        }

        else if (is_type(result, x->py->tp_tuple)) {
            Tuple tuple_result = py_cast<Tuple>(x->py, result);
            outlet_anything(x->outlet, gensym("tuple"), 0, (t_atom*)NIL);
        }

        else if (is_type(result, x->py->tp_slice)) {
            Slice slice_result = py_cast<Slice>(x->py, result);
            outlet_anything(x->outlet, gensym("slice"), 0, (t_atom*)NIL);
        }

        else if (is_type(result, x->py->tp_range)) {
            Range range_result = py_cast<Range>(x->py, result);
            outlet_anything(x->outlet, gensym("range"), 0, (t_atom*)NIL);
        }

        else if (is_type(result, x->py->tp_exception)) {
            Exception exception_result = py_cast<Exception>(x->py, result);
            outlet_anything(x->outlet, gensym("star_wrapper"), 0, (t_atom*)NIL);
        }

        else if (is_type(result, x->py->tp_star_wrapper)) {
            StarWrapper star_wrapper_result = py_cast<StarWrapper>(x->py, result);
            outlet_anything(x->outlet, gensym("star_wrapper"), 0, (t_atom*)NIL);
        }

        else if (is_type(result, x->py->tp_function)) {
            Function func_result = py_cast<Function>(x->py, result);
            outlet_anything(x->outlet, gensym("function"), 0, (t_atom*)NIL);
        }

        else if (is_type(result, x->py->tp_native_function)) {
            NativeFunc func_result = py_cast<NativeFunc>(x->py, result);
            outlet_anything(x->outlet, gensym("native_function"), 0, (t_atom*)NIL);
        }
    
        else if (is_type(result, x->py->tp_module)) {
            outlet_anything(x->outlet, gensym("module"), 0, (t_atom*)NIL);
        }
    
        else {
             // pass
        }
        return MAX_ERR_NONE;
    }
    return MAX_ERR_GENERIC;
}

/**
 * @brief Evaluate a max symbol as a python expression
 *
 * @param x instance of t_pktpy struct
 * @param s symbol of object to be evaluated
 * @param argc atom argument count
 * @param argv atom argument vector
 *
 * @return t_max_err error code
 */
t_max_err pktpy_eval(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
{
    char* pcode = atom_getsym(argv)->s_name;

    if (pcode == NULL) {
        return MAX_ERR_GENERIC;
    }

    return pktpy_eval_text(x, pcode);
}



t_max_err pktpy_anything(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
{

    t_atom atoms[PY_MAX_ELEMS];
    long textsize = 0;
    char* text = NULL;
    // int is_eval = 1;


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
            post("cannot process unknown type");
            break;
        }
    }
    
    t_max_err err = atom_gettext(argc+1, atoms, &textsize, &text,
                                 OBEX_UTIL_ATOM_GETTEXT_DEFAULT);
    post("text: %s", text);
    if (err == MAX_ERR_NONE && textsize && text) {
        post(">>> %s", text);
    } else {
        error("pktpy_anything: null entry");
        return MAX_ERR_GENERIC;
    }

    if (pktpy_eval_text(x, text) == MAX_ERR_NONE) {
        post("pktpy_anything: eval succeeded");
        return MAX_ERR_NONE;
    }
    
    if (x->py->exec(text, "main.py", EXEC_MODE) != NULL) {
        post("pktpy_anything: exec succeeded");
        return MAX_ERR_NONE;
    }
    error("pktpy_anything: exec failed");
    return MAX_ERR_GENERIC;
}




// t_max_err pktpy_execfile(t_pktpy* x, t_symbol* s)
// {
//     return x->py->execfile(s);
// }

// t_max_err pktpy_assign(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
// {
//     return x->py->assign(s, argc, argv);
// }

// t_max_err pktpy_call(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
// {
//     return x->py->call(s, argc, argv, x->outlet);
// }


// t_max_err pktpy_code(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
// {
//     return x->py->code(s, argc, argv, x->outlet);
// }


// t_max_err pktpy_anything(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
// {
//     return x->py->anything(s, argc, argv, x->outlet);
// }


// t_max_err pktpy_pipe(t_pktpy* x, t_symbol* s, long argc, t_atom* argv)
// {
//     return x->py->pipe(s, argc, argv, x->outlet);
// }

