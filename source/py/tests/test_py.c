
/* python */
#define PY_SSIZE_T_CLEAN
#include <Python.h>


/* --------------------------------------- */
// types

typedef struct _py {
    PyObject* p_globals;
} t_py;


typedef enum _bool {
    false,
    true
} bool;

/* --------------------------------------- */
// forward func declarations

void py_import(t_py* x, char* args);
void py_eval(t_py* x, char* args);
void py_exec(t_py* x, char* args);
void py_execfile(t_py* x, char* args);
void py_run(t_py* x, char* args);
void py_pipe(t_py* x, char* args);

int main(int argc, char* argv[])
{
    t_py obj = { .p_globals = NULL };
    t_py* x = &obj;

    Py_Initialize();

    // python init
    PyObject* main_module = PyImport_AddModule("__main__"); // borrowed reference
    x->p_globals = PyModule_GetDict(main_module); // borrowed reference
    int err = PyDict_SetItemString(x->p_globals, "__builtins__", PyEval_GetBuiltins());
    if (err == -1)
        return err;



    if (argc > 2) {
        if (strcmp(argv[1], "import") == 0)
            py_import(x, argv[2]);
        if (strcmp(argv[1], "eval") == 0)
            py_eval(x, argv[2]);
        if (strcmp(argv[1], "exec") == 0)
            py_exec(x, argv[2]);
        if (strcmp(argv[1], "execfile") == 0)
            py_execfile(x, argv[2]);
        if (strcmp(argv[1], "run") == 0)
            py_run(x, argv[2]);
        if (strcmp(argv[1], "pipe") == 0)
            py_pipe(x, argv[2]);
    } else {
        printf("usage: test [import, eval, exec, execfile, run, pipe] args\n");
    }

    Py_FinalizeEx();
    return 0;
}

void py_handle_error(void)
{
    if (PyErr_Occurred()) {
        PyErr_Print();
    }
}

static void print(PyObject *o)
{
    PyObject_Print(o, stdout, Py_PRINT_RAW);
    printf("\n");
}

// static void print_repr(PyObject *o)
// {
//     PyObject_Print(o, stdout, 0);
// }

void py_handle_float_output(t_py* x, PyObject* pfloat, bool free_now)
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

        printf("float: %f\n", float_result);
    }

    if (free_now) {
        Py_XDECREF(pfloat);
    }
    return;

error:
    py_handle_error();
    Py_XDECREF(pfloat);
}


void py_handle_long_output(t_py* x, PyObject* plong, bool free_now)
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
        printf("long: %ld\n", long_result);
    }

    if (free_now) {
        Py_XDECREF(plong);
    }
    return;

error:
    py_handle_error();
    Py_XDECREF(plong);
}


void py_handle_string_output(t_py* x, PyObject* pstring, bool free_now)
{
    char buffer[100];

    if (pstring == NULL) {
        goto error;
    }

    if (PyUnicode_Check(pstring)) {
        const char* unicode_result = PyUnicode_AsUTF8(pstring);
        if (unicode_result == NULL) {
            goto error;
        }
        strcpy(buffer, unicode_result);
        printf("unicode: %s\n", buffer);
    }

    if (free_now) {
        Py_XDECREF(pstring);
    }
    return;

error:
    py_handle_error();
    Py_XDECREF(pstring);
}


void py_handle_list_output(t_py* x, PyObject* plist, bool free_now)
{
    if (plist == NULL) {
        goto error;
    }

    if (PySequence_Check(plist) && !PyUnicode_Check(plist)
        && !PyBytes_Check(plist) && !PyByteArray_Check(plist)) {
        PyObject* iter = NULL;
        PyObject* item = NULL;
        int i = 0;

        Py_ssize_t seq_size = PySequence_Length(plist);

        if (seq_size == 0) {
            printf("cannot convert py list of length 0 to atoms");
            goto error;
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
                printf("%d long: %ld\n", i, long_item);
                i++;
            }

            if PyFloat_Check (item) {
                float float_item = PyFloat_AsDouble(item);
                if (float_item == -1.0) {
                    if (PyErr_Occurred())
                        goto error;
                }
                printf("%d float: %f\n", i, float_item);
                i++;
            }

            if PyUnicode_Check (item) {
                const char* unicode_item = PyUnicode_AsUTF8(item);
                if (unicode_item == NULL) {
                    goto error;
                }
                printf("%d unicode: %s\n", i, unicode_item);
                i++;
            }
            Py_DECREF(item);
        }

    }

    if (free_now) {
        Py_XDECREF(plist);
    }
    return;

