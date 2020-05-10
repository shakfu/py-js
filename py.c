/**
    py.c - basic experiment in minimal max object for calling python code

    This object has 1 inlet and 1 outlet

    Basic Features

    1.  Per-Object Namespace. It responds to an 'import <module>' message in 
        the left inlet which loads a python module in its namespace.

    2.  Code Messages. It responds to any other message in the left inlet
        which is evaluated in the namespace and outputs results to the outlet
    
    3.  Rerun stored code. Tt responds to a 'bang' message in the left inlet
        and runs the previously sent code message.

    py interpreter object
        @import <module>
        @eval <code>

        (phase 1)
        @run <script>

        (phase 2)
        @load <script>
        @code <stored code>

    TODO

        - [ ] add right inlet bang after eval op ends


*/

#include "ext.h"            // you must include this - it contains the external object's link to available Max functions
#include "ext_obex.h"       // this is required for all objects using the newer style for writing objects.

/* python */
#define PY_SSIZE_T_CLEAN
#include <Python.h>


typedef struct _py {
    t_object p_ob;          // object header - ALL objects MUST begin with this...
    t_symbol *import;       // python import: additional imports
    t_symbol *code;         // python code to evaluate to default outlet
    long p_value0;          // int value - received from the left inlet and stored internally for each object instance
    long p_value1;          // int value - received from the right inlet and stored internally for each object instance
    void *p_outlet;         // outlet creation - inlets are automatic, but objects must "own" their own outlets
} t_py;


// these are prototypes for the methods that are defined below
void py_bang(t_py *x);
void py_import(t_py *x, t_symbol *s);
void py_eval(t_py *x, t_symbol *s, long argc, t_atom *argv);
void py_run(t_py *x, t_symbol *s, long argc, t_atom *argv);
void py_int(t_py *x, long n);
void py_in1(t_py *x, long n);
void py_dblclick(t_py *x);
void py_assist(t_py *x, void *b, long m, long a, char *s);

void *py_new(t_symbol *s, long argc, t_atom *argv);
void py_free(t_py *x);

t_class *py_class;      // global pointer to the object class - so max can reference the object


/*--------------------------------------------------------------------------*/

void ext_main(void *r)
{
    t_class *c;

    c = class_new("py", (method)py_new, (method)py_free, (long)sizeof(t_py),
                  0L /* leave NULL!! */, A_GIMME, 0);

    // methods
    class_addmethod(c, (method)py_bang,    "bang",      0);
    class_addmethod(c, (method)py_import,  "import",    A_DEFSYM, 0);
    class_addmethod(c, (method)py_eval,    "anything",  A_GIMME, 0);
    class_addmethod(c, (method)py_eval,    "run",       A_GIMME, 0);
    class_addmethod(c, (method)py_int,     "int",       A_LONG, 0);
    class_addmethod(c, (method)py_in1,     "in1",       A_LONG, 0);

    /* you CAN'T call this from the patcher */
    class_addmethod(c, (method)py_dblclick, "dblclick", A_CANT, 0);
    class_addmethod(c, (method)py_assist,   "assist",   A_CANT, 0);

    // attributes
    CLASS_ATTR_SYM(c, "import", 0, t_py, import);
    CLASS_ATTR_BASIC(c, "import", 0);

    CLASS_ATTR_SYM(c, "code",    0, t_py, code);
    CLASS_ATTR_BASIC(c, "code", 0);


    class_register(CLASS_BOX, c);
    py_class = c;

    post("py object loaded...",0);  // post any important info to the max window when our class is loaded
}


/*--------------------------------------------------------------------------*/

void *py_new(t_symbol *s, long argc, t_atom *argv)
{
    t_py *x;

    x = (t_py *)object_alloc(py_class);

    // create inlet(s)
    intin(x,1);      // create a second int inlet (leftmost inlet is automatic - all objects have one inlet by default)

    // create outlet
    x->p_outlet = outlet_new(x, NULL);

    x->import = gensym("");
    x->code = gensym("");

    x->p_value0 = 0;
    x->p_value1 = 0;
    
    // process @arg attributes
    attr_args_process(x, argc, argv);
    
    post("new py object instance added to patch...", 0);

    return(x);
}


void py_free(t_py *x)
{
    ;
}


//--------------------------------------------------------------------------

void py_assist(t_py *x, void *b, long m, long a, char *s) // 4 final arguments are always the same for the assistance method
{
    if (m == ASSIST_OUTLET)
        sprintf(s,"Sum of Left and Right Inlets");
    else {
        switch (a) {
        case 0:
            sprintf(s,"Inlet %ld: Left Operand (Causes Output)", a);
            break;
        case 1:
            sprintf(s,"Inlet %ld: Right Operand (Added to Left)", a);
            break;
        }
    }
}


void py_dblclick(t_py *x)
{
    object_post((t_object *)x, "I got a double-click");
}


