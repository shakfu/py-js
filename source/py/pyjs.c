#include "ext.h"
#include "ext_obex.h"

/* python */
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#define PY_MAX_ATOMS 128
#define PY_MAX_LOG_CHAR 500 // high number during development
#define PY_MAX_ERR_CHAR PY_MAX_LOG_CHAR



typedef struct _pyjs {
    /* object header */
    t_object p_ob;

    /* object attributes */
    double myattr;

    /* python-related */
    PyObject* p_globals;    /* per object 'globals' python namespace */
    t_symbol* p_name; /* unique object name */
    t_symbol* p_pythonpath; /* path to python directory */
    t_symbol* p_code_filepath; /* python filepath */
	t_bool p_debug;         /* bool to switch per-object debug state */

} t_pyjs;


void *pyjs_new(t_symbol *s, long argc, t_atom *argv);
void pyjs_free(t_pyjs *x);
void pyjs_print(t_pyjs *x, t_symbol *s, long ac, t_atom *av);
t_max_err pyjs_doabs(t_pyjs *x, t_symbol *s, long ac, t_atom *av, t_atom *rv);
t_max_err pyjs_code(t_pyjs* x, t_symbol* s, long argc, t_atom* argv, t_atom *rv);
void pyjs_init_builtins(t_pyjs* x);
void pyjs_init(t_pyjs* x);
void pyjs_log(t_pyjs* x, char* fmt, ...);
void pyjs_error(t_pyjs* x, char* fmt, ...);
t_max_err pyjs_handle_output(t_pyjs* x, PyObject* pval, t_atom* rv);
t_max_err pyjs_handle_float_output(t_pyjs* x, PyObject* pfloat, t_atom* rv);
t_max_err pyjs_handle_long_output(t_pyjs* x, PyObject* plong, t_atom* rv);
void pyjs_handle_error(t_pyjs* x, char* fmt, ...);

static t_class *pyjs_class;

static int pyjs_global_obj_count;       // when 0 then free interpreter


void ext_main(void *r)
{
	t_class *c;

	c = class_new("pyjs", 
		(method)pyjs_new, 
		(method)pyjs_free, 
		(long)sizeof(t_pyjs),
		0L /* leave NULL!! */, A_GIMME, 0);

	class_addmethod(c, (method)pyjs_print, "print",	0);
	class_addmethod(c, (method)pyjs_doabs, "doAbs",	A_GIMMEBACK, 0);
	class_addmethod(c, (method)pyjs_code,  "code",	A_GIMMEBACK, 0);

	CLASS_ATTR_DOUBLE(c, "myattr", 0, t_pyjs, myattr);

	c->c_flags = CLASS_FLAG_POLYGLOT;
	class_register(CLASS_NOBOX, c);
	pyjs_class = c;
}

void pyjs_free(t_pyjs *x)
{
    Py_XDECREF(x->p_globals);
    pyjs_log(x, "will be deleted");
    pyjs_global_obj_count--;
    if (pyjs_global_obj_count == 0) {
         Py_FinalizeEx();
    }
}


void *pyjs_new(t_symbol *s, long argc, t_atom *argv)
{
	t_pyjs *x = NULL;

	// object instantiation, NEW STYLE
	if ((x = (t_pyjs *)object_alloc(pyjs_class))) {
		// Initialize values
		x->myattr = 74.;

        if (pyjs_global_obj_count == 0) {
            // first py obj is called '__main__'
            x->p_name = gensym("__main__");
        } else {
            x->p_name = symbol_unique();
        }
        // x->p_name = symbol_unique();

        x->p_pythonpath = gensym("");
        x->p_debug = 1;
        x->p_code_filepath = gensym("");

        // process @arg attributes
        attr_args_process(x, argc, argv);

        // python init
        pyjs_init(x);
	}
	return (x);
}


void pyjs_init_builtins(t_pyjs* x)
{
    PyObject* p_name = NULL;
    PyObject* builtins = NULL;
    int err = -1;

    p_name = PyUnicode_FromString(x->p_name->s_name);
    if (p_name == NULL)
        goto error;

    builtins = PyEval_GetBuiltins();
    if (builtins == NULL)
        goto error;

    err = PyDict_SetItemString(builtins, "PY_OBJ_NAME", p_name);
    if (err == -1)
        goto error;

    err = PyDict_SetItemString(x->p_globals, "__builtins__", builtins);
    if (err == -1)
        goto error;

    Py_XDECREF(p_name);
    return;

error:
    pyjs_handle_error(x, "could not update object namespace with object name");
    Py_XDECREF(p_name);
}


