/**
    @file cobra - an ITM-based python evaluator

    Experiment to defer the evaluation of a python function
    via the ITM-based sequencing.
*/

#include "ext.h"
#include "ext_common.h"
#include "ext_obex.h"
#include "ext_time.h"
#include "ext_itm.h"
#include "Python.h"

// constants
#define PY_MAX_ELEMS 1024
#define PY_MAX_LOG_CHAR PY_MAX_ELEMS
#define PY_MAX_ERR_CHAR PY_MAX_LOG_CHAR

// globals
static int py_global_obj_count = 0; // when 0 then free interpreter

static t_hashtab* py_global_registry = NULL; // global object lookups


// datastructure
typedef struct cobra
{
    t_object c_obj;

    t_symbol* c_name;           /*!< unique python object name */
    t_symbol* c_pythonpath;     /*!< path to python directory */
    t_bool c_debug;             /*!< bool to switch per-object debug state */
    PyObject* c_globals;        /*!< per object 'globals' python namespace */
    PyObject* c_func;           /*!< python nullary function to be called in a task */

    void *c_clock;
    t_object *c_timeobj;
    t_object *c_quantize;
    
    void *c_proxy;
    long c_inletnum;

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

void cobra_init(t_cobra *x);
void cobra_handle_error(t_cobra* x, char* fmt, ...);
t_max_err cobra_handle_float_output(t_cobra* x, PyObject* pfloat);
t_max_err cobra_handle_long_output(t_cobra* x, PyObject* plong);
t_max_err cobra_handle_string_output(t_cobra* x, PyObject* pstring);
t_max_err cobra_handle_list_output(t_cobra* x, PyObject* plist);
t_max_err cobra_handle_dict_output(t_cobra* x, PyObject* pdict);
t_max_err cobra_handle_output(t_cobra* x, PyObject* pval);
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
    class_addmethod(c, (method)cobra_defer,      "defer",       A_GIMME, 0);

    class_time_addattr(c, "delaytime", "Delay Time", TIME_FLAGS_TICKSONLY | TIME_FLAGS_USECLOCK | TIME_FLAGS_TRANSPORT);
    class_time_addattr(c, "quantize", "Quantization", TIME_FLAGS_TICKSONLY);

    class_register(CLASS_BOX, c);

    s_cobra_class = c;
}

// initial optional arg is delay time

void *cobra_new(t_symbol *s, long argc, t_atom *argv)
{
    t_cobra *x = (t_cobra *)object_alloc(s_cobra_class);
    long attrstart = attr_args_offset(argc, argv);
    t_atom a;

    x->c_inletnum = 0;
    x->c_proxy = proxy_new(x, 1, &x->c_inletnum);
    x->c_outlet2 = bangout(x);
    x->c_outlet = bangout(x);

    x->c_timeobj = (t_object *) time_new((t_object *)x, gensym("delaytime"), (method)cobra_tick, TIME_FLAGS_TICKSONLY | TIME_FLAGS_USECLOCK);
    x->c_quantize = (t_object *) time_new((t_object *)x, gensym("quantize"), NULL, TIME_FLAGS_TICKSONLY);
    x->c_clock = clock_new((t_object *)x, (method)cobra_clocktick);

    x->c_name = symbol_unique();
    x->c_pythonpath = gensym("");
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

    // cobra python init
    cobra_init(x);

    return x;
}

void cobra_init(t_cobra* x)
{
    Py_Initialize();

    // python init
    PyObject* main_mod = PyImport_AddModule(x->c_name->s_name); // borrowed
    x->c_globals = PyModule_GetDict(main_mod); // borrowed reference

    PyObject* p_name = NULL;
    PyObject* builtins = NULL;

    p_name = PyUnicode_FromString(x->c_name->s_name);
    builtins = PyEval_GetBuiltins();
    PyDict_SetItemString(builtins, "PY_OBJ_NAME", p_name);
    PyDict_SetItemString(x->c_globals, "__builtins__", builtins);
    Py_XDECREF(p_name);

    // register the object
    object_register(CLASS_BOX, x->c_name, x);

    // increment global object counter
    py_global_obj_count++;

    if (py_global_obj_count == 1) {
        // if first py object create the py_global_registry;
        py_global_registry = (t_hashtab*)hashtab_new(0);
        hashtab_flags(py_global_registry, OBJ_FLAG_REF);
    }
}


