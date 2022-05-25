/**
    @file mamba - an experimental attempt to modularize the python object

    The idea is that it can be included as a header and then used
*/

#include "ext.h"
#include "ext_common.h"
#include "ext_obex.h"
#include "ext_time.h"
#include "ext_itm.h"
#include "Python.h"

// constants
#define PY_MAX_ATOMS 128
#define PY_MAX_LOG_CHAR 500 // high number during development
#define PY_MAX_ERR_CHAR PY_MAX_LOG_CHAR

// globals
typedef struct _py
{
    t_symbol* c_name;           /*!< unique python object name */
    t_symbol* c_pythonpath;     /*!< path to python directory */
    t_bool c_debug;             /*!< bool to switch per-object debug state */
    PyObject* c_globals;        /*!< per object 'globals' python namespace */
} t_py;


// datastructure
typedef struct mamba
{
    t_object c_obj;
    t_py* py;
    void *c_outlet;

} t_mamba;

// prototypes
void *mamba_new(t_symbol *s, long argc, t_atom *argv);
void mamba_free(t_mamba *x);
void mamba_bang(t_mamba *x);

void mamba_init(t_mamba *x);
void mamba_handle_error(t_mamba* x, char* fmt, ...);

t_max_err mamba_handle_float_output(t_mamba* x, PyObject* pfloat);
t_max_err mamba_handle_long_output(t_mamba* x, PyObject* plong);
t_max_err mamba_handle_string_output(t_mamba* x, PyObject* pstring);
t_max_err mamba_handle_list_output(t_mamba* x, PyObject* plist);
t_max_err mamba_handle_dict_output(t_mamba* x, PyObject* pdict);
t_max_err mamba_handle_output(t_mamba* x, PyObject* pval);
t_max_err mamba_import(t_mamba* x, t_symbol* s);
t_max_err mamba_eval(t_mamba* x, t_symbol* s, long argc, t_atom* argv);


static t_class *s_mamba_class = NULL;

void ext_main(void *r)
{
    t_class *c = class_new( "mamba", (method)mamba_new, (method)mamba_free, sizeof(t_mamba), (method)0L, A_GIMME, 0);

    class_addmethod(c, (method)mamba_bang,      "bang",         0);

    class_addmethod(c, (method)mamba_import,    "import",       A_SYM,  0);
    class_addmethod(c, (method)mamba_eval,      "eval",        A_GIMME, 0);

    class_register(CLASS_BOX, c);

    s_mamba_class = c;
}

// initial optional arg is delay time

void *mamba_new(t_symbol *s, long argc, t_atom *argv)
{
    t_mamba *x = (t_mamba *)object_alloc(s_mamba_class);

    x->c_outlet = bangout(x);

    x->py = malloc(sizeof (struct _py));
    x->py->c_name = symbol_unique();
    x->py->c_pythonpath = gensym("");
 
    attr_args_process(x, argc, argv);

    // mamba python init
    mamba_init(x);

    return x;
}

void mamba_init(t_mamba* x)
{
    Py_Initialize();

    // python init
    PyObject* main_mod = PyImport_AddModule(x->py->c_name->s_name); // borrowed
    x->py->c_globals = PyModule_GetDict(main_mod); // borrowed reference

    PyObject* p_name = NULL;
    PyObject* builtins = NULL;

    p_name = PyUnicode_FromString(x->py->c_name->s_name);
    builtins = PyEval_GetBuiltins();
    PyDict_SetItemString(builtins, "PY_OBJ_NAME", p_name);
    PyDict_SetItemString(x->py->c_globals, "__builtins__", builtins);
    Py_XDECREF(p_name);
}


void mamba_free(t_mamba *x)
{
    Py_XDECREF(x->py->c_globals);
    Py_FinalizeEx();
    free(x->py);
}

void mamba_bang(t_mamba *x)
{
    outlet_bang(x->c_outlet);
}

void mamba_handle_error(t_mamba* x, char* fmt, ...)
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

        error("[py %s] %s: %s", x->py->c_name->s_name, msg, pvalue_str);
    }
}