void pyjs_init(t_pyjs* x)
{

    Py_Initialize();

    // python init
    PyObject* main_mod = PyImport_AddModule(x->p_name->s_name); // borrowed
    x->p_globals = PyModule_GetDict(main_mod); // borrowed reference
    pyjs_init_builtins(x); // does this have to be a separate function?

    // increment global object counter
    pyjs_global_obj_count++;
}


void pyjs_log(t_pyjs* x, char* fmt, ...)
{
    if (x->p_debug) {
        char msg[PY_MAX_LOG_CHAR];

        va_list va;
        va_start(va, fmt);
        vsprintf(msg, fmt, va);
        va_end(va);

        post("[pyjs %s]: %s", x->p_name->s_name, msg);
    }
}


void pyjs_error(t_pyjs* x, char* fmt, ...)
{
    char msg[PY_MAX_ERR_CHAR];

    va_list va;
    va_start(va, fmt);
    vsprintf(msg, fmt, va);
    va_end(va);

    error("[pyjs %s]: %s", x->p_name->s_name, msg);
}

void pyjs_handle_error(t_pyjs* x, char* fmt, ...)
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

        error("[pyjs %s] %s: %s", x->p_name->s_name, msg, pvalue_str);
    }
}

void pyjs_print(t_pyjs *x, t_symbol *s, long ac, t_atom *av)
{
	post("The value of myattr is: %f", x->myattr);
}


t_max_err pyjs_doabs(t_pyjs *x, t_symbol *s, long ac, t_atom *av, t_atom *rv)
{
	t_atom a[1];
	double f = 0;

	if (ac) {
		if (atom_gettype(av) == A_LONG)
            f = (double)llabs(atom_getlong(av));
		else if( atom_gettype(av) == A_FLOAT)
			f = fabs(atom_getfloat(av));
	} else
		error("missing argument for method doAbs()");

	// store the result in the a array.
	atom_setfloat(a, f);

	// return the result to js
	atom_setobj(rv, object_new(gensym("nobox"), gensym("atomarray"), 1, a));

	return MAX_ERR_NONE;
}


// t_max_err pyjs_code(t_pyjs* x, t_symbol* s, long argc, t_atom* argv, t_atom* rv)
// {
//     t_atom atoms[PY_MAX_ATOMS];
//     long textsize = 0;
//     char* text = NULL;
//     PyObject* co = NULL;
//     PyObject* pval = NULL;
//     t_max_err err;
//     int is_eval = 1;

//     err = atom_gettext(argc, argv, &textsize, &text,
//                        OBEX_UTIL_ATOM_GETTEXT_DEFAULT);
//     if (err == MAX_ERR_NONE && textsize && text) {
//         pyjs_log(x, "call %s", text);
//     } else {
//         goto error;
//     }

//     co = Py_CompileString(text, x->p_name->s_name, Py_eval_input);

//     if (PyErr_ExceptionMatches(PyExc_SyntaxError)) {
//         PyErr_Clear();
//         co = Py_CompileString(text, x->p_name->s_name, Py_single_input);
//         is_eval = 0;
//     }

//     if (co == NULL) { // can be eval-co or exec-co or NULL here
//         goto error;
//     }
//     sysmem_freeptr(text);

//     pval = PyEval_EvalCode(co, x->p_globals, x->p_globals);
//     if (pval == NULL) {
//         goto error;
//     }
//     Py_DECREF(co);

//     if (!is_eval) {
//         return -1;

//     } else {

// 	    if (pval == NULL) {
// 	        pyjs_error(x, "cannot handle NULL value");
// 	        return;
// 	    }

// 	    if (PyFloat_Check(pval)) {

// 	        float float_result = (float)PyFloat_AsDouble(pval);
// 	        if (float_result == -1.0) {
// 	            if (PyErr_Occurred())
// 	                goto error;
// 	        }
// 			atom_setfloat(atoms, float_result);
// 			atom_setobj(rv, object_new(gensym("nobox"), gensym("atomarray"), 1, atoms));        
		    
// 		    Py_XDECREF(pval);
// 	    }
//     }
//     return MAX_ERR_NONE;

// error:
//     pyjs_handle_error(x, "code failed");
//     Py_XDECREF(pval);
//     return -1;
// }