void cobra_free(t_cobra *x)
{
    freeobject(x->c_timeobj);
    freeobject(x->c_quantize);
    freeobject((t_object *) x->c_proxy);
    freeobject((t_object *)x->c_clock);

    Py_XDECREF(x->c_globals);
    // python objects cleanup
    py_global_obj_count--;
    if (py_global_obj_count == 0) {
        hashtab_chuck(py_global_registry);
        post("last py obj freed -> finalizing py mem / interpreter.");
        Py_FinalizeEx();
    }
}

void cobra_assist(t_cobra *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {    // Inlets
        switch (a) {
        case 0: snprintf_zero(s, PY_MAX_ELEMS, "bang Gets Delayed, stop Cancels"); break;
        case 1: snprintf_zero(s, PY_MAX_ELEMS, "Set Delay Time"); break;
        }
    }
    else {                      // Outlets
        switch (a) {
        case 0: snprintf_zero(s, PY_MAX_ELEMS, "Delayed bang"); break;
        case 1: snprintf_zero(s, PY_MAX_ELEMS, "Another Delayed bang"); break;
        }
    }
}

void cobra_inletinfo(t_cobra *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}

void cobra_int(t_cobra *x, long n)
{
    cobra_float(x, n);
}

void cobra_float(t_cobra *x, double f)
{
    t_atom a;

    atom_setfloat(&a, f);
    time_setvalue(x->c_timeobj, NULL, 1, &a);

    if (proxy_getinlet((t_object *)x) == 0)
        cobra_bang(x);
}

void cobra_list(t_cobra *x, t_symbol *s, long argc, t_atom *argv)
{
    cobra_anything(x, NULL, argc, argv);
}

void cobra_anything(t_cobra *x, t_symbol *msg, long argc, t_atom *argv)
{
    time_setvalue(x->c_timeobj, msg, argc, argv);

    if (proxy_getinlet((t_object *)x) == 0)
        cobra_bang(x);
}