t_max_err mamba_handle_float_output(t_mamba* x, PyObject* pfloat)
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
    mamba_handle_error(x, "mamba_handle_float_output failed");
    Py_XDECREF(pfloat);
    return MAX_ERR_GENERIC;
}


t_max_err mamba_handle_long_output(t_mamba* x, PyObject* plong)
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
    mamba_handle_error(x, "mamba_handle_long_output failed");
    Py_XDECREF(plong);
    return MAX_ERR_GENERIC;
}


t_max_err mamba_handle_string_output(t_mamba* x, PyObject* pstring)
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
    mamba_handle_error(x, "mamba_handle_string_output failed");
    Py_XDECREF(pstring);
    return MAX_ERR_GENERIC;
}


t_max_err mamba_handle_list_output(t_mamba* x, PyObject* plist)
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
        post("seq_size: %d", seq_size);

        if (seq_size == 0) {
            error("cannot convert py list of length 0 to atoms");
            goto error;
        }

        if (seq_size > PY_MAX_ATOMS) {
            post("dynamically increasing size of atom array");
            atoms = atom_dynamic_start(atoms_static, PY_MAX_ATOMS,
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
    mamba_handle_error(x, "mamba_handle_list_output failed");
    Py_XDECREF(plist);
    return MAX_ERR_GENERIC;
}


t_max_err mamba_handle_dict_output(t_mamba* x, PyObject* pdict)
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
                               Py_single_input, x->py->c_globals, x->py->c_globals);

        if (pfun_co == NULL) {
            error("out_dict function code object is NULL");
            goto error;
        }

        pfun = PyDict_GetItemString(x->py->c_globals, "__py_maxmsp_out_dict");
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
            mamba_handle_list_output(x, pval); // this decrefs pval
            Py_XDECREF(pfun_co);
   
            return MAX_ERR_NONE;
        } else {
            error("expected list output got something else");
            goto error;
        }
    }

error:
    mamba_handle_error(x, "mamba_handle_dict_output failed");
    Py_XDECREF(pfun_co);
    Py_XDECREF(pval);
    // fail bang
    return MAX_ERR_GENERIC;
}


t_max_err mamba_handle_output(t_mamba* x, PyObject* pval)
{
    if (pval == NULL) {
        error("cannot handle NULL value");
        return MAX_ERR_GENERIC;
    }

    if (PyFloat_Check(pval)) {
        post("float handled");
        return mamba_handle_float_output(x, pval);
    }

    else if (PyLong_Check(pval)) {
        post("long handled");
        return mamba_handle_long_output(x, pval);
    }

    else if (PyUnicode_Check(pval)) {
        post("string handled");
        return mamba_handle_string_output(x, pval);
    }

    else if (PySequence_Check(pval) && !PyBytes_Check(pval)
             && !PyByteArray_Check(pval)) {
        post("list handled");
        return mamba_handle_list_output(x, pval);
    }

    else if (PyDict_Check(pval)) {
        post("dict handled");
        return mamba_handle_dict_output(x, pval);
    }

    else if (pval == Py_None) {
        return MAX_ERR_GENERIC;
    }

    else {
        error("cannot handle this type of value");
        return MAX_ERR_GENERIC;
    }
}



t_max_err mamba_import(t_mamba* x, t_symbol* s)
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
        PyDict_SetItemString(x->py->c_globals, s->s_name, x_module);
        PyGILState_Release(gstate);
        post("imported: %s", s->s_name);
    }
    return MAX_ERR_NONE;

error:
    mamba_handle_error(x, "import %s", s->s_name);
    PyGILState_Release(gstate);
    return MAX_ERR_GENERIC;
}

t_max_err mamba_eval(t_mamba* x, t_symbol* s, long argc, t_atom* argv)
{
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    char* py_argv = atom_getsym(argv)->s_name;
    post("%s %s", s->s_name, py_argv);

    PyObject* pval = PyRun_String(py_argv, Py_eval_input, x->py->c_globals, x->py->c_globals);

    if (pval != NULL) {
        mamba_handle_output(x, pval);
        PyGILState_Release(gstate);
        return MAX_ERR_NONE;
    } else {
        mamba_handle_error(x, "eval error %s", py_argv);
        PyGILState_Release(gstate);
        return MAX_ERR_GENERIC;
    }
}