void py_import(t_py *x, t_symbol *s) {
    x->import = s;
    post("import: %s", x->import->s_name);
}

void py_run(t_py *x, t_symbol *s, long argc, t_atom *argv) {
    ;
}

void py_eval(t_py *x, t_symbol *s, long argc, t_atom *argv) {
    PyObject *locals, *globals;
    PyObject *pval;

    if (gensym(s->s_name) == gensym("eval")) {
        char *code_input = atom_getsym(argv)->s_name; 
        post("eval: %s", code_input);

        // python init and setup
        Py_Initialize();
        locals = PyDict_New();
        //globals = PyDict_New();
        //PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());
        PyObject* main_module = PyImport_AddModule("__main__");
        PyObject* globals = PyModule_GetDict(main_module);

        if (x->import != gensym("")) {
            post("eval-import: %s", x->import->s_name);

            PyObject* x_module = PyImport_ImportModule(x->import->s_name);
            PyDict_SetItemString(globals, x->import->s_name, x_module);
            post("eval-imported: %s", x->import->s_name);
        }
        
        pval = PyRun_String(code_input, Py_eval_input, globals, locals);

        if (pval) {

            if (PyLong_Check(pval)) {
                long int_result = PyLong_AsLong(pval);
                outlet_int(x->p_outlet, int_result);
            }

            if (PyFloat_Check(pval)) {
                float float_result = (float) PyFloat_AsDouble(pval);
                outlet_float(x->p_outlet, float_result);
            }

            if (PySequence_Check(pval)) {
                Py_ssize_t length = PySequence_Length(pval);
                PyObject *item;
                t_atom atom_dummy[3];
                t_atomarray* atom_array = atomarray_new(3, atom_dummy);

                while ((item = PyIter_Next(pval))) {
                    if PyLong_Check(item) {
                        long int_item = PyLong_AsLong(item);
                        atomarray_appendatom(atom_array, int_item);
                    }
                    if PyFloat_Check(item) {
                        float float_item = PyFloat_AsDouble(item);
                        atomarray_appendatom(atom_array float_item);
                    }
                    if PyUnicode_Check(item) {
                        const char *unicode_result = PyUnicode_AsUTF8(item);
                        atomarray_appendatom(atom_array, gensym(unicode_result));
                    }

                }

                long array_size = (long) atomarray_getsize(atom_array);
                t_atom *atoms = NULL;

                atomarray_copyatoms(atom_array, &array_size, &atoms);
                if (array_size && atoms) {
                    sysmem_freeptr(atoms);
                }   


                // char *bytes_result = PyBytes_AsString(pval);
                // outlet_symbol(x->p_outlet, bytes_result);
            }

            if PyBytes_Check(pval) {
                error("python bytes returned");
                // char *bytes_result = PyBytes_AsString(pval);
                // outlet_symbol(x->p_outlet, bytes_result);
            }

            // if PyUnicode_Check(pval) {
            //     //const char *unicode_result = PyUnicode_AsUTF8(pval);
            //     PyObject *unicode_list = PyUnicode_Split(pv, NULL, -1);

            //     if PyIter_Check(unicode_list) {
            //         PyObject *item;
            //         while ((item = PyIter_Next(unicode_list))) {

            //         }
            //     }



            //     t_atom atoms[10];
            //     atom_array = t_atomarray* atomarray_new(10, atoms);


            //     outlet_anything(x->p_outlet, unicode_result, 0, NULL);
            // }

            Py_DECREF(pval);

        } else {
            if (PyErr_Occurred()) {
                PyErr_Print();
                error("error ocurred: %s", code_input);
            }
        }
        
        // --------------
        // new refs
        Py_DECREF(locals);
        Py_DECREF(globals);
        // borrowed refs
        // Py_XDECREF(main_module);
        // Py_XDECREF(x_module);
        // --------------
        Py_FinalizeEx();

    }
    return;  
}


void py_bang(t_py *x)
{
    long sum;
    PyObject *locals, *globals;
    PyObject *pval, *xval, *yval;

    // python init and setup
    Py_Initialize();
    locals = PyDict_New();
    globals = PyDict_New();
    PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());

    xval = PyLong_FromLong(x->p_value0);
    yval = PyLong_FromLong(x->p_value1);

    PyDict_SetItemString(globals, "x", xval);
    PyDict_SetItemString(globals, "y", yval);

    pval = PyRun_String("x**y", Py_eval_input, globals, locals);

    if PyLong_Check(pval) {
        sum = PyLong_AsLong(pval);
    }

    outlet_int(x->p_outlet, sum);

    Py_DECREF(pval);
    Py_DECREF(xval);
    Py_DECREF(yval);
    Py_FinalizeEx();
}

void py_int(t_py *x, long n)
{
    x->p_value0 = n;
    py_bang(x);
}


void py_in1(t_py *x, long n)
{
    x->p_value1 = n;
}

