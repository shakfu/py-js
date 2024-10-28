/**
    @file cobra - an ITM-based python evaluator

    Experiment to defer the evaluation of a python function
    via the ITM-based sequencing.

*/

#include "ext.h"
#include "ext_obex.h"
#include "ext_common.h"
#include "ext_time.h"
#include "ext_itm.h"

#define PY_IMPLEMENTATION // <-- activate the implementation
#include "py.h"           // <-- include this


// datastructure
typedef struct cobra
{
    t_object c_obj;

    t_py* py;           /*!< this is the key opaque type and instance */
    PyObject* c_func;   /*!< python nullary function to be called in a task */

    // clocking
    void *c_clock;
    t_object *c_timeobj;
    t_object *c_quantize;

    // inlets    
    void *c_proxy;
    long c_inletnum;

    // outlets
    void *c_outlet;
    void *c_outlet2;

} t_cobra;

// prototypes
void *cobra_new(t_symbol *s, long argc, t_atom *argv);
void cobra_free(t_cobra *x);
void cobra_assist(t_cobra *x, void *b, long m, long a, char *s);
void cobra_inletinfo(t_cobra *x, void *b, long a, char *t);
void cobra_int(t_cobra *x, long n);
void cobra_float(t_cobra *x, double f);
void cobra_list(t_cobra *x, t_symbol *s, long argc, t_atom *argv);
void cobra_anything(t_cobra *x, t_symbol *msg, long argc, t_atom *argv);
void cobra_tick(t_cobra *x);
void cobra_bang(t_cobra *x);
void cobra_stop(t_cobra *x);
void cobra_clocktick(t_cobra *x);

t_max_err cobra_import(t_cobra* x, t_symbol* s);
t_max_err cobra_defer(t_cobra* x, t_symbol* s, long argc, t_atom* argv);


static t_class *s_cobra_class = NULL;

void ext_main(void *r)
{
    t_class *c = class_new( "cobra", (method)cobra_new, (method)cobra_free, sizeof(t_cobra), (method)0L, A_GIMME, 0);

    class_addmethod(c, (method)cobra_bang,      "bang",         0);
    class_addmethod(c, (method)cobra_stop,      "stop",         0);
    class_addmethod(c, (method)cobra_int,       "int",          A_LONG, 0);
    class_addmethod(c, (method)cobra_float,     "float",        A_FLOAT, 0);
    class_addmethod(c, (method)cobra_list,      "list",         A_GIMME, 0);
    class_addmethod(c, (method)cobra_anything,  "anything",     A_GIMME, 0);

    class_addmethod(c, (method)cobra_assist,    "assist",       A_CANT, 0);
    class_addmethod(c, (method)cobra_inletinfo, "inletinfo",    A_CANT, 0);

    class_addmethod(c, (method)cobra_import,    "import",       A_SYM,  0);
    class_addmethod(c, (method)cobra_defer,     "defer",       A_GIMME, 0);

    class_time_addattr(c, "delaytime", "Delay Time", TIME_FLAGS_TICKSONLY | TIME_FLAGS_USECLOCK | TIME_FLAGS_TRANSPORT);
    class_time_addattr(c, "quantize", "Quantization", TIME_FLAGS_TICKSONLY);

    class_register(CLASS_BOX, c);

    s_cobra_class = c;
}


/**
 * @brief Initializes the cobra object
 *
 * @param s symbol value
 * @param argc number of arguments
 * @param argv array of atom values
 *
 * @return pointer to cobra object
 *
 * @note: initial optional arg is delay time
 */
void *cobra_new(t_symbol *s, long argc, t_atom *argv)
{
    t_cobra *x = (t_cobra *)object_alloc(s_cobra_class);
    long attrstart = attr_args_offset(argc, argv);
    t_atom a;

    x->py = (t_py*)py_init(); // Initialize and allocate python interpreter instance

    x->c_inletnum = 0;
    x->c_proxy = proxy_new(x, 1, &x->c_inletnum);
    x->c_outlet2 = bangout(x);
    x->c_outlet = bangout(x);

    x->c_timeobj = (t_object *) time_new((t_object *)x, gensym("delaytime"), (method)cobra_tick, TIME_FLAGS_TICKSONLY | TIME_FLAGS_USECLOCK);
    x->c_quantize = (t_object *) time_new((t_object *)x, gensym("quantize"), NULL, TIME_FLAGS_TICKSONLY);
    x->c_clock = clock_new((t_object *)x, (method)cobra_clocktick);

    // x->c_name = symbol_unique();
    x->c_func = NULL;
 
    if (attrstart && argv)
        time_setvalue(x->c_timeobj, NULL, 1, argv);
    else {
        atom_setfloat(&a, 0.);
        time_setvalue(x->c_timeobj, NULL, 1, &a);
    }
    atom_setfloat(&a,0);
    time_setvalue(x->c_quantize, NULL, 1, &a);

    attr_args_process(x, argc, argv);

    return x;
}

/**
 * @brief Frees the cobra object
 *
 * @param x pointer to cobra object
 *
 * @note: `py_free` is called to cleanup the python instance
 */