t_max_err pyjs_code(t_pyjs* x, t_symbol* s, long argc, t_atom* argv, t_atom* rv)
{
    long textsize = 0;
    char* text = NULL;
    PyObject* co = NULL;
    PyObject* pval = NULL;
    t_max_err err;
    int is_eval = 1;

    err = atom_gettext(argc, argv, &textsize, &text,
                       OBEX_UTIL_ATOM_GETTEXT_DEFAULT);
    if (err == MAX_ERR_NONE && textsize && text) {
        pyjs_log(x, "code %s", text);
    } else {
        goto error;
    }

    co = Py_CompileString(text, x->p_name->s_name, Py_eval_input);

    if (PyErr_ExceptionMatches(PyExc_SyntaxError)) {
        PyErr_Clear();
        co = Py_CompileString(text, x->p_name->s_name, Py_single_input);
        is_eval = 0;
    }

    if (co == NULL) { // can be eval-co or exec-co or NULL here
        goto error;
    }
    sysmem_freeptr(text);

    pval = PyEval_EvalCode(co, x->p_globals, x->p_globals);
    if (pval == NULL) {
        goto error;
    }
    Py_DECREF(co);

    if (is_eval) {
        pyjs_handle_output(x, pval, rv);
    } else {
        Py_XDECREF(pval);
    }
    return MAX_ERR_NONE;

error:
    pyjs_handle_error(x, "pyjs code failed");
    Py_XDECREF(pval);
    return MAX_ERR_GENERIC;
}



// ---------------------------------------------------------------------------
// Handlers


t_max_err pyjs_handle_float_output(t_pyjs* x, PyObject* pfloat, t_atom* rv)
{
	t_atom atom_result[1];

    if (pfloat == NULL) {
        goto error;
    }

    if (PyFloat_Check(pfloat)) {
        float float_result = (float)PyFloat_AsDouble(pfloat);
        if (float_result == -1.0) {
            if (PyErr_Occurred())
                goto error;
        }
		atom_setfloat(atom_result, float_result);
		atom_setobj(rv, object_new(gensym("nobox"), gensym("atomarray"), 1, atom_result));        
    }
    Py_XDECREF(pfloat);
    return MAX_ERR_NONE;

error:
    pyjs_handle_error(x, "pyjs_handle_float_output failed");
    Py_XDECREF(pfloat);
    return MAX_ERR_GENERIC;
}


t_max_err pyjs_handle_long_output(t_pyjs* x, PyObject* plong, t_atom* rv)
{
	t_atom atom_result[1];

    if (plong == NULL) {
        goto error;
    }

    if (PyLong_Check(plong)) {
        long long_result = PyLong_AsLong(plong);
        if (long_result == -1) {
            if (PyErr_Occurred())
                goto error;
        }
		atom_setlong(atom_result, long_result);
		atom_setobj(rv, object_new(gensym("nobox"), gensym("atomarray"), 1, atom_result)); 
    }
    Py_XDECREF(plong);
    return MAX_ERR_NONE;

error:
    pyjs_handle_error(x, "pyjs_handle_long_output failed");
    Py_XDECREF(plong);
    return MAX_ERR_GENERIC;
}


t_max_err pyjs_handle_string_output(t_pyjs* x, PyObject* pstring, t_atom* rv)
{
	t_atom atom_result[PY_MAX_ATOMS];

    if (pstring == NULL) {
        goto error;
    }

    if (PyUnicode_Check(pstring)) {
        const char* unicode_result = PyUnicode_AsUTF8(pstring);
        if (unicode_result == NULL) {
            goto error;
        }
		atom_setsym(atom_result, gensym(unicode_result));
		atom_setobj(rv, object_new(gensym("nobox"), gensym("atomarray"), 1, atom_result));
    }
    Py_XDECREF(pstring);
    return MAX_ERR_NONE;

error:
    pyjs_handle_error(x, "pyjs_handle_string_output failed");
    Py_XDECREF(pstring);
    return MAX_ERR_GENERIC;
}