error:
    py_handle_error();
    Py_XDECREF(plist);

}


void py_handle_output(t_py* x, PyObject* pval)
{
    py_handle_float_output(x, pval, 0);
    py_handle_long_output(x, pval, 0);
    py_handle_string_output(x, pval, 0);
    py_handle_list_output(x, pval, 0);

    // final cleanup
    Py_XDECREF(pval);
    return;
}

//--------------------------------------------------------------------------

void py_import(t_py* x, char* args)
{
    PyObject* x_module = NULL;

    if (args != NULL) {
        x_module = PyImport_ImportModule(args); // x_module borrrowed ref
        if (x_module == NULL) {
            PyErr_SetString(PyExc_ImportError, "Ooops again.");
            goto error;
        }
        PyDict_SetItemString(x->p_globals, args, x_module);
        printf("imported: %s\n", args);
    }
    // else goto error;

error:
    py_handle_error();
}


void py_run(t_py* x, char* args)
{
    PyObject* pval = NULL;
    FILE* fhandle = NULL;
    int ret = 0;

    if (args == NULL) {
        printf("%s: could not retrieve args\n", args);
        goto error;
    }

    pval = Py_BuildValue("s", args); // new reference
    if (pval == NULL) {
        goto error;
    }

    fhandle = _Py_fopen_obj(pval, "r+");
    if (fhandle == NULL) {
        printf("could not open file '%s'\n", args);
        goto error;
    }

    ret = PyRun_SimpleFile(fhandle, args);
    if (ret == -1) {
        goto error;
    }

    // success
    fclose(fhandle);
    Py_DECREF(pval);

error:
    py_handle_error();
    Py_XDECREF(pval);
}


void py_execfile(t_py* x, char* args)
{
    PyObject* pval = NULL;
    FILE* fhandle = NULL;

    if (args == NULL) {
        printf("execfile: could not retrieve arg: %s\n", args);
        goto error;
    }

    fhandle = fopen(args, "r");
    if (fhandle == NULL) {
        printf("could not open file '%s'\n", args);
        goto error;
    }

    pval = PyRun_File(fhandle, args, Py_file_input, x->p_globals,
                      x->p_globals);
    if (pval == NULL) {
        fclose(fhandle);
        goto error;
    }

    // success cleanup
    fclose(fhandle);
    Py_DECREF(pval);

error:
    py_handle_error();
    Py_XDECREF(pval);
}


void py_exec(t_py* x, char* args)
{
    PyObject* pval = NULL;

    if (args == NULL) {
        printf("exec: could not retrieve args: %s\n", args);
        goto error;
    }

    pval = PyRun_String(args, Py_single_input, x->p_globals, x->p_globals);
    if (pval == NULL) {
        goto error;
    }

    // success cleanup
    Py_DECREF(pval);

error:
    py_handle_error();
    Py_XDECREF(pval);
}


void py_eval(t_py* x, char* args)
{
    PyObject* pval = NULL;
    PyObject* locals = NULL;

    if (args == NULL) {
        printf("eval: could not retrieve quoted args: %s\n", args);
        goto error;
    }

    locals = PyDict_New();
    if (locals == NULL) {
        goto error;
    }

    pval = PyRun_String(args, Py_eval_input, x->p_globals, locals);
    if (pval == NULL) {
        goto error;
    }

    py_handle_output(x, pval);
    return;

error:
    py_handle_error();
    Py_XDECREF(pval);
}




