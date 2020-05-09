/**
    @file
    py.c - basic experiment in minimal max object for calling python code

    This object has 1 inlet and 1 outlet

    Basic Features

    1.  Per-Object Namespace. It responds to an 'import <module>' message in 
        the left inlet which loads a python module in its namespace.

    2.  Code Messages. It responds to any other message in the left inlet
        which is evaluated in the namespace and outputs results to the outlet
    
    3.  Rerun stored code. Tt responds to a 'bang' message in the left inlet
        and runs the previously sent code message.


    py object
        @module

        (optional)
        @imports

    @ingroup    examples
*/

#include "ext.h"            // you must include this - it contains the external object's link to available Max functions
#include "ext_obex.h"       // this is required for all objects using the newer style for writing objects.
#include "ext_path.h"       // required for MAX_FILENAME_CHARS

/* python */
#define PY_SSIZE_T_CLEAN
#include <Python.h>


typedef struct _py {    // defines our object's internal variables for each instance in a patch
    t_object p_ob;          // object header - ALL objects MUST begin with this...
    t_symbol *module;       // python module to import
    // char **p_value;         // cstring value to eval in py namespace - received from the left inlet and stored internally for each object instance
    void *p_outlet;         // outlet creation - inlets are automatic, but objects must "own" their own outlets
} t_py;


// these are prototypes for the methods that are defined below
void py_bang(t_py *x);
void py_import(t_py *x, t_symbol *s);
void py_msg(t_py *x, t_symbol *s, long argc, t_atom *argv);
void *py_new(t_symbol *s, long argc, t_atom *argv);
void py_free(t_py *x);

t_class *py_class;      // global pointer to the object class - so max can reference the object


//--------------------------------------------------------------------------

void ext_main(void *r)
{
    t_class *c;

    c = class_new("py", (method)py_new, (method)py_free, (long)sizeof(t_py),
                  0L /* leave NULL!! */, A_GIMME, 0);

    class_addmethod(c, (method)py_bang,   "bang",     0);   // the method it uses when it gets a bang in the left inlet
    class_addmethod(c, (method)py_import, "import", A_DEFSYM, 0);
    class_addmethod(c, (method)py_msg,    "anything", A_GIMME, 0);
    
    // attributes
    CLASS_ATTR_SYM(c, "module",  0, t_py, module);
    CLASS_ATTR_BASIC(c, "module", 0);

    class_register(CLASS_BOX, c);
    py_class = c;

    post("py object loaded...",0);  // post any important info to the max window when our class is loaded
}


//--------------------------------------------------------------------------
void *py_new(t_symbol *s, long argc, t_atom *argv)
{
    t_py *x;                // local variable (pointer to a t_py data structure)

    x = (t_py *)object_alloc(py_class); // create a new instance of this object

    x->p_outlet = outlet_new(x, NULL); // can send any message out

    x->module = NULL;
    // x->p_value = NULL;
    // x->p_value = sysmem_newhandle(0);

    x->module = gensym("");
    // if(argc){
    //     atom_arg_getsym(&x->module, 0, argc, argv);
    //     if(x->module != _sym_nothing){
    //         if(x->module->s_name[0] == '@'){
    //             x->module = _sym_nothing;
    //         }
    //     } 
    //     //post("s4m_new() module: %s", x->source_file->s_name);
    // }

    // process @arg attributes
    attr_args_process(x, argc, argv);

    post(" new py object instance added to patch...", 0); // post important info to the max window when new instance is created

    return(x);                  // return a reference to the object instance
}


void py_free(t_py *x)
{
    ;
}


//--------------------------------------------------------------------------


void py_import(t_py *x, t_symbol *s) {
    x->module = s;
}


void py_msg(t_py *x, t_symbol *s, long argc, t_atom *argv) {

    if( gensym(s->s_name) == gensym("eval") ){
        char *code_input = atom_getsym(argv)->s_name; 
        post("eval: %s", code_input);
        return; 
    }

}


void py_bang(t_py *x)           // x = reference to this instance of the object
{
    return;
    // char *cstr;
    // PyObject *locals, *globals;
    // PyObject *pval;
    // // PyObject *pval, *xval, *yval;

    // // python init and setup
    // Py_Initialize();
    // locals = PyDict_New();
    // globals = PyDict_New();
    // PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());
    
    // pval = PyRun_String(cstr, Py_eval_input, globals, locals);

    // if PyLong_Check(pval) {
    //     sum = PyLong_AsLong(pval);
    // }
    // outlet_int(x->p_outlet, sum);       // send out the sum on bang

    // Py_DECREF(pval);
    // Py_FinalizeEx();
}