// void cobra_tick(t_cobra *x)
// {
//     outlet_bang(x->c_outlet);
// }
void cobra_tick(t_cobra *x)
{
    if (x->c_func != NULL) {
        PyObject* pval = PyObject_CallObject(x->c_func, NULL);
        if (!PyErr_ExceptionMatches(PyExc_TypeError)) {
            if (pval == NULL) {
                error("unable to apply callable(*args)");
                return;
            }
            cobra_handle_output(x, pval);
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

void cobra_bang(t_cobra *x)
{
    double ms, tix;

    time_schedule(x->c_timeobj, x->c_quantize);

    tix = time_getticks(x->c_timeobj);
    ms = itm_tickstoms(time_getitm(x->c_timeobj), tix);
    clock_fdelay(x->c_clock, ms);
}

void cobra_clocktick(t_cobra *x)
{
    outlet_bang(x->c_outlet2);
}

void cobra_stop(t_cobra *x)
{
    time_stop(x->c_timeobj);
    clock_unset(x->c_clock);
}

void cobra_handle_error(t_cobra* x, char* fmt, ...)
{
    if (PyErr_Occurred()) {

        // build custom msg
        char msg[PY_MAX_ERR_CHAR];

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

        error("[py %s] %s: %s", x->c_name->s_name, msg, pvalue_str);
    }
}


t_max_err cobra_handle_float_output(t_cobra* x, PyObject* pfloat)
{
    if (pfloat == NULL) {
        goto error;
    }

    if (PyFloat_Check(pfloat)) {
        float float_result = (float)PyFloat_AsDouble(pfloat);
        if (float_result == -1.0) {
            if (PyErr_Occurred())
                goto error;
        }

        outlet_float(x->c_outlet, float_result);
    }
    Py_XDECREF(pfloat);
    return MAX_ERR_NONE;

error:
    cobra_handle_error(x, "cobra_handle_float_output failed");
    Py_XDECREF(pfloat);
    return MAX_ERR_GENERIC;
}


t_max_err cobra_handle_long_output(t_cobra* x, PyObject* plong)
{
    if (plong == NULL) {
        goto error;
    }

    if (PyLong_Check(plong)) {
        long long_result = PyLong_AsLong(plong);
        if (long_result == -1) {
            if (PyErr_Occurred())
                goto error;
        }
        outlet_int(x->c_outlet, long_result);
    }

    Py_XDECREF(plong);
    return MAX_ERR_NONE;

error:
    cobra_handle_error(x, "cobra_handle_long_output failed");
    Py_XDECREF(plong);
    return MAX_ERR_GENERIC;
}


t_max_err cobra_handle_string_output(t_cobra* x, PyObject* pstring)
{
    if (pstring == NULL) {
        goto error;
    }

    if (PyUnicode_Check(pstring)) {
        const char* unicode_result = PyUnicode_AsUTF8(pstring);
        if (unicode_result == NULL) {
            goto error;
        }
        outlet_anything(x->c_outlet, gensym(unicode_result), 0, NIL);
    }

    Py_XDECREF(pstring);
    return MAX_ERR_NONE;

error:
    cobra_handle_error(x, "cobra_handle_string_output failed");
    Py_XDECREF(pstring);
    return MAX_ERR_GENERIC;
}


t_max_err cobra_handle_list_output(t_cobra* x, PyObject* plist)
{
    if (plist == NULL) {
        goto error;
    }

    if (PySequence_Check(plist) && !PyUnicode_Check(plist)
        && !PyBytes_Check(plist) && !PyByteArray_Check(plist)) {
        PyObject* iter = NULL;
        PyObject* item = NULL;
        int i = 0;

        t_atom atoms_static[PY_MAX_ELEMS];
        t_atom* atoms = NULL;
        int is_dynamic = 0;

        Py_ssize_t seq_size = PySequence_Length(plist);
        post("seq_size: %d", seq_size);

        if (seq_size == 0) {
            error("cannot convert py list of length 0 to atoms");
            goto error;
        }

        if (seq_size > PY_MAX_ELEMS) {
            post("dynamically increasing size of atom array");
            atoms = atom_dynamic_start(atoms_static, PY_MAX_ELEMS,
                                       seq_size + 1);
            is_dynamic = 1;

        } else {
            atoms = atoms_static;
        }

        if ((iter = PyObject_GetIter(plist)) == NULL) {
            goto error;
        }
        post("seq_size2: %d", seq_size);

        while ((item = PyIter_Next(iter)) != NULL) {
            if (PyLong_Check(item)) {
                long long_item = PyLong_AsLong(item);
                if (long_item == -1) {
                    if (PyErr_Occurred())
                        goto error;
                }
                atom_setlong(atoms + i, long_item);
                post("%d long: %ld\n", i, long_item);
                i++;
            }

            if (PyFloat_Check(item)) {
                float float_item = PyFloat_AsDouble(item);
                if (float_item == -1.0) {
                    if (PyErr_Occurred())
                        goto error;
                }
                atom_setfloat(atoms + i, float_item);
                post("%d float: %f\n", i, float_item);
                i++;
            }

            if (PyUnicode_Check(item)) {
                const char* unicode_item = PyUnicode_AsUTF8(item);
                if (unicode_item == NULL) {
                    goto error;
                }
                atom_setsym(atoms + i, gensym(unicode_item));
                post("%d unicode: %s\n", i, unicode_item);
                i++;
            }
            Py_DECREF(item);
        }

        outlet_list(x->c_outlet, NULL, i, atoms);
        post("end iter op: %d", i);

        if (is_dynamic) {
            post("restoring to static atom array");
            atom_dynamic_end(atoms_static, atoms);
        }
    }

    Py_XDECREF(plist);
    return MAX_ERR_NONE;

error:
    cobra_handle_error(x, "cobra_handle_list_output failed");
    Py_XDECREF(plist);
    return MAX_ERR_GENERIC;
}


t_max_err cobra_handle_dict_output(t_cobra* x, PyObject* pdict)
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
                               Py_single_input, x->c_globals, x->c_globals);

        if (pfun_co == NULL) {
            error("out_dict function code object is NULL");
            goto error;
        }

        pfun = PyDict_GetItemString(x->c_globals, "__py_maxmsp_out_dict");
        if (pfun == NULL) {
            error("retrieving out_dict func from globals failed");
            goto error;
        }

        pval = PyObject_CallFunctionObjArgs(pfun, pdict, NULL);
        if (pval == NULL) {
            error("out_dict call failed to retrieve result");
            goto error;
        }

        if (PyList_Check(pval)) {           // expecting a python list
            cobra_handle_list_output(x, pval); // this decrefs pval
            Py_XDECREF(pfun_co);
   
            return MAX_ERR_NONE;
        } else {
            error("expected list output got something else");
            goto error;
        }
    }

error:
    cobra_handle_error(x, "cobra_handle_dict_output failed");
    Py_XDECREF(pfun_co);
    Py_XDECREF(pval);
    // fail bang
    return MAX_ERR_GENERIC;
}