t_max_err pyjs_handle_list_output(t_pyjs* x, PyObject* plist, t_atom* rv)
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

        if (seq_size == 0) {
            pyjs_error(x, "cannot convert py list of length 0 to atoms");
            goto error;
        }

        if (seq_size > PY_MAX_ATOMS) {
            pyjs_log(x, "dynamically increasing size of atom array");
            atoms = atom_dynamic_start(atoms_static, PY_MAX_ATOMS,
                                       seq_size + 1);
            is_dynamic = 1;

        } else {
            atoms = atoms_static;
        }

        if ((iter = PyObject_GetIter(plist)) == NULL) {
            goto error;
        }

        while ((item = PyIter_Next(iter)) != NULL) {
            if (PyLong_Check(item)) {
                long long_item = PyLong_AsLong(item);
                if (long_item == -1) {
                    if (PyErr_Occurred())
                        goto error;
                }
                atom_setlong(atoms + i, long_item);
                pyjs_log(x, "%d long: %ld\n", i, long_item);
                i++;
            }

            if PyFloat_Check (item) {
                float float_item = PyFloat_AsDouble(item);
                if (float_item == -1.0) {
                    if (PyErr_Occurred())
                        goto error;
                }
                atom_setfloat(atoms + i, float_item);
                pyjs_log(x, "%d float: %f\n", i, float_item);
                i++;
            }

            if PyUnicode_Check (item) {
                const char* unicode_item = PyUnicode_AsUTF8(item);
                if (unicode_item == NULL) {
                    goto error;
                }
                atom_setsym(atoms + i, gensym(unicode_item));
                pyjs_log(x, "%d unicode: %s\n", i, unicode_item);
                i++;
            }
            Py_DECREF(item);
        }

        atom_setobj(rv, object_new(gensym("nobox"), gensym("atomarray"), (long)i, atoms));
        // atom_setobj(rv, object_new(gensym("nobox"), gensym("atomarray"), 1, atoms));
        pyjs_log(x, "end iter op: %d", i);
        if (is_dynamic) {
            pyjs_log(x, "restoring to static atom array");
            atom_dynamic_end(atoms_static, atoms);
        }
    }

    Py_XDECREF(plist);
    return MAX_ERR_NONE;

error:
    pyjs_handle_error(x, "pyjs_handle_list_output failed");
    Py_XDECREF(plist);
    return MAX_ERR_GENERIC;
}


// void pyjs_handle_dict_output(t_py* x, PyObject* pdict)
// {
//     PyObject* pfun_co = NULL;
//     PyObject* pfun = NULL;
//     PyObject* pval = NULL;

//     if (pdict == NULL) {
//         goto error;
//     }

//     if (PyDict_Check(pdict)) {

//         pfun_co = PyRun_String( \
//             "def __py_maxmsp_out_dict(arg):\n"
//                 "\tres = []\n"
//                 "\tfor k,v in arg.items():\n"
//                     "\t\tres.append(k)\n"
//                     "\t\tres.append(':')\n"
//                     "\t\tif type(v) in [list, set, tuple]:\n"
//                         "\t\t\tfor i in v:\n"
//                             "\t\t\t\tres.append(i)\n"
//                     "\t\telse:\n"
//                         "\t\t\tres.append(v)\n"
//                 "\treturn res\n", 
//                 Py_single_input, x->p_globals, x->p_globals);

//         if (pfun_co == NULL) {
//             pyjs_error(x, "out_dict function code object is NULL");
//             goto error;
//         }

//         pfun = PyDict_GetItemString(x->p_globals, "__py_maxmsp_out_dict");
//         if (pfun == NULL) {
//             pyjs_error(x, "retrieving out_dict func from globals failed");
//             goto error;
//         }

//         pval = PyObject_CallFunctionObjArgs(pfun, pdict, NULL);
//         if (pval == NULL) {
//             pyjs_error(x, "out_dict call failed to retrieve result");
//             goto error;
//         }

//         if (PyList_Check(pval)) { // expecting a python list
//             pyjs_handle_list_output(x, pval); // this decrefs pval
//             Py_XDECREF(pfun_co);
//             outlet_bang(x->p_outlet_right);
//             return;
//         } else {
//             pyjs_error(x, "expected list output got something else");
//             goto error;
//         }
//     }

// error:
//     pyjs_handle_error(x, "pyjs_handle_dict_output failed");
//     Py_XDECREF(pfun_co);
//     Py_XDECREF(pval);
//     // fail bang
//     outlet_bang(x->p_outlet_middle);
// }


t_max_err pyjs_handle_output(t_pyjs* x, PyObject* pval, t_atom* rv)
{
    if (pval == NULL) {
        pyjs_error(x, "cannot handle NULL value");
        return MAX_ERR_GENERIC;
    }

    if (PyFloat_Check(pval)) {
        return pyjs_handle_float_output(x, pval, rv);
    }

    else if (PyLong_Check(pval)) {
        return pyjs_handle_long_output(x, pval, rv);
    }

    else if (PyUnicode_Check(pval)) {
        return pyjs_handle_string_output(x, pval, rv);
    }

    else if (PySequence_Check(pval) && !PyBytes_Check(pval)
             && !PyByteArray_Check(pval)) {
        return pyjs_handle_list_output(x, pval, rv);
    }

    // else if (PyDict_Check(pval)) {
    //     pyjs_handle_dict_output(x, pval);
    //     return;
    // }

    // else if (pval == Py_None) {
    //     return;
    // }

    else {
        pyjs_error(x, "cannot handle his type of value");
        return MAX_ERR_GENERIC;
    }

}

