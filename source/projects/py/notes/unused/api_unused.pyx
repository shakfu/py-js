# place for 'archived' cython code


# ----------------------------------------------------------------------------
# String type

# This string type is totally useless in this context
# use python string instead!!

cdef class String:
    cdef mx.t_string* _str

    def __cinit__(self, bytes cstr):
        self._str = mx.string_new(cstr)

    def getptr(self) -> bytes:
        return mx.string_getptr(self._str)
            
    def reserve(self, long numbytes):
        mx.string_reserve(self._str, numbytes)

    def append(self, bytes s):
        mx.string_append(self._str, s)

    def chop(self, long numchars):
        mx.string_chop(self._str, numchars)


# ----------------------------------------------------------------------------
# TABLE type

# removed redundant methods in py.c and `api.PyExternal`, see `api.Table`
# for equivalent functionality.

cdef class PyExternal:
    # ...
    cdef bint table_exists(self, str table_name):
        """Return true if a table exists."""
        return px.py_table_exists(self.ptr, table_name.encode())

    cdef mx.t_max_err list_to_table(self, char* table_name, PyObject* plist):
        """Convert a Python list to a table."""
        return px.py_list_to_table(self.ptr, table_name, plist)

    cdef PyObject* table_to_list(self, char* table_name):
        """Convert a table to python list"""
        return px.py_table_to_list(self.ptr, table_name)


def table_exists(str name):
    """checks if a table exists."""

    cdef long **storage
    cdef long size

    result = mx.table_get(mx.gensym(name.encode('utf-8')), &storage, &size)

    if result == 0:
        success_bang()
    else:
        failure_bang()

    return result


def copy_list_to_table(list[int] xs, str name):
    """copies integers from a python list[int] to a max table"""
    
    cdef long **storage
    cdef long size

    length = len(xs)

    result = mx.table_get(str_to_sym(name), &storage, &size)

    if result == 0:
        if length <= size:
            for i, x in enumerate(xs):
                storage[0][i] = <long>x
        else:
            for i in range(size):
                storage[0][i] = <long>xs[i]


def get_table_as_list(str name):
    """gets integer content of a named max table as a python list"""

    cdef long **storage
    cdef long size
    cdef long value
    cdef list[int] xs = []

    result = mx.table_get(str_to_sym(name), &storage, &size)

    if result == 0:
        for i in range(size):
            value = storage[0][i]
            xs.append(<int>value)

    return xs


"""
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
"""


# ----------------------------------------------------------------------------
# Object methods


"""
You can make a patcher by passing a dictionary to object_new_typed() 
when making a "jpatcher". Using atom_setparse() and attr_args_dictionary() makes 
this relatively easy.

Use newobject_sprintf() to programmatically make an object in a patch. Actually, you
don't explicitly use a dictionary here! If you do want more control, so you can touch
the dictionary to customize it, then see the next bullet.

Use dictionary_sprintf() to make a dictionary to specify a box (i.e. specify class
with @maxclass attr). Then, make another dictionary and append your box dictionary to
it under the key "box" via dictionary_appenddictionary(). Finally, make your object
with newobject_fromdictionary().


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
"""