t_max_err cobra_handle_output(t_cobra* x, PyObject* pval)
{
    if (pval == NULL) {
        error("cannot handle NULL value");
        return MAX_ERR_GENERIC;
    }

    if (PyFloat_Check(pval)) {
        post("float handled");
        return cobra_handle_float_output(x, pval);
    }

    else if (PyLong_Check(pval)) {
        post("long handled");
        return cobra_handle_long_output(x, pval);
    }

    else if (PyUnicode_Check(pval)) {
        post("string handled");
        return cobra_handle_string_output(x, pval);
    }

    else if (PySequence_Check(pval) && !PyBytes_Check(pval)
             && !PyByteArray_Check(pval)) {
        post("list handled");
        return cobra_handle_list_output(x, pval);
    }

    else if (PyDict_Check(pval)) {
        post("dict handled");
        return cobra_handle_dict_output(x, pval);
    }

    else if (pval == Py_None) {
        return MAX_ERR_GENERIC;
    }

    else {
        error("cannot handle this type of value");
        return MAX_ERR_GENERIC;
    }
}



t_max_err cobra_import(t_cobra* x, t_symbol* s)
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
        PyDict_SetItemString(x->c_globals, s->s_name, x_module);
        PyGILState_Release(gstate);
        post("imported: %s", s->s_name);
    }
    return MAX_ERR_NONE;

error:
    cobra_handle_error(x, "import %s", s->s_name);
    PyGILState_Release(gstate);
    return MAX_ERR_GENERIC;
}

t_max_err cobra_defer(t_cobra* x, t_symbol* s, long argc, t_atom* argv)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    char* py_argv = atom_getsym(argv)->s_name;
    post("%s %s", s->s_name, py_argv);

    x->c_func = PyRun_String(py_argv, Py_eval_input, x->c_globals, x->c_globals);

    if (x->c_func != NULL) {
        PyGILState_Release(gstate);
        return MAX_ERR_NONE;
    } else {
        cobra_handle_error(x, "defer py func %s", py_argv);
        PyGILState_Release(gstate);
        return MAX_ERR_GENERIC;
    }
}