void py_pipe(t_py* x, char* args)
{
    PyObject* pargstr = NULL;
    PyObject* pstr = NULL;
    PyObject* list = NULL;
    PyObject* item = NULL;
    PyObject* funcs = NULL;
    PyObject* funcs_iter = NULL;
    PyObject* func = NULL;
    PyObject* pval = NULL;

    printf("args: %s\n", args);


    pargstr = PyUnicode_FromString(args);
    if (pargstr == NULL) {
        printf("could not convert cstring to py unicode string\n");
        goto error;
    }

    list = PyUnicode_Split(pargstr, NULL, -1);
    if (list == NULL) {
        printf("could not not split py unicode string into py list\n");
        goto error;
    }

    Py_ssize_t argc = PyList_Size(list);
    printf("argc: %ld\n", argc);

    if (argc < 2) {
        printf("pipe needs at least two arguments.\n");
        goto error;
    }

    pstr = PyList_GetItem(list, 0);
    print(pstr);
    // print_repr(pstr);
    if (pstr == NULL) {
        printf("could not retrieve input value\n");
        goto error;
    }

    pval = PyNumber_Long(pstr);
    if (pval == NULL) {
        printf("input value is not a int\n");
        pval = PyNumber_Float(pstr);
        if (pval == NULL) {
            printf("input value is a string\n");
            pval = pstr;
        } else {
            printf("input value is a float\n");
        }
    } else {
        printf("input value is an int\n");
    }

    funcs = PyList_GetSlice(list, 1, argc);
    print(funcs);
    if (funcs == NULL || !PyList_Check(funcs)) {
        printf("could not retrieve function names\n");
        goto error;
    }
    funcs_iter = PyObject_GetIter(funcs);
    if (funcs_iter == NULL) {
        goto error;
    }

    PyObject* builtins = PyDict_GetItemString(x->p_globals, "__builtins__");

    while ((item = PyIter_Next(funcs_iter)) != NULL) {
        print(item);

        func = PyDict_GetItemWithError(x->p_globals, item);
        if (func == NULL) {
            printf("could not retrieve callable name from globals dicts\n");
            printf("trying to get from builtins\n");
            if (PyDict_Contains(builtins, item)) {
                func = PyDict_GetItemWithError(builtins, item);
            }
            else {
                printf("not a builtin nor in globals\n");
                goto error;
            }
        }
        if (func == NULL) {
            printf("unable to to retrieve func without error\n");
            goto error;
        }

        print(func);

        if (!PyCallable_Check(func)) {
            printf("object retrieved is not a callable\n");
            goto error;
        }

        pval = PyObject_CallFunctionObjArgs(func, pval, NULL);
        if (pval == NULL) {
            printf("error occurred returning output from func\n");
            goto error;
        }
        Py_DECREF(func);
        Py_DECREF(item);
    }
    Py_XDECREF(funcs_iter);

    if (pval != NULL) {
        py_handle_output(x, pval);
        Py_XDECREF(list);
        Py_XDECREF(funcs);
        Py_XDECREF(pstr);
        Py_XDECREF(pval);
        return;
    }

error:
    py_handle_error();
    Py_XDECREF(list);
    Py_XDECREF(funcs);
    Py_XDECREF(pstr);
    Py_XDECREF(pval);
}


void py_pipe2(t_py* x, char* args)
{
    PyObject* pipe_co = NULL;
    PyObject* pipe_fun = NULL;    
    PyObject* pval = NULL;
    PyObject* p_str = NULL;

    pipe_co = PyRun_String(
        "def pipe(arg):\n"
            "\targs = arg.split()\n"
            "\tval = eval(args[0])\n"
            "\tfuncs = [eval(f) for f in args[1:]]\n"
            "\tfor f in funcs:\n"
                "\t\tval = f(val)\n"
            "\treturn val\n",
            Py_single_input, x->p_globals, x->p_globals);

    if (pipe_co == NULL) {
        printf("pipe func is NULL");
        goto error;
    }

    p_str = PyUnicode_FromString(args);
    if (p_str == NULL) {
        printf("cstr -> pyunicode conversion failed");
        goto error;
    }

    pipe_fun = PyDict_GetItemString(x->p_globals, "pipe");
    if (pipe_fun == NULL) {
        printf("retrieving pipe func from globals failed");
        goto error;
    }

    pval = PyObject_CallFunctionObjArgs(pipe_fun, p_str, NULL);

    if (pval != NULL) {
        py_handle_output(x, pval);
        Py_XDECREF(pipe_co);
        Py_XDECREF(p_str);
        // Py_XDECREF(pipe_fun);
        Py_XDECREF(pval);

        return;
    } else {
        goto error;
    }

error:
    py_handle_error();
    Py_XDECREF(pipe_co);
    Py_XDECREF(p_str);
    // Py_XDECREF(pipe_fun);
    Py_XDECREF(pval);
}