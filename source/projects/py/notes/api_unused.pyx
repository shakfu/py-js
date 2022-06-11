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

