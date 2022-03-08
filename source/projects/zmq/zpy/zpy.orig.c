
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <zmq.h>

#include "ext.h"
#include "ext_obex.h"


#define PY_MAX_ATOMS 128
#define PY_MAX_LOG_CHAR 500 // high number during development
#define PY_MAX_ERR_CHAR PY_MAX_LOG_CHAR

#define ZPY_ADDRESS "tcp://localhost:5555"

typedef struct _zpy {
    /* object header */
    t_object p_ob;
    /* python-related */
    t_symbol* p_name;          /* unique object name */
    t_symbol* p_pythonpath;    /* path to python directory */
    t_symbol* p_code_filepath; /* python filepath */
    t_bool p_debug;            /* bool to switch per-object debug state */
} t_zpy;


void* zpy_new(t_symbol* s, long argc, t_atom* argv);
void zpy_free(t_zpy* x);
void zpy_log(t_zpy* x, char* fmt, ...);
void zpy_error(t_zpy* x, char* fmt, ...);
void zpy_handle_error(t_zpy* x, char* fmt, ...);

t_max_err zpy_import(t_zpy* x, t_symbol* s);
t_max_err zpy_exec(t_zpy* x, t_symbol* s);
t_max_err zpy_execfile(t_zpy* x, t_symbol* s);
t_max_err zpy_eval(t_zpy* x, t_symbol* s, long argc, t_atom* argv, t_atom* rv);
t_max_err zpy_code(t_zpy* x, t_symbol* s, long argc, t_atom* argv, t_atom* rv);
t_max_err zpy_handle_output(t_zpy* x, PyObject* pval, t_atom* rv);
t_max_err zpy_handle_float_output(t_zpy* x, PyObject* pfloat, t_atom* rv);
t_max_err zpy_handle_long_output(t_zpy* x, PyObject* plong, t_atom* rv);
t_max_err zpy_handle_list_output(t_zpy* x, PyObject* plist, t_atom* rv);
t_max_err zpy_handle_dict_output(t_zpy* x, PyObject* pdict, t_atom* rv);


/* globals */
static t_class* zpy_class;


void ext_main(void* module_ref)
{
    t_class* c;

    c = class_new("zpy", (method)zpy_new, (method)zpy_free,
                  (long)sizeof(t_zpy), 0L /* leave NULL!! */, A_GIMME, 0);

    // methods
    class_addmethod(c, (method)zpy_import,   "import",   A_SYM, 0);
    class_addmethod(c, (method)zpy_eval,     "eval",     A_GIMMEBACK, 0);
    class_addmethod(c, (method)zpy_exec,     "exec",     A_SYM, 0);
    class_addmethod(c, (method)zpy_execfile, "execfile", A_SYM, 0);
    class_addmethod(c, (method)zpy_code,     "code",     A_GIMMEBACK, 0);

    // attributes
    CLASS_ATTR_SYM(c, "name", 0, t_zpy, p_name);
    CLASS_ATTR_CHAR(c, "debug", 0, t_zpy, p_debug);
    CLASS_ATTR_SYM(c, "file", 0, t_zpy, p_code_filepath);
    CLASS_ATTR_SYM(c, "pythonpath", 0, t_zpy, p_pythonpath);

    // activate for javascript wrapping
    c->c_flags = CLASS_FLAG_POLYGLOT;
    class_register(CLASS_NOBOX, c);
    zpy_class = c;

}

void zpy_free(t_zpy* x)
{
    ;
}


void* zpy_new(t_symbol* s, long argc, t_atom* argv)
{
    t_zpy* x = NULL;

    // object instantiation, NEW STYLE
    if ((x = (t_zpy*)object_alloc(zpy_class))) {
        // Initialize values

        x->p_name = symbol_unique();
        x->p_pythonpath = gensym("");
        x->p_debug = 1;
        x->p_code_filepath = gensym("");

        // process @arg attributes
        attr_args_process(x, argc, argv);

    }
    return (x);
}


void zpy_log(t_zpy* x, char* fmt, ...)
{
    if (x->p_debug) {
        char msg[PY_MAX_LOG_CHAR];

        va_list va;
        va_start(va, fmt);
        vsprintf(msg, fmt, va);
        va_end(va);

        post("[zpy %s]: %s", x->p_name->s_name, msg);
    }
}


void zpy_error(t_zpy* x, char* fmt, ...)
{
    char msg[PY_MAX_ERR_CHAR];

    va_list va;
    va_start(va, fmt);
    vsprintf(msg, fmt, va);
    va_end(va);

    error("[zpy %s]: %s", x->p_name->s_name, msg);
}


// ---------------------------------------------------------------------------
// Handlers

void zpy_handle_error(t_zpy* x, char* fmt, ...)
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

        error("[zpy %s] %s: %s", x->p_name->s_name, msg, pvalue_str);
    }
}