void cobra_free(t_cobra *x)
{
    freeobject(x->c_timeobj);
    freeobject(x->c_quantize);
    freeobject((t_object *) x->c_proxy);
    freeobject((t_object *)x->c_clock);
    py_free(x->py); // cleanup python.
}

/**
 * @brief Handles assist messages
 */
void cobra_assist(t_cobra *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {    // Inlets
        switch (a) {
        case 0: snprintf_zero(s, ASSIST_MAX_STRING_LEN, "bang Gets Delayed, stop Cancels"); break;
        case 1: snprintf_zero(s, ASSIST_MAX_STRING_LEN, "Set Delay Time"); break;
        }
    }
    else {                      // Outlets
        switch (a) {
        case 0: snprintf_zero(s, ASSIST_MAX_STRING_LEN, "Delayed bang"); break;
        case 1: snprintf_zero(s, ASSIST_MAX_STRING_LEN, "Another Delayed bang"); break;
        }
    }
}

/**
 * @brief Handles inlet info
 */
void cobra_inletinfo(t_cobra *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}

/**
 * @brief Handles int messages
 *
 * @param x pointer to cobra object
 * @param n int value
 */
void cobra_int(t_cobra *x, long n)
{
    cobra_float(x, n);
}

/**
 * @brief Handles float messages
 *
 * @param x pointer to cobra object
 * @param f float value
 */
void cobra_float(t_cobra *x, double f)
{
    t_atom a;

    atom_setfloat(&a, f);
    time_setvalue(x->c_timeobj, NULL, 1, &a);

    if (proxy_getinlet((t_object *)x) == 0)
        cobra_bang(x);
}

/**
 * @brief Handles list messages
 *
 * @param x pointer to cobra object
 * @param s symbol value
 * @param argc number of arguments
 * @param argv array of atom values
 */
void cobra_list(t_cobra *x, t_symbol *s, long argc, t_atom *argv)
{
    cobra_anything(x, NULL, argc, argv);
}

/**
 * @brief Sets the delay time
 *
 * @param x pointer to cobra object
 * @param msg symbol value
 * @param argc number of arguments
 * @param argv array of atom values
 */
void cobra_anything(t_cobra *x, t_symbol *msg, long argc, t_atom *argv)
{
    time_setvalue(x->c_timeobj, msg, argc, argv);

    if (proxy_getinlet((t_object *)x) == 0)
        cobra_bang(x);
}

/**
 * @brief Calls the python function
 *
 * @param x pointer to cobra object
 */
void cobra_tick(t_cobra *x)
{
    if (x->c_func != NULL) {
        PyObject* pval = PyObject_CallObject(x->c_func, NULL);
        if (!PyErr_ExceptionMatches(PyExc_TypeError)) {
            if (pval == NULL) {
                error("unable to apply callable(*args)");
                return;
            }
            py_handle_output(x->py, x->c_outlet, pval);
            // success cleanup
            Py_XDECREF(x->c_func);
            outlet_bang(x->c_outlet);
            return;
        }
        PyErr_Clear();
        return;
    }
    outlet_bang(x->c_outlet);
}

/**
 * @brief Schedules the delay time
 *
 * @param x pointer to cobra object
 */
void cobra_bang(t_cobra *x)
{
    double ms, tix;

    time_schedule(x->c_timeobj, x->c_quantize);

    tix = time_getticks(x->c_timeobj);
    ms = itm_tickstoms(time_getitm(x->c_timeobj), tix);
    clock_fdelay(x->c_clock, ms);
}

/**
 * @brief Handles the clock tick
 *
 * @param x pointer to cobra object
 */
void cobra_clocktick(t_cobra *x)
{
    outlet_bang(x->c_outlet2);
}

/**
 * @brief Stops the delay time
 *
 * @param x pointer to cobra object
 */
void cobra_stop(t_cobra *x)
{
    post("stop");
    time_stop(x->c_timeobj);
    clock_unset(x->c_clock);
    x->c_func = NULL; // reset the function
}

/**
 * @brief Imports a python module
 *
 * @param x pointer to cobra object
 * @param s symbol value
 *
 * @return t_max_err
 */
t_max_err cobra_import(t_cobra* x, t_symbol* s)
{
    return py_import(x->py, s); // returns t_max_err
}


/**
 * @brief Defers the python function
 *
 * @param x pointer to cobra object
 * @param s symbol value
 * @param argc number of arguments
 * @param argv array of atom values
 *
 * @return t_max_err
 */
t_max_err cobra_defer(t_cobra* x, t_symbol* s, long argc, t_atom* argv)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    char* py_argv = atom_getsym(argv)->s_name;
    post("%s %s", s->s_name, py_argv);

    x->c_func = PyRun_String(py_argv, Py_eval_input, x->py->p_globals, x->py->p_globals);

    if (x->c_func != NULL) {
        PyGILState_Release(gstate);
        return MAX_ERR_NONE;
    } else {
        py_handle_error(x->py, "defer py func %s", py_argv);
        PyGILState_Release(gstate);
        return MAX_ERR_GENERIC;
    }
}
