/*
==> CURRENTLY UNUSED in any code

General functions for conversion between Max atoms and PyObjects

These have been extracted from more specialized code as a step
towards possible refactoring.

*/

#include "ext.h"
#include "ext_obex.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#define PY_MAX_ATOMS 128
#define PY_MAX_LOG_CHAR 500
#define PY_MAX_ERR_CHAR PY_MAX_LOG_CHAR

// ---------------------------------------------------------------------------------------
// Forward Declarations

static PyObject* t_atom_to_PyObject(t_atom* atom);
static PyObject* t_atom_list_to_PyObject_list(int argc, t_atom* argv);
static void PyObject_to_atom(PyObject* value, t_atom* atom);
static void new_list_from_sequence(PyObject* seq, int* argc, t_atom** argv);
static void emit_outlet_message(PyObject* value, void* x_outlet);

void py_log(char* fmt, ...);
void py_error(char* fmt, ...);
void py_handle_error(char* fmt, ...);
void py_handle_float_output(PyObject* pfloat, void* x_outlet);
void py_handle_long_output(PyObject* plong, void* x_outlet);
void py_handle_string_output(PyObject* pstring, void* x_outlet);
void py_handle_list_output(PyObject* plist, void* x_outlet);
void py_handle_output(PyObject* pval, void* x_outlet);
PyObject* py_atoms_to_list(long argc, t_atom* argv, int start_from);

// ---------------------------------------------------------------------------------------

static PyObject* t_atom_to_PyObject(t_atom* atom)
{
    post("t_atom_to_PyObject start");

    switch (atom->a_type) {

    case A_LONG:
        post("int: %i", atom_getlong(atom));
        return PyLong_FromLong(atom_getlong(atom));

    case A_FLOAT:
        post("float: %f", atom_getfloat(atom));
        return PyFloat_FromDouble(atom_getfloat(atom));

    case A_SYM:
        post("symbol: %s", atom_getsym(atom)->s_name);
        return PyUnicode_FromString(atom_getsym(atom)->s_name);

    case A_NOTHING:
        Py_RETURN_NONE;

    default:
        post("Warning: type %d unsupported for conversion to Python.",
             atom->a_type);
        Py_RETURN_NONE;
    }
}

static PyObject* t_atom_list_to_PyObject_list(int argc, t_atom* argv)
{
    post("t_atom_list_to_PyObject_list start");

    PyObject* list = PyTuple_New(argc);
    int i;
    for (i = 0; i < argc; i++) {
        PyObject* value = t_atom_to_PyObject(&argv[i]);
        PyTuple_SetItem(list, i, value); // pass value ref to the tuple
    }
    return list;
}

static void PyObject_to_atom(PyObject* value, t_atom* atom)
{
    post("PyObject_to_atom start");

    if (value == Py_True)
        atom_setlong(atom, 1);
    else if (value == Py_False)
        atom_setlong(atom, 0);
    else if (PyFloat_Check(value))
        atom_setfloat(atom, (float)PyFloat_AsDouble(value));
    else if (PyLong_Check(value))
        atom_setlong(atom, (float)PyLong_AsLong(value));
    else if (PyUnicode_Check(value))
        atom_setsym(atom, gensym(PyUnicode_AsUTF8(value)));
    else
        atom_setsym(atom, gensym("error"));
}

static void new_list_from_sequence(PyObject* seq, int* argc, t_atom** argv)
{
    post("new_list_from_sequence start");

    Py_ssize_t len = 0;
    Py_ssize_t i;

    if (PyList_Check(seq)) {
        len = PyList_Size(seq);
        *argv = (t_atom*)malloc(len * sizeof(t_atom));
        for (i = 0; i < len; i++) {
            PyObject* elem = PyList_GetItem(seq, i);
            PyObject_to_atom(elem, (*argv) + i);
        }
    }
    *argc = (int)len;
}

static void emit_outlet_message(PyObject* value, void* x_outlet)
{
    post("emit_outlet_message start");

    if (value == Py_True)
        outlet_float(x_outlet, 1.0);
    else if (value == Py_False)
        outlet_float(x_outlet, 0.0);

    else if (PyFloat_Check(value))
        outlet_float(x_outlet, (float)PyFloat_AsDouble(value));
    else if (PyLong_Check(value))
        outlet_float(x_outlet, (float)PyLong_AsLong(value));
    else if (PyUnicode_Check(value))
        outlet_anything(x_outlet, gensym(PyUnicode_AsUTF8(value)), 0, NIL);

    else if (PyList_Check(value)) {
        t_atom* argv = NULL;
        int argc = 0;
        new_list_from_sequence(value, &argc, &argv);

        if (argc > 0) {
            if (argv[0].a_type == A_SYM) {
                outlet_anything(x_outlet, atom_getsym(&argv[0]), argc - 1,
                                argv + 1);
            } else {
                // outlet_list(x_outlet, &s_list, argc, argv);
                outlet_list(x_outlet, NULL, argc, argv);
            }
        }
        if (argv)
            free(argv);
    }
}