t_max_err zpy_handle_float_output(t_zpy* x, PyObject* pfloat, t_atom* rv)
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
        atom_setobj(
            rv,
            object_new(gensym("nobox"), gensym("atomarray"), 1, atom_result));
    }
    Py_XDECREF(pfloat);
    return MAX_ERR_NONE;

error:
    zpy_handle_error(x, "zpy_handle_float_output failed");
    Py_XDECREF(pfloat);
    return MAX_ERR_GENERIC;
}

t_max_err zpy_handle_long_output(t_zpy* x, PyObject* plong, t_atom* rv)
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
        atom_setobj(
            rv,
            object_new(gensym("nobox"), gensym("atomarray"), 1, atom_result));
    }
    Py_XDECREF(plong);
    return MAX_ERR_NONE;

error:
    zpy_handle_error(x, "zpy_handle_long_output failed");
    Py_XDECREF(plong);
    return MAX_ERR_GENERIC;
}

t_max_err zpy_handle_string_output(t_zpy* x, PyObject* pstring, t_atom* rv)
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
        atom_setobj(
            rv,
            object_new(gensym("nobox"), gensym("atomarray"), 1, atom_result));
    }
    Py_XDECREF(pstring);
    return MAX_ERR_NONE;

error:
    zpy_handle_error(x, "zpy_handle_string_output failed");
    Py_XDECREF(pstring);
    return MAX_ERR_GENERIC;
}

t_max_err zpy_handle_list_output(t_zpy* x, PyObject* plist, t_atom* rv)
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
            zpy_error(x, "cannot convert py list of length 0 to atoms");
            goto error;
        }

        if (seq_size > PY_MAX_ATOMS) {
            zpy_log(x, "dynamically increasing size of atom array");
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

            if (PyFloat_Check(item)) {
                float float_item = PyFloat_AsDouble(item);
                if (float_item == -1.0) {
                    if (PyErr_Occurred())
                        goto error;
                }
                atom_setfloat(atoms + i, float_item);
                zpy_log(x, "%d float: %f\n", i, float_item);
                i++;
            }

            if (PyLong_Check(item)) {
                long long_item = PyLong_AsLong(item);
                if (long_item == -1) {
                    if (PyErr_Occurred())
                        goto error;
                }
                atom_setlong(atoms + i, long_item);
                zpy_log(x, "%d long: %ld\n", i, long_item);
                i++;
            }

            // only for numpy int64 (not recognized by PyLong_Check)
            // if (PyNumber_Check(item)) {
            //     long long_item = PyLong_AsLong(item);
            //     if (long_item == -1) {
            //         if (PyErr_Occurred())
            //             goto error;
            //     }
            //     atom_setlong(atoms + i, long_item);
            //     zpy_log(x, "%d long: %ld\n", i, long_item);
            //     i++;
            // }

            if (PyUnicode_Check(item)) {
                const char* unicode_item = PyUnicode_AsUTF8(item);
                if (unicode_item == NULL) {
                    goto error;
                }
                atom_setsym(atoms + i, gensym(unicode_item));
                zpy_log(x, "%d unicode: %s\n", i, unicode_item);
                i++;
            }
            Py_DECREF(item);
        }

        atom_setobj(
            rv,
            object_new(gensym("nobox"), gensym("atomarray"), (long)i, atoms));
        // atom_setobj(rv, object_new(gensym("nobox"), gensym("atomarray"), 1,
        // atoms));
        zpy_log(x, "end iter op: %d", i);
        if (is_dynamic) {
            zpy_log(x, "restoring to static atom array");
            atom_dynamic_end(atoms_static, atoms);
        }
    }

    Py_XDECREF(plist);
    return MAX_ERR_NONE;

error:
    zpy_handle_error(x, "zpy_handle_list_output failed");
    Py_XDECREF(plist);
    return MAX_ERR_GENERIC;
}

t_max_err zpy_handle_dict_output(t_zpy* x, PyObject* pdict, t_atom* rv)
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
                               Py_single_input, x->p_globals, x->p_globals);

        if (pfun_co == NULL) {
            zpy_error(x, "out_dict function code object is NULL");
            goto error;
        }

        pfun = PyDict_GetItemString(x->p_globals, "__py_maxmsp_out_dict");
        if (pfun == NULL) {
            zpy_error(x, "retrieving out_dict func from globals failed");
            goto error;
        }

        pval = PyObject_CallFunctionObjArgs(pfun, pdict, NULL);
        if (pval == NULL) {
            zpy_error(x, "out_dict call failed to retrieve result");
            goto error;
        }

        if (PyList_Check(pval)) { // expecting a python list
            Py_XDECREF(pfun_co);
            return zpy_handle_list_output(x, pval, rv); // this decrefs pval
        } else {
            zpy_error(x, "expected list output got something else");
            goto error;
        }
    }

error:
    zpy_handle_error(x, "zpy_handle_dict_output failed");
    Py_XDECREF(pfun_co);
    Py_XDECREF(pval);
    return MAX_ERR_GENERIC;
}

