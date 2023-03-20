#include "ext.h"
#include "ext_obex.h"

#include "pocketpy.h"

#define PY_MAX_ELEMS 1024

using namespace pkpy;

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
// t_max_err pktpy_import(t_pktpy* x, t_symbol* s);
t_max_err pktpy_eval(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
t_max_err pktpy_exec(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
// t_max_err pktpy_execfile(t_pktpy* x, t_symbol* s);

// extra py methods
// t_max_err pktpy_call(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
// t_max_err pktpy_assign(t_pktpy* x, t_symbol* s, long argc, t_atom* argv); 
// t_max_err pktpy_code(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
t_max_err pktpy_anything(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);
// t_max_err pktpy_pipe(t_pktpy* x, t_symbol* s, long argc, t_atom* argv);

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
    // class_addmethod(c, (method)pktpy_import,     "import",   A_SYM,      0);
    class_addmethod(c, (method)pktpy_eval,       "eval",     A_GIMME,    0);
    class_addmethod(c, (method)pktpy_exec,       "exec",     A_GIMME,    0);
    // class_addmethod(c, (method)pktpy_execfile,   "execfile", A_DEFSYM,   0);

    // class_addmethod(c, (method)pktpy_assign,     "assign",   A_GIMME,    0);
    // class_addmethod(c, (method)pktpy_call,       "call",     A_GIMME,    0);
    // class_addmethod(c, (method)pktpy_code,       "code",     A_GIMME,    0);
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

void pktpy_assist(t_pktpy* x, void* b, long m, long a, char* s)
{
    if (m == ASSIST_INLET) { // inlet
        snprintf(s, PY_MAX_ELEMS, "I am inlet %ld", a);
    } else { // outlet
        snprintf(s, PY_MAX_ELEMS, "I am outlet %ld", a);
    }
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
        // x->py = pkpy_new_vm(false); // instanciate the PocketPy VM

        attr_args_process(x, argc, argv);
    }
    return (x);
}

void pktpy_bang(t_pktpy* x)
{
    outlet_bang(x->outlet);
}


// t_max_err pktpy_import(t_pktpy* x, t_symbol* s)
// {
//     char* pcode = atom_getsym(s->s_name;
//     return x->py->import(s);
// }


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
            outlet_anything(x->outlet, gensym("list"), 0, (t_atom*)NIL);
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
            outlet_anything(x->outlet, gensym("unknown_type"), 0, (t_atom*)NIL);
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

    // return py_eval_text(x, argc, atoms, 1);
    
    t_max_err err = atom_gettext(argc+1, atoms, &textsize, &text,
                                 // OBEX_UTIL_ATOM_GETTEXT_SYM_FORCE_QUOTE);
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