// ---------------------------------------------------------------------------------------

void py_log(char* fmt, ...)
{
    char msg[PY_MAX_LOG_CHAR];

    va_list va;
    va_start(va, fmt);
    vsprintf(msg, fmt, va);
    va_end(va);

    post("%s", msg);

}


void py_error(char* fmt, ...)
{
    char msg[PY_MAX_ERR_CHAR];

    va_list va;
    va_start(va, fmt);
    vsprintf(msg, fmt, va);
    va_end(va);

    error("%s", msg);
}


void py_handle_error(char* fmt, ...)
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

        error("%s: %s", msg, pvalue_str);
    }
}


void py_handle_float_output(PyObject* pfloat, void* x_outlet)
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
        outlet_float(x_outlet, float_result);
    }
    Py_XDECREF(pfloat);
    return;

error:
    py_handle_error("py_handle_float_output failed");
    Py_XDECREF(pfloat);
}


void py_handle_long_output(PyObject* plong, void* x_outlet)
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
        outlet_int(x_outlet, long_result);
    }

    Py_XDECREF(plong);
    return;

error:
    py_handle_error("py_handle_long_output failed");
    Py_XDECREF(plong);
}


void py_handle_string_output(PyObject* pstring, void* x_outlet)
{
    if (pstring == NULL) {
        goto error;
    }

    if (PyUnicode_Check(pstring)) {
        const char* unicode_result = PyUnicode_AsUTF8(pstring);
        if (unicode_result == NULL) {
            goto error;
        }
        outlet_anything(x_outlet, gensym(unicode_result), 0, NIL);
    }

    Py_XDECREF(pstring);
    return;

error:
    py_handle_error("py_handle_string_output failed");
    Py_XDECREF(pstring);
}


void py_handle_list_output(PyObject* plist, void* x_outlet)
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
        py_log("seq_size: %d", seq_size);

        if (seq_size == 0) {
            py_error("cannot convert py list of length 0 to atoms");
            goto error;
        }

        if (seq_size > PY_MAX_ATOMS) {
            py_log("dynamically increasing size of atom array");
            atoms = atom_dynamic_start(atoms_static, PY_MAX_ATOMS,
                                       seq_size + 1);
            is_dynamic = 1;

        } else {
            atoms = atoms_static;
        }

        if ((iter = PyObject_GetIter(plist)) == NULL) {
            goto error;
        }
        py_log("seq_size2: %d", seq_size);

        while ((item = PyIter_Next(iter)) != NULL) {
            if (PyLong_Check(item)) {
                long long_item = PyLong_AsLong(item);
                if (long_item == -1) {
                    if (PyErr_Occurred())
                        goto error;
                }
                atom_setlong(atoms + i, long_item);
                py_log("%d long: %ld\n", i, long_item);
                i++;
            }

            if (PyFloat_Check(item)) {
                float float_item = PyFloat_AsDouble(item);
                if (float_item == -1.0) {
                    if (PyErr_Occurred())
                        goto error;
                }
                atom_setfloat(atoms + i, float_item);
                py_log("%d float: %f\n", i, float_item);
                i++;
            }

            if (PyUnicode_Check(item)) {
                const char* unicode_item = PyUnicode_AsUTF8(item);
                if (unicode_item == NULL) {
                    goto error;
                }
                atom_setsym(atoms + i, gensym(unicode_item));
                py_log("%d unicode: %s\n", i, unicode_item);
                i++;
            }
            Py_DECREF(item);
        }

        outlet_list(x_outlet, NULL, i, atoms);
        py_log("end iter op: %d", i);

        if (is_dynamic) {
            py_log("restoring to static atom array");
            atom_dynamic_end(atoms_static, atoms);
        }
    }

    Py_XDECREF(plist);
    return;

error:
    py_handle_error("py_handle_list_output failed");
    Py_XDECREF(plist);
}


void py_handle_output(PyObject* pval, void* x_outlet)
{
    if (pval == NULL) {
        py_error("cannot handle NULL value");
        return;
    }

    if (PyFloat_Check(pval)) {
        py_handle_float_output(pval, x_outlet);
        return;
    }

    else if (PyLong_Check(pval)) {
        py_handle_long_output(pval, x_outlet);
        return;
    }

    else if (PyUnicode_Check(pval)) {
        py_handle_string_output(pval, x_outlet);
        return;
    }

    else if (PySequence_Check(pval) && !PyBytes_Check(pval)
             && !PyByteArray_Check(pval)) {
        py_handle_list_output(pval, x_outlet);
        return;
    }

    else if (pval == Py_None) {
        return;
    }

    else {
        py_error("cannot handle his type of value");
        return;
    }
}

/*--------------------------------------------------------------------------*/
// TRANSLATORS

PyObject* py_atoms_to_list(long argc, t_atom* argv, int start_from)
{

    PyObject* plist = NULL; // python list

    if ((plist = PyList_New(0)) == NULL) {
        py_error("could not create an empty python list");
        goto error;
    }

    for (int i = start_from; i < argc; i++) {
        switch ((argv + i)->a_type) {
        case A_FLOAT: {
            double c_float = atom_getfloat(argv + i);
            PyObject* p_float = PyFloat_FromDouble(c_float);
            if (p_float == NULL) {
                goto error;
            }
            PyList_Append(plist, p_float);
            Py_DECREF(p_float);
            break;
        }
        case A_LONG: {
            PyObject* p_long = PyLong_FromLong(atom_getlong(argv + i));
            if (p_long == NULL) {
                goto error;
            }
            PyList_Append(plist, p_long);
            Py_DECREF(p_long);
            break;
        }
        case A_SYM: {
            PyObject* p_str = PyUnicode_FromString(
                atom_getsym(argv + i)->s_name);
            if (p_str == NULL) {
                goto error;
            }
            PyList_Append(plist, p_str);
            Py_DECREF(p_str);
            break;
        }
        default:
            py_log("cannot process unknown type");
            break;
        }
    }
    return plist;

error:
    py_error("atom to list conversion failed");
    return NULL;
}

// -------------------------------------------------------------------------------------------
// Get PYTHONHOME from .mxo bundle


#if defined(__APPLE__) && (defined(PY_STATIC_EXT) || defined(PY_SHARED_PKG) || defined(PY_FWK_EXT))
#include <CoreFoundation/CoreFoundation.h>
#include <libgen.h>
#endif

#if defined(__APPLE__) && (defined(PY_STATIC_EXT) || defined(PY_SHARED_PKG) || defined(PY_FWK_EXT))
CFBundleRef py_global_bundle;
#endif

#if defined(_WIN64) && defined(PY_STATIC_EXT)
static char* py_global_external_path[MAX_PATH_CHARS];
#endif




#if defined(__APPLE__) && defined(PY_STATIC_EXT)
void py_init_osx_set_home_static_ext(void) {
    // sets python_home to <bundle>/Resources folder

    wchar_t *python_home;

    CFURLRef resources_url;
    CFURLRef resources_abs_url;
    CFStringRef resources_str;
    const char* resources_path;

    // Look for a bundle using its using global bundle ref
    resources_url = CFBundleCopyResourcesDirectoryURL(py_global_bundle);
    resources_abs_url = CFURLCopyAbsoluteURL(resources_url);
    resources_str = CFURLCopyFileSystemPath(resources_abs_url, kCFURLPOSIXPathStyle);
    resources_path = CFStringGetCStringPtr(resources_str, kCFStringEncodingUTF8);

    python_home = Py_DecodeLocale(resources_path, NULL);

    CFRelease(resources_str);
    CFRelease(resources_abs_url);
    CFRelease(resources_url);

    post("py resources_path: %s", resources_path);

    if (python_home == NULL) {
        error("unable to set python_home");
        return;
    }
    Py_SetPythonHome(python_home);
}
#endif



// NOT USED OR NECESSARY (HERE for archive)
#if defined(__APPLE__) && defined(PY_FWK_EXT)
void py_init_osx_set_home_framework_ext(void) {

    const char* relative_path = "Python.framework";

    wchar_t *python_home;

    CFURLRef resources_url;
    CFURLRef resources_abs_url;
    CFStringRef resources_str;
    CFStringRef relative_path_str;
    CFURLRef py_home_url;
    CFStringRef py_home_str;
    const char* py_home_path;

    // Look for a bundle using its using global bundle ref
    resources_url = CFBundleCopyResourcesDirectoryURL(py_global_bundle);
    resources_abs_url = CFURLCopyAbsoluteURL(resources_url);
    resources_str = CFURLCopyFileSystemPath(resources_abs_url, kCFURLPOSIXPathStyle);

    relative_path_str = CFStringCreateWithCString(kCFAllocatorDefault, relative_path, kCFStringEncodingASCII);
    py_home_url = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, resources_str, relative_path_str, FALSE);
    py_home_str = CFURLCopyFileSystemPath(py_home_url, kCFURLPOSIXPathStyle);
    py_home_path = CFStringGetCStringPtr(py_home_str, kCFStringEncodingUTF8);

    python_home = Py_DecodeLocale(resources_path, NULL);

    CFRelease(resources_str);
    CFRelease(resources_abs_url);
    CFRelease(resources_url);

    CFRelease(py_home_str);
    CFRelease(py_home_url);
    CFRelease(relative_path_str);

    post("py home path: %s", py_home_path);

    if (python_home == NULL) {
        error("unable to set python_home");
        return;
    }
    Py_SetPythonHome(python_home);
}
#endif



#if defined(__APPLE__) && defined(PY_SHARED_PKG)
void py_init_osx_set_home_shared_pkg(void) {
    // sets python_home to <package>/support/pythonX.Y folder

    wchar_t *python_home;

    CFURLRef bundle_url;
    CFURLRef bundle_abs_url;
    CFStringRef bundle_str;
    const char* bundle_path;

    const char* relative_path = "support/python" PY_VER;
    CFStringRef relative_path_str;
    CFURLRef externals_url;
    CFURLRef package_url;
    CFURLRef py_home_url;
    CFStringRef py_home_str;
    const char* py_home_path;

    // get self bundle path
    bundle_url = CFBundleCopyBundleURL(py_global_bundle);
    bundle_abs_url = CFURLCopyAbsoluteURL(bundle_url);
    bundle_str = CFURLCopyFileSystemPath(bundle_abs_url, kCFURLPOSIXPathStyle);
    bundle_path = CFStringGetCStringPtr(bundle_str, kCFStringEncodingUTF8);
    
    // get the absolute path of the <package>/support/pythonX.Y directory in a package
    externals_url = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, bundle_abs_url);
    package_url = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, externals_url);
    relative_path_str = CFStringCreateWithCString(kCFAllocatorDefault, relative_path, kCFStringEncodingASCII);
    py_home_url = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, package_url, relative_path_str, FALSE);
    py_home_str = CFURLCopyFileSystemPath(py_home_url, kCFURLPOSIXPathStyle);
    py_home_path = CFStringGetCStringPtr(py_home_str, kCFStringEncodingUTF8);

    CFRelease(bundle_str);
    CFRelease(bundle_abs_url);
    CFRelease(bundle_url);

    CFRelease(py_home_str);
    CFRelease(py_home_url);
    CFRelease(relative_path_str);
    CFRelease(package_url);
    CFRelease(externals_url);

    post("py bundle_path: %s", bundle_path);

    post("py home path: %s", py_home_path);

    python_home = Py_DecodeLocale(py_home_path, NULL);

    if (python_home == NULL) {
        error("unable to set python_home");
        return;
    }
    Py_SetPythonHome(python_home);
}
#endif


void py_init(t_py* x)
{
    #if defined(__APPLE__) && defined(PY_STATIC_EXT)
    py_init_osx_set_home_static_ext();
    #endif

    #if defined(__APPLE__) && defined(PY_FWK_EXT)
    py_init_osx_set_home_framework_ext();
    #endif

    #if defined(__APPLE__) && defined(PY_SHARED_PKG)
    py_init_osx_set_home_shared_pkg();
    #endif

    // ..
}

void ext_main(void* module_ref)
{ // example
    t_class* c;

    // ...

#if defined(__APPLE__) && (defined(PY_STATIC_EXT) || defined(PY_SHARED_PKG) || defined(PY_FWK_EXT))
    // set global bundle ref for macos case
    py_global_bundle = module_ref;
#endif
#if defined(_WIN64) && defined(PY_STATIC_EXT)
    // set external_path for win64 case
    GetModuleFileName(moduleRef, (LPCH)py_global_external_path,
                      sizeof(py_global_external_path));
    post("external path: %s", py_global_external_path);
#endif

}



