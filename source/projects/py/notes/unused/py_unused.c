
// locate a folder in the max seach path
// from example by Luigi Castelli
// https://cycling74.com/forums/locatefolder
void locatefolder(t_symbol *foldername)
{
    char name[MAX_FILENAME_CHARS];
    char pathname[MAX_FILENAME_CHARS];
    short path;
    short err;
    t_max_err merr;
    t_fourcc type;
    
    strncpy_zero(name, foldername->s_name, MAX_FILENAME_CHARS);
    if (locatefile_extended(name, &path, &type, NULL, 0)) {
        error("folder %s not found", name);
    } else {
        post("folder %s, path %d", name, path);
    }
    
    err = path_topathname(path, name, pathname);
    if (err)
        error("path_topathname failed");

    post("pathname: %s", pathname);

    merr = path_toabsolutesystempath(path, name, pathname);
    if (merr)
        error("path_toabsolutesystempath failed");

    post("absolutesystempath: %s", pathname);
}


// table
bool py_table_exists(t_py* x, char* table_name);
t_max_err py_list_to_table(t_py* x, char* table_name, PyObject* plist);
PyObject* py_table_to_list(t_py* x, char* table_name);

// used to be in py.c

/*--------------------------------------------------------------------------*/
/* Max Datastructures Support */

/**
 * @brief      Determines if py table exists.
 *
 * @param      x           pointer to object structure
 * @param      table_name  The table name
 *
 * @return     True if py table exists, False otherwise.
 */
bool py_table_exists(t_py* x, char* table_name)
{
    long **storage, size;

    return (table_get(gensym(table_name), &storage, &size) == 0);
}

/**
 * @brief      Convert python list to Max table
 *
 * @param      x           pointer to object structure
 * @param      table_name  The table name
 * @param      plist       The python list
 *
 * @return     The t maximum error.
 */
t_max_err py_list_to_table(t_py* x, char* table_name, PyObject* plist)
{
    long **storage, size, value;

    Py_ssize_t len = 0;
    PyObject* elem = NULL;

    if (plist == NULL) {
        goto error;
    }

    if (!PyList_Check(plist)) {
        goto error;
    }

    len = PyList_Size(plist);


    if (table_get(gensym(table_name), &storage, &size)) {
        if (len > size)
            goto error;

        for (int i = 0; i < len; i++) {
            elem = PyList_GetItem(plist, i); // borrowed
            value = PyLong_AsLong(elem);
            *((*storage) + i) = value;
            py_debug(x, "storage[%d] = %d", i, value);
        }
    }
    Py_CLEAR(plist);
    // Py_XDECREF(plist);
    // Py_XDECREF(elem);
    return MAX_ERR_NONE;

error:
    py_handle_error(x, "plist to table failed");
    Py_CLEAR(plist);
    // Py_XDECREF(plist);
    // Py_XDECREF(elem);
    return MAX_ERR_GENERIC;
}

/**
 * @brief      Convert Max table to python list
 *
 * @param      x           pointer to object structure
 * @param      table_name  The table name
 *
 * @return     python list
 */
PyObject* py_table_to_list(t_py* x, char* table_name)
{

    PyObject* plist = NULL;
    long **storage, size, value;

    if ((plist = PyList_New(0)) == NULL) {
        py_debug(x, "could not create an empty python list");
        goto error;
    }

    if (table_get(gensym(table_name), &storage, &size) == 0) {
        for (int i = 0; i < size; i++) {
            value = *((*storage) + i);
            py_debug(x, "storage[%d] = %d", i, value);
            PyObject* p_long = PyLong_FromLong(value);
            if (p_long == NULL) {
                goto error;
            }
            PyList_Append(plist, p_long);
            Py_CLEAR(p_long);
            // Py_DECREF(p_long);
        }
        return plist;
    }

error:
    py_error(x, "table to list conversion failed");
    Py_RETURN_NONE;
}


# ----------------------------------------------------------------------------
# Object methods


"""
You can make a patcher by passing a dictionary to object_new_typed() 
when making a `patcher`. Using atom_setparse() and attr_args_dictionary() makes 
this relatively easy.

Use newobject_sprintf() to programmatically make an object in a patch. Actually, you
don't explicitly use a dictionary here! If you do want more control, so you can touch
the dictionary to customize it, then see the next bullet.

Use dictionary_sprintf() to make a dictionary to specify a box (i.e. specify class
with @maxclass attr). Then, make another dictionary and append your box dictionary to
it under the key "box" via dictionary_appenddictionary(). Finally, make your object
with newobject_fromdictionary().
"""

// not implemented
cdef void *object_new_typed(t_symbol *name_space, t_symbol *classname, long ac, t_atom *av)
cdef t_dictionary *class_cloneprototype(t_class *x)
cdef t_dictionary *dictionary_sprintf(char *fmt, ...)
cdef t_dictionary *object_dictionaryarg(long ac, t_atom *av)
cdef t_max_err atom_setparse(long *ac, t_atom **av, const char *parsestr)
cdef t_object *newobject_fromboxtext(t_object *patcher, const char *text)
cdef t_object *newobject_fromdictionary(t_object *patcher, t_dictionary *d)
cdef t_object *newobject_sprintf(t_object *patcher, const char *fmt, ...)
cdef void attr_args_dictionary(t_dictionary *x, short ac, t_atom *av)
cdef void attr_dictionary_check(void *x, t_dictionary *d)
cdef void attr_dictionary_process(void *x, t_dictionary *d)