t_max_err zpy_handle_output(t_zpy* x, PyObject* pval, t_atom* rv)
{
    if (pval == NULL) {
        zpy_error(x, "cannot handle NULL value");
        return MAX_ERR_GENERIC;
    }

    if (PyFloat_Check(pval)) {
        return zpy_handle_float_output(x, pval, rv);
    }

    else if (PyLong_Check(pval)) {
        return zpy_handle_long_output(x, pval, rv);
    }

    else if (PyUnicode_Check(pval)) {
        return zpy_handle_string_output(x, pval, rv);
    }

    else if (PySequence_Check(pval) && !PyBytes_Check(pval)
             && !PyByteArray_Check(pval)) {
        return zpy_handle_list_output(x, pval, rv);
    }

    else if (PyDict_Check(pval)) {
        return zpy_handle_dict_output(x, pval, rv);
    }

    else if (pval == Py_None) {
        return MAX_ERR_NONE;
    }

    else {
        zpy_error(x, "cannot handle his type of value");
        return MAX_ERR_GENERIC;
    }
}

t_max_err zpy_code(t_zpy* x, t_symbol* s, long argc, t_atom* argv, t_atom* rv)
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
        zpy_log(x, ">>> %s", text);
    } else {
        goto error;
    }

    post("Connecting to hello world server...\n");
    void* context = zmq_ctx_new();
    void* requester = zmq_socket(context, ZMQ_REQ);
    zmq_connect(requester, ZPY_ADDRESS);


    char buffer[10];
    zmq_send(x->requester, text, textsize, 0);
    zmq_recv(requester, buffer, 10, 0);
    post("Received): '%s'\n", buffer);

    sysmem_freeptr(text);
    zmq_close(requester);
    zmq_ctx_destroy(context);

    if (buffer) {
        zpy_handle_output(x, buffer, 10);
    } else {
        goto error;
    }

    return MAX_ERR_NONE;

error:
    zpy_handle_error(x, "zpy code failed");
    return MAX_ERR_GENERIC;
}


t_max_err zpy_import(t_zpy* x, t_symbol* s)
{
    PyObject* x_module = NULL;

    if (s != gensym("")) {
        x_module = PyImport_ImportModule(s->s_name);
        
        if (x_module == NULL) {
            goto error;
        }

        PyDict_SetItemString(x->p_globals, s->s_name, x_module);
        zpy_log(x, "imported: %s", s->s_name);
        return MAX_ERR_NONE;
    }    

error:
    zpy_handle_error(x, "import %s", s->s_name);
    return MAX_ERR_GENERIC;
}


t_max_err zpy_eval(t_zpy* x, t_symbol* s, long argc, t_atom* argv,
                    t_atom* rv)
{
    char* py_argv = atom_getsym(argv)->s_name;
    zpy_log(x, "%s %s", s->s_name, py_argv);

    PyObject* pval = PyRun_String(py_argv, Py_eval_input, x->p_globals,
                                  x->p_globals);

    if (pval != NULL) {
        zpy_handle_output(x, pval, rv);
        return MAX_ERR_NONE;
    } else {
        zpy_handle_error(x, "eval %s", py_argv);
        return MAX_ERR_GENERIC;
    }
}


t_max_err zpy_execfile(t_zpy* x, t_symbol* s)
{
    PyObject* pval = NULL;
    FILE* fhandle = NULL;

    if (s != gensym("")) {
        // set x->p_code_filepath
        zpy_locate_path_from_symbol(x, s);
    }

    if (s == gensym("") || x->p_code_filepath == gensym("")) {
        zpy_error(x, "could not set filepath");
        goto error;
    }

    // assume x->p_code_filepath has be been set without errors

    zpy_log(x, "pathname: %s", x->p_code_filepath->s_name);
    fhandle = fopen(x->p_code_filepath->s_name, "r+");

    if (fhandle == NULL) {
        zpy_error(x, "could not open file");
        goto error;
    }

    pval = PyRun_File(fhandle, x->p_code_filepath->s_name, Py_file_input,
                      x->p_globals, x->p_globals);
    if (pval == NULL) {
        fclose(fhandle);
        goto error;
    }

    // success cleanup
    fclose(fhandle);
    Py_DECREF(pval);
    return MAX_ERR_NONE;

error:
    zpy_handle_error(x, "execfile failed");
    Py_XDECREF(pval);
    return MAX_ERR_GENERIC;
}


t_max_err zpy_exec(t_zpy* x, t_symbol* s)
{
    PyObject* pval = NULL;

    if (s == gensym("")) {
        zpy_log(x, "no input given");
        goto error;
    }

    pval = PyRun_String(s->s_name, Py_single_input, x->p_globals,
                        x->p_globals);
    if (pval == NULL) {
        goto error;
    }

    // success cleanup
    Py_DECREF(pval);
    zpy_log(x, "exec %s", s->s_name);
    return MAX_ERR_NONE;

error:
    zpy_handle_error(x, "exec %s", s->s_name);
    Py_XDECREF(pval);
    return MAX_ERR_GENERIC;
}