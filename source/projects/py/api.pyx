# api.pyx
"""
# api: max api wrapped by cython for use by `py` external

The main place to create and re-use cython wrappers and utilities which
access Max's c-api.

Using this file requires some knowledge of cython (https://cython.org).

The wrappers here are available for use by python code running on the
`py` external. First you have to send the `py` object an `import api`
message and then call one of the functions or classes in this file
making sure to prefix it with `api.`. For example:

        1.
   ( import api )
        |
        |                      2.
      [ py ] ------ ( api.post('hello world') )


A lot of the painful laborious work of creating header mappings has been
done (at least for the  max api) and can be reviewed (and corrected) in
the `api_max.pxd` file.

This provides the benefit that we can import the max api via its header
declarations as follows:

    cimport api_max as mx

Then you can start using the max api's symbols or functions by prefixing
with `mx.` For example

    gensym()    -> mx.gensym()
    post()      -> mx.post()
    ...

In addition the `py` external api is also mapped for use by cython
(see below) in api_py.pxd file:

    cimport api_py as px


Again please note any function exposed from the `py` external must
be prefixed as `px`:

    py_scan()   -> px.py_scan()


This separation of namespaces is clearly very useful when you are
wrapping code.


## Extension Types

We use cython extension types to wrap related C data structures and functions
in the Max api and provide a Python-like interface to them.

So far the following extension types are planned or implemented (partial or otherwise)

- [x] Atom
- [x] Atom Array: container for an array of atoms
- [x] Binbuf
- [x] Buffer
- [x] Database: SQLite database access
- [x] Dictionary: structured/hierarchical data that is both sortable and fast
- [x] Hash Table: hash table for mapping symbols to data
- [ ] Index Map: managed array of pointers
- [x] Linked List: doubly-linked-list
- [ ] Quick Map: a double hash with keys mapped to values and vice-versa
- [x] String Object: wrapper for C-strings with an API for manipulating them
- [ ] Symbol Object: wrapper for symbols
- [x] Table

- [x] PyExternal

Workarounds for max types which are not exposed in the c-api:

- coll: import and export the contents of a coll into a dict by
  sending a message to the dict object.

- jit.cellblock: link a coll to a cellblock and data sent to the
  coll will be sent to the cellblock.

- jit.matrix: can be populated via jit.fill from a coll


## Table of Contents

- imports
- compile time conditional imports
- compile time constants
- helper cdef functions (type-translation)
- extension types
- helper def functions
- test functions
"""

# ----------------------------------------------------------------------------
# imports

# cimport cython
# from cpython cimport PyFloat_AsDouble
# from cpython cimport PyLong_AsLong
from cpython.ref cimport PyObject
from libc.stdint cimport uintptr_t
# from libc.stdlib cimport malloc, free
# from libc.string cimport strcpy, strlen

cimport api_max as mx  # api is a cython keyword!
cimport api_msp as mp
cimport api_py as px

# ----------------------------------------------------------------------------
# compile-time conditional imports

cdef extern from *:
    """
    #define INCLUDE_NUMPY 0
    """
    bint INCLUDE_NUMPY

if INCLUDE_NUMPY:
    import numpy as np
    cimport numpy as np
    np.import_array()


# ----------------------------------------------------------------------------
# compile-time constants

DEF MAX_CHARS = 32767
DEF PY_MAX_ATOMS = 1024


# ----------------------------------------------------------------------------
# python c-api imports

# TODO: can't this be imported from cimport!
cdef extern from "Python.h":
    const char* PyUnicode_AsUTF8(object unicode)
    unicode PyUnicode_FromString(const char *u)

# ----------------------------------------------------------------------------
# helper cdef functions


cdef mx.t_symbol* str_to_sym(str string):
    """converts a python string to a t_symbol*

    api.gensym(str s) -> t_symbol*
    """
    return mx.gensym(string.encode('utf-8'))


cdef str sym_to_str(mx.t_symbol* symbol):
    """converts a max symbol to a python string

    api.sym_to_str(symbol) -> python str
    """
    return symbol.s_name.decode()


# ============================================================================
# EXTENSION TYPES

# ----------------------------------------------------------------------------
# Atom

cdef class Atom:
    """A wrapper class for a Max t_atom
    """
    cdef mx.t_atom *ptr
    cdef bint ptr_owner
    cdef int size

    def __cinit__(self):
        self.ptr_owner = False

    def __dealloc__(self):
        # De-allocate if not null and flag is set
        if self.ptr is not NULL and self.ptr_owner is True:
            mx.sysmem_freeptr(self.ptr)
            self.ptr = NULL

    def set_float(self, float f, int idx=0):
        mx.atom_setfloat(self.ptr + idx, f)

    def get_float(self, int idx=0) -> float:
        return <float>mx.atom_getfloat(self.ptr + idx)

    def set_long(self, long x, int idx=0):
        mx.atom_setlong(self.ptr + idx, x)

    def get_long(self, int idx=0) -> long:
        return <long>mx.atom_getlong(self.ptr + idx)

    def get_int(self, int idx=0) -> int:
        return <int>mx.atom_getlong(self.ptr + idx)

    def set_symbol(self, str symbol, int idx=0):
        mx.atom_setsym(self.ptr + idx, str_to_sym(symbol))

    cdef mx.t_symbol *get_symbol(self, int idx=0):
        return mx.atom_getsym(self.ptr + idx)

    def get_string(self, int idx=0) -> str:
        # return (self.get_symbol(idx).s_name).decode()
        return sym_to_str(self.get_symbol(idx))

    cdef bint is_symbol(self, int idx=0):
        return (self.ptr + idx).a_type == mx.A_SYM

    cdef bint is_long(self, int idx=0):
        return (self.ptr + idx).a_type == mx.A_LONG

    cdef bint is_float(self, int idx=0):
        return (self.ptr + idx).a_type == mx.A_FLOAT

    def to_list(self) -> list:
        _res = []
        for i in range(self.size):
            if self.is_symbol(i):
                _res.append(self.get_string(i))
            elif self.is_long(i):
                _res.append(self.get_int(i))
            elif self.is_float(i):
                _res.append(self.get_float(i))
        return _res

    # def to_np_array(self):
    #     return np.array(self.to_list(), dtype=np.float64)

    def display(self):
        for i in range(self.size):
            if self.is_float(i):
                print("is_float:", i)
            elif self.is_symbol(i):
                print("is_symbol:", i)
                s = self.get_string(i)
                print("string:", type(s))
            else:
                print("other:", i)

    # TODO: move to PyExternal
    # def create_object(self, classname: str, namespace: str = "box"):
    #     """Creates an object.

    #     :param      classname:  t_class class name
    #     :type       classname:  str
    #     :param      namespace:  The namespace i.e. CLASS_BOX | CLASS_NOBOX
    #     :type       namespace:  str
    #     """
    #     mx.object_new_typed(str_to_sym(namespace),
    #         str_to_sym(classname), self.size, self.ptr)

    @staticmethod
    cdef Atom from_ptr(mx.t_atom *ptr, int size, bint owner=False):
        # Call to __new__ bypasses __init__ constructor
        cdef Atom atom = Atom.__new__(Atom)
        atom.ptr = ptr
        atom.ptr_owner = owner
        atom.size = size
        return atom

    @staticmethod
    cdef Atom new(int size):
        cdef mx.t_atom *ptr = <mx.t_atom *>mx.sysmem_newptr(size * sizeof(mx.t_atom))
        if ptr is NULL:
            raise MemoryError
        return Atom.from_ptr(ptr, size, owner=True)

    @staticmethod
    cdef Atom from_seq(object seq):
        cdef int size = len(seq)
        cdef mx.t_atom *ptr = <mx.t_atom *>mx.sysmem_newptr(size * sizeof(mx.t_atom))
        if ptr is NULL:
            raise MemoryError

        cdef int i
        for i, obj in enumerate(seq):

            if isinstance(obj, float):
                mx.atom_setfloat(ptr+i, <float>obj)

            elif isinstance(obj, int):
                mx.atom_setlong(ptr+i, <long>obj)

            elif isinstance(obj, bytes):
                mx.atom_setsym(ptr+i, mx.gensym(obj))

            elif isinstance(obj, str):
                mx.atom_setsym(ptr+i, str_to_sym(obj))

            else:
                print("cannot convert:", obj)
                continue

        return Atom.from_ptr(ptr, size, owner=True)

# ----------------------------------------------------------------------------
# Table

cdef class Table:
    """A wrapper class to acess a pre-existing Max table
    """
    cdef str name
    cdef long **storage
    cdef readonly long size

    def __cinit__(self, str name):
        self.name = name
        check = mx.table_get(str_to_sym(name), &self.storage, &self.size)
        assert check == 0, f"table with name '{name}' doesn't exist"

    def populate(self, list[int] xs):
        """populate table from python list[int]"""
        if len(xs) <= self.size:
            for i, x in enumerate(xs):
                self.storage[0][i] = <long>x
        else:
            for i in range(self.size):
                self.storage[0][i] = <long>xs[i]

    def as_list(self):
        """converts table to python list of ints"""

        cdef long value
        cdef list[int] xs = []

        for i in range(self.size):
            value = self.storage[0][i]
            xs.append(<int>value)

        return xs

# ----------------------------------------------------------------------------
# Buffer

cdef class Buffer:
    """A wrapper class for a Max t_buffer_obj
    """
    cdef mp.t_buffer_obj *obj
    cdef mp.t_buffer_ref *ref
    # cdef mx.t_object *tobj # t_object with ref to buffer
    cdef bint is_locked
    cdef float* samples

    # def __cinit__(self, str name):
    #     self.x = <mx.t_object*>mx.sysmem_newptr(sizeof(mx.t_object))
    #     self.ref = mp.buffer_ref_new(self.x, str_to_sym(name))
    #     assert(mp.buffer_ref_exists(self.ref))
    #     self.obj = mp.buffer_ref_getobject(self.ref)
    #     self.samples = NULL
    #     self.is_locked = False

    # def __dealloc__(self):
    #     # De-allocate if not null
    #     if self.ref is not NULL:
    #         mx.object_free(self.ref)
    #         self.ref = NULL
    #     if self.x is not NULL:
    #         mx.sysmem_freeptr(self.x)
    #         self.x = NULL

    def __cinit__(self):
        self.obj = NULL
        self.ref = NULL
        self.samples = NULL
        self.is_locked = False

    def __dealloc__(self):
        # De-allocate if not null
        if self.ref is not NULL:
            mx.object_free(self.ref)
            self.ref = NULL

    @staticmethod
    cdef Buffer from_name(mx.t_object *x, str name):
        """Create a reference to a buffer~ object by name."""
        # Call to __new__ bypasses __init__ constructor
        cdef Buffer buffer = Buffer.__new__(Buffer)
        buffer.ref = mp.buffer_ref_new(x, str_to_sym(name))
        assert(mp.buffer_ref_exists(buffer.ref))
        buffer.obj = mp.buffer_ref_getobject(buffer.ref)
        return buffer

    def change_reference(self, str name):
        """Change a buffer reference to refer to a different buffer object by name."""
        mp.buffer_ref_set(self.ref, str_to_sym(name))
        assert(mp.buffer_ref_exists(self.ref))
        self.obj = mp.buffer_ref_getobject(self.ref)

    def view(self):
        """Open a viewer window to display the contents of the buffer."""
        mp.buffer_view(self.obj)

    @property
    def channelcount(self):
        """Get how many channels are present in the buffer content."""
        return mp.buffer_getchannelcount(self.obj)

    @property
    def framecount(self):
        """Get how many frames long the buffer content is in samples."""
        return mp.buffer_getframecount(self.obj)

    @property
    def samplerate(self):
        """Get the buffer's native sample rate in samples per second."""
        return mp.buffer_getsamplerate(self.obj)

    @property
    def millisamplerate(self):
        """Get the buffer's native sample rate in samples per millisecond."""
        return mp.buffer_getmillisamplerate(self.obj)

    @property
    def filename(self):
        """Retrieve the name of the last file to be read by a buffer~."""
        return sym_to_str(mp.buffer_getfilename(self.obj))

    def setdirty(self):
        """Set the buffer's dirty flag, indicating that changes have been made."""
        mp.buffer_setdirty(self.obj)

    def setpadding(self, long samplecount):
        """Set the number of samples with which to zero-pad the buffer~'s contents."""
        mp.buffer_setpadding(self.obj, samplecount)

    def locksamples(self):
        """Claim the buffer∼ and get a pointer to the first sample in memory."""
        self.samples = mp.buffer_locksamples(self.obj)
        self.is_locked = True

    def unlocksamples(self):
        """Release claim on buffer's contents so other objects can read/write to it."""
        mp.buffer_unlocksamples(self.obj)
        self.is_locked = False

# ----------------------------------------------------------------------------
# Dictionary

cdef class Dictionary:
    """A wrapper class for a Max t_dictionary
    """
    cdef mx.t_dictionary *d
    cdef dict type_map

    def __cinit__(self):
        # Create a new dictionary object.
        self.d = mx.dictionary_new()
        self.type_map = dict()

    # def __init__(self):
    #     self.type_map = dict()

    def __dealloc__(self):
        # De-allocate if not null
        if self.d is not NULL:
            mx.object_free(self.d)
            self.d = NULL

    def __setitem__(self, str key, value):
        if isinstance(value, float):
            self.type_map[key] = 'float'
            self.appendfloat(str_to_sym(key), <double>value)
        elif isinstance(value, int):
            self.type_map[key] = 'long'
            self.appendlong(str_to_sym(key), <long>value)
        elif isinstance(value, str):
            self.type_map[key] = 'str'
            self.appendsym(str_to_sym(key), str_to_sym(value))

    def __getitem__(self, str key):
        return {
            'float': self.get_float(key),
            'long': self.get_long(key),
            'str': self.get_str(key),
        }[key]

    cdef mx.t_max_err appendlong(self, mx.t_symbol* key, mx.t_atom_long value):
        """Add a long integer value to the dictionary."""
        return mx.dictionary_appendlong(self.d, key, value)

    cdef mx.t_max_err appendfloat(self, mx.t_symbol* key, double value):
        """Add a double-precision float value to the dictionary."""
        return mx.dictionary_appendfloat(self.d, key, value)

    cdef mx.t_max_err appendsym(self, mx.t_symbol* key, mx.t_symbol* value):
        """Add a t_symbol* value to the dictionary."""
        return mx.dictionary_appendsym(self.d, key, value)

    cdef mx.t_max_err appendatom(self, mx.t_symbol* key, mx.t_atom* value):
        """Add a t_atom* value to the dictionary."""
        return mx.dictionary_appendatom(self.d, key, value)

    cdef mx.t_max_err appendstring(self, mx.t_symbol* key, const char* value):
        """Add a c-string to the dictionary."""
        return mx.dictionary_appendstring(self.d, key, value)

    cdef mx.t_max_err appendatoms(self, mx.t_symbol* key, long argc, mx.t_atom* argv):
        """Add an array of atoms to the dictionary."""
        return mx.dictionary_appendatoms(self.d, key, argc, argv)

    cdef mx.t_max_err appendatomarray(self, mx.t_symbol* key, mx.t_object* value):
        """Add an Atom Array object to the dictionary."""
        return mx.dictionary_appendatomarray(self.d, key, value)

    cdef mx.t_max_err appenddictionary(self, mx.t_symbol* key, mx.t_object* value):
        """Add a dictionary object to the dictionary."""
        return mx.dictionary_appenddictionary(self.d, key, value)

    cdef mx.t_max_err appendobject(self, mx.t_symbol* key, mx.t_object* value):
        """Add an object to the dictionary."""
        return mx.dictionary_appendobject(self.d, key, value)

    # cdef mx.t_max_err appendobject_flags(self, mx.t_symbol* key, mx.t_object* value, long flags):
    #     return mx.dictionary_appendobject_flags(self.d, key, value, flags)

    # cdef mx.t_max_err appendbinbuf(self, mx.t_symbol* key, void* value):
    #     return mx.dictionary_appendbinbuf(self.d, key, value)

    cdef mx.t_max_err getlong(self, mx.t_symbol* key, mx.t_atom_long* value):
        """Retrieve a long integer from the dictionary."""
        return mx.dictionary_getlong(self.d, key, value)

    def get_long(self, str key) -> int:
        """Retrieve a long integer from the dictionary."""
        cdef mx.t_atom_long value
        cdef mx.t_max_err err = mx.dictionary_getlong(self.d, str_to_sym(key), &value)
        # cdef mx.t_max_err err = self.getlong(str_to_sym(key), &value)
        if err == mx.MAX_ERR_NONE:
            return <int>value

    cdef mx.t_max_err getfloat(self, mx.t_symbol* key, double* value):
        """Retrieve a double-precision float from the dictionary."""
        return mx.dictionary_getfloat(self.d, key, value)

    def get_float(self, str key) -> float:
        """Retrieve a double-precision float from the dictionary."""
        cdef double value
        cdef mx.t_max_err err = mx.dictionary_getfloat(self.d, str_to_sym(key), &value)
        if err == mx.MAX_ERR_NONE:
            return <float>value

    cdef mx.t_max_err getsym(self, mx.t_symbol* key, mx.t_symbol** value):
        """Retrieve a t_symbol* from the dictionary."""
        return mx.dictionary_getsym(self.d, key, value)

    def get_str(self, str key) -> str:
        """Retrieve a t_symbol* as a python string from the dictionary."""
        cdef mx.t_symbol* value
        cdef mx.t_max_err err = mx.dictionary_getsym(self.d, str_to_sym(key), &value)
        if err == mx.MAX_ERR_NONE:
            return sym_to_str(value)

    cdef mx.t_max_err getatom(self, mx.t_symbol* key, mx.t_atom* value):
        """Copy a t_atom from the dictionary."""
        return mx.dictionary_getatom(self.d, key, value)

    cdef mx.t_max_err getstring(self, mx.t_symbol* key, const char** value):
        """Retrieve a C-string pointer from the dictionary."""
        return mx.dictionary_getstring(self.d, key, value)

    cdef mx.t_max_err getatoms(self, mx.t_symbol* key, long* argc, mx.t_atom** argv):
        """Retrieve the address of a t_atom array of in the dictionary."""
        return mx.dictionary_getatoms(self.d, key, argc, argv)

    cdef mx.t_max_err getatoms_ext(self, mx.t_symbol* key, long stringstosymbols, long* argc, mx.t_atom** argv):
        """Retrieve the address of a t_atom array of in the dictionary."""
        return mx.dictionary_getatoms_ext(self.d, key, stringstosymbols, argc, argv)

    cdef mx.t_max_err copyatoms(self, mx.t_symbol* key, long* argc, mx.t_atom** argv):
        """Retrieve copies of a t_atom array in the dictionary."""
        return mx.dictionary_copyatoms(self.d, key, argc, argv)

    cdef mx.t_max_err getatomarray(self, mx.t_symbol* key, mx.t_object** value):
        """Retrieve a t_atomarray pointer from the dictionary."""
        return mx.dictionary_getatomarray(self.d, key, value)

    cdef mx.t_max_err getdictionary(self, mx.t_symbol* key, mx.t_object** value):
        """Retrieve a t_dictionary pointer from the dictionary."""
        return mx.dictionary_getdictionary(self.d, key, value)

    cdef mx.t_max_err get_ex(self, mx.t_symbol* key, long* ac, mx.t_atom** av, char* errstr):
        """Retrieve the address of a t_atom array of in the dictionary within nested dictionaries."""
        return mx.dictionary_get_ex(self.d, key, ac, av, errstr)

    cdef mx.t_max_err getobject(self, mx.t_symbol* key, mx.t_object** value):
        """Retrieve a t_object pointer from the dictionary."""
        return mx.dictionary_getobject(self.d, key, value)

    cdef bint has_string_value(self, mx.t_symbol* key):
        """Test a key to set if the data stored with that key contains a t_string object."""
        return mx.dictionary_entryisstring(self.d, key)

    cdef bint has_atomarray_value(self, mx.t_symbol* key):
        """Test a key to set if the data stored with that key contains a t_atomarray object."""
        return mx.dictionary_entryisatomarray(self.d, key)

    cdef bint has_dictionary_value(self, mx.t_symbol* key):
        """Test a key to set if the data stored with that key contains a t_dictionary object."""
        return mx.dictionary_entryisdictionary(self.d, key)

    cdef bint has_entry(self, mx.t_symbol* key):
        """Test a key to set if it exists in the dictionary."""
        return mx.dictionary_hasentry(self.d, key)

    def getentrycount(self) -> long:
        """Return the number of keys in a dictionary."""
        return mx.dictionary_getentrycount(self.d)

    cdef mx.t_max_err getkeys(self, long* numkeys, mx.t_symbol*** keys):
        """Retrieve all of the key names stored in a dictionary."""
        return mx.dictionary_getkeys(self.d, numkeys, keys)

    # cdef mx.t_max_err getkeys_ordered(self, long* numkeys, mx.t_symbol*** keys):
    #     return mx.dictionary_getkeys_ordered(self.d, numkeys, keys)

    cdef void freekeys(self, long numkeys, mx.t_symbol** keys):
        """Free memory allocated by the dictionary_getkeys() method."""
        mx.dictionary_freekeys(self.d, numkeys, keys)

    cdef mx.t_max_err deleteentry(self, mx.t_symbol* key):
        """Remove a value from the dictionary."""
        return mx.dictionary_deleteentry(self.d, key)

    cdef mx.t_max_err chuckentry(self, mx.t_symbol* key):
        """Remove a value from the dictionary without freeing it."""
        return mx.dictionary_chuckentry(self.d, key)

    cdef mx.t_max_err clear(self):
        """Delete all values from a dictionary."""
        return mx.dictionary_clear(self.d)

    cdef mx.t_dictionary* clone(self):
        return mx.dictionary_clone(self.d)

    cdef mx.t_max_err clone_to_existing(self, mx.t_dictionary* dc):
        return mx.dictionary_clone_to_existing(self.d, dc)

    # cdef mx.t_max_err copy_to_existing(self, mx.t_dictionary* dc):
    #     return mx.dictionary_copy_to_existing(self.d, dc)

    cdef mx.t_max_err merge_to_existing(self, mx.t_dictionary* dc):
        return mx.dictionary_merge_to_existing(self.d, dc)

    # cdef mx.t_max_err copy_nonunique_to_existing(self, mx.t_dictionary* dc):
    #     return mx.dictionary_copy_nonunique_to_existing(self.d, dc)

    cdef void funall(self, mx.method fun, void* arg):
        """Call the specified function for every entry in the dictionary."""
        mx.dictionary_funall(self.d, fun, arg)

    cdef mx.t_symbol* entry_getkey(self, mx.t_dictionary_entry* x):
        """Given a t_dictionary_entry*, return the key associated with that entry."""
        return mx.dictionary_entry_getkey(x)

    cdef void entry_getvalue(self, mx.t_dictionary_entry* x, mx.t_atom* value):
        """Given a t_dictionary_entry*, return the value associated with that entry."""
        mx.dictionary_entry_getvalue(x, value)

    cdef void entry_getvalues(self, mx.t_dictionary_entry* x, long* argc, mx.t_atom** argv):
        """Given a t_dictionary_entry*, return the values associated with that entry."""
        mx.dictionary_entry_getvalues(x, argc, argv)

    cdef mx.t_max_err copyunique(self, mx.t_dictionary* copyfrom):
        """Given 2 dictionaries, copy the keys unique to one of the dictionaries to the other dictionary."""
        return mx.dictionary_copyunique(self.d, copyfrom)

    cdef mx.t_max_err getdeflong(self, mx.t_symbol* key, mx.t_atom_long* value, mx.t_atom_long dfn):
        """Retrieve a long integer from the dictionary."""
        return mx.dictionary_getdeflong(self.d, key, value, dfn)

    cdef mx.t_max_err getdeffloat(self, mx.t_symbol* key, double* value, double dfn):
        """Retrieve a double-precision float from the dictionary."""
        return mx.dictionary_getdeffloat(self.d, key, value, dfn)

    cdef mx.t_max_err getdefsym(self, mx.t_symbol* key, mx.t_symbol** value, mx.t_symbol* dfn):
        """Retrieve a t_symbol* from the dictionary."""
        return mx.dictionary_getdefsym(self.d, key, value, dfn)

    cdef mx.t_max_err getdefatom(self, mx.t_symbol* key, mx.t_atom* value, mx.t_atom* dfn):
        """Retrieve a t_atom* from the dictionary."""
        return mx.dictionary_getdefatom(self.d, key, value, dfn)

    cdef mx.t_max_err getdefstring(self, mx.t_symbol* key, const char** value, char* dfn):
        """Retrieve a c-string from the dictionary."""
        return mx.dictionary_getdefstring(self.d, key, value, dfn)

    cdef mx.t_max_err getdefatoms(self, mx.t_symbol* key, long* argc, mx.t_atom** argv, mx.t_atom* dfn):
        """Retrieve the address of a t_atom array in the dictionary."""
        return mx.dictionary_getdefatoms(self.d, key, argc, argv, dfn)

    cdef mx.t_max_err copydefatoms(self, mx.t_symbol* key, long* argc, mx.t_atom** argv, mx.t_atom* dfn):
        """Retrieve copies of a t_atom array in the dictionary."""
        return mx.dictionary_copydefatoms(self.d, key, argc, argv, dfn)

    cdef mx.t_max_err dump(self, long recurse, long console):
        """Print the contents of a dictionary to the Max window."""
        return mx.dictionary_dump(self.d, recurse, console)

    cdef mx.t_max_err copyentries(self, mx.t_dictionary *dst, mx.t_symbol **keys):
        """Copy specified entries from one dictionary to another."""
        return mx.dictionary_copyentries(self.d, dst, keys)

    # cdef mx.t_dictionary* sprintf(self, char* fmt, ...):
    #   """Create a new dictionary populated with values using a combination of attribute and sprintf syntax."""
    #   return mx.dictionary_sprintf(char* fmt, ...)

    cdef long transaction_lock(self):
        """Take a lock on a dictionary.

        For preventing dictionary lock for transactions across multiple calls, or holding
        on to internal dictionary element pointers for complex operations.
        """
        return mx.dictionary_transaction_lock(self.d)

    cdef long transaction_unlock(self):
        """Release a lock on a dictionary.

        For preventing dictionary lock for transactions across multiple calls, or holding
        on to internal dictionary element pointers for complex operations.
        """
        return mx.dictionary_transaction_unlock(self.d)

    # FIXME: add staticmethod as well
    cdef mx.t_max_err read(self, const char* filename, short path, mx.t_dictionary** d):
        """Read the specified JSON file and return a t_dictionary object."""
        return mx.dictionary_read(filename, path, d)

    cdef mx.t_max_err write(self, const char* filename, short path):
        """Serialize the specified t_dictionary object to a JSON file."""
        return mx.dictionary_write(self.d, filename, path)

    # cdef mx.t_max_err read_yaml(self, const char* filename, const short path, mx.t_dictionary** d):
    #     """Read the specified JSON file and return a t_dictionary object."""
    #     return mx.dictionary_read_yaml(filename, path, d)

    # cdef mx.t_max_err dictionary_write_yaml(self, const char *filename, const short path):
    #     """Serialize the specified t_dictionary object to a YAML file."""
    #     return mx.dictionary_write_yaml(self.d, filename, path)

    cdef void post(self):
        """Print the contents of a dictionary to the Max window."""
        mx.postdictionary(<mx.t_object*>self.x)

    # ----------------------------------------------------------------------------
    # t_dictionary passing api

    cdef mx.t_dictionary *dictobj_register(self, mx.t_symbol **name):
        """Register a t_dictdictobj_registerionary with the dictionary passing system and map it to a unique name.

        If the t_symbol pointer has a NULL value then a unique name will be generated and filled-in
        upon return.
        """
        return mx.dictobj_register(self.d, name)

    cdef mx.t_max_err dictobj_unregister(self):
        """unregister dictionary (not required if dict is object-freed)"""
        return mx.dictobj_unregister(self.d)

    cdef mx.t_dictionary *dictobj_findregistered_clone(self, mx.t_symbol *name):
        """Find the t_dictionary for a given name, and return a copy of that dictionary.

        When you are done, do not call dictobj_release() on the dictionary, because you
        are working on a copy rather than on a retained pointer.
        """
        return mx.dictobj_findregistered_clone(name)

    cdef mx.t_dictionary *dictobj_findregistered_retain(self, mx.t_symbol *name):
        """Find the t_dictionary for a given name, return a pointer to that t_dictionary, and increment
        its reference count.

        When you are done you should call dictobj_release() on the dictionary.
        """
        return mx.dictobj_findregistered_retain(name)

    cdef mx.t_max_err dictobj_release(self):
        """For a t_dictionary/name previously retained with dictobj_findregistered_retain(),
        release it (decrement its reference count).
        """
        return mx.dictobj_release(self.d)

    cdef mx.t_symbol *dictobj_namefromptr(self):
        """Find the name associated with a given t_dictionary."""
        return mx.dictobj_namefromptr(self.d)

    cdef void dictobj_outlet_atoms(self, void *out, long argc, mx.t_atom *argv):
        """Send atoms to an outlet in your Max object, handling complex datatypes
        that may be present in those atoms.
        """
        mx.dictobj_outlet_atoms(out, argc, argv)

    cdef long dictobj_atom_safety(self, mx.t_atom *a):
        """Ensure that an atom is safe for passing.

        Atoms are allowed to be A_LONG, A_FLOAT, or A_SYM, but not A_OBJ. If the atom is an A_OBJ,
        it will be converted into something that will be safe to pass.
        """
        return mx.dictobj_atom_safety(a)

    # cdef long dictobj_atom_safety_flags(self, mx.t_atom *a, long flags):
    #     """Ensure that an atom is safe for passing.

    #     Atoms are allowed to be A_LONG, A_FLOAT, or A_SYM, but not A_OBJ. If the atom is an A_OBJ,
    #     it will be converted into something that will be safe to pass.

    #     Pass DICTOBJ_ATOM_FLAGS_REGISTER to flaga to have dictionary atoms registered/retained.
    #     """
    #     mx.dictobj_atom_safety_flags(a, flags)

    cdef long dictobj_validate(self, const mx.t_dictionary *schema, const mx.t_dictionary *candidate):
        """Validate the contents of a t_dictionary against a second t_dictionary containing a schema."""
        return mx.dictobj_validate(schema, candidate)

    cdef mx.t_max_err dictobj_jsonfromstring(self, long *jsonsize, char **json, const char *cstr):
        """Convert a C-string of Dictionary Syntax into a C-string of JSON."""
        return mx.dictobj_jsonfromstring(jsonsize, json, cstr)

    cdef mx.t_max_err dictobj_dictionaryfromstring(self, mx.t_dictionary **d, const char *cstr, int str_is_already_json, char *errorstring):
        """Create a new t_dictionary from Dictionary Syntax which is passed in as a C-string."""
        return mx.dictobj_dictionaryfromstring(d, cstr, str_is_already_json, errorstring)

    cdef mx.t_max_err dictobj_dictionaryfromatoms(self, mx.t_dictionary **d, const long argc, const mx.t_atom *argv):
        """Create a new t_dictionary from Dictionary Syntax which is passed in as an array of atoms."""
        return mx.dictobj_dictionaryfromatoms(d, argc, argv)

    # cdef t_max_err dictobj_dictionaryfromatoms_extended(t_dictionary **d, const t_symbol *msg, long argc, const t_atom *argv):
    #     """Create a new t_dictionary from from an array of atoms that use Max dictionary syntax, JSON, or compressed JSON."""

    cdef mx.t_max_err dictobj_dictionarytoatoms(self, long *argc, mx.t_atom **argv):
        """Serialize the contents of a t_dictionary into Dictionary Syntax."""
        return mx.dictobj_dictionarytoatoms(self.d, argc, argv)

    # cdef t_max_err dictobj_key_parse(t_object *x, t_dictionary *d, t_atom *akey, t_bool create, t_dictionary **targetdict, t_symbol **targetkey, t_int32 *index):
    #     """Given a complex key (one that includes potential heirarchy or array-member access), return the actual key and the dictionary in which the key should be referenced."""

# ----------------------------------------------------------------------------
# Database

cdef class Database:
    cdef mx.t_database *db
    cdef mx.t_symbol* db_name
    cdef bytes db_path

    def __cinit__(self, str db_name, str db_path):
        self.db_name = str_to_sym(db_name)
        self.db_path = db_path.encode('utf-8')
        mx.db_open(self.db_name, self.db_path, &self.db)

    def __dealloc__(self):
        mx.db_close(&self.db)

    def open(self):
        mx.db_open(self.db_name, self.db_path, &self.db)

    def close(self):
        mx.db_close(&self.db)

    def query_table_new(self, str tablename):
        mx.db_query_table_new(self.db, tablename.encode('utf-8'))

    def query_table_addcolumn(self, str tablename, str columnname, str columntype, str flags):
        mx.db_query_table_addcolumn(
            self.db,
            tablename.encode('utf-8'),
            columnname.encode('utf-8'),
            columntype.encode('utf-8'),
            flags.encode('utf-8')
        )

    def transaction_start(self):
        mx.db_transaction_start(self.db)

    def transaction_end(self):
        mx.db_transaction_end(self.db)

    def transaction_flush(self):
        mx.db_transaction_flush(self.db)

    cdef mx.t_max_err query_direct(self, mx.t_db_result **dbresult, const char *sql):
        return mx.db_query_direct(self.db, dbresult, sql)

    cdef mx.t_max_err query_getlastinsertid(self, long *idx):
        return mx.db_query_getlastinsertid(self.db, idx)

    def query_table_new(self, str tablename) -> int:
        return mx.db_query_table_new(self.db, tablename.encode('utf-8'))

    def query_table_addcolumn(self, str tablename, str columnname, str columntype, str flags) -> int:
        return mx.db_query_table_addcolumn(
            self.db,
            tablename.encode('utf-8'),
            columnname.encode('utf-8'),
            columntype.encode('utf-8'),
            flags.encode('utf-8')
        )

    cdef mx.t_max_err view_create(self, const char *sql, mx.t_db_view **dbview):
        return mx.db_view_create(self.db, sql, dbview)

    cdef mx.t_max_err view_remove(self, mx.t_db_view **dbview):
        return mx.db_view_remove(self.db, dbview)

    cdef mx.t_max_err view_getresult(self, mx.t_db_view *dbview, mx.t_db_result **result):
        return mx.db_view_getresult(dbview, result)

    cdef mx.t_max_err view_setquery(self, mx.t_db_view *dbview, char *newquery):
        return mx.db_view_setquery(dbview, newquery)

    cdef char** result_nextrecord(self, mx.t_db_result *result):
        return mx.db_result_nextrecord(result)

    cdef void result_reset(self, mx.t_db_result *result):
        mx.db_result_reset(result)

    cdef void result_clear(self, mx.t_db_result *result):
        mx.db_result_clear(result)

    cdef long result_numrecords(self, mx.t_db_result *result):
        return mx.db_result_numrecords(result)

    cdef long result_numfields(self, mx.t_db_result *result):
        return mx.db_result_numfields(result)

    cdef char* result_fieldname(self, mx.t_db_result *result, long fieldindex):
        return mx.db_result_fieldname(result, fieldindex)

    cdef char* result_string(self, mx.t_db_result *result, long recordindex, long fieldindex):
        return mx.db_result_string(result, recordindex, fieldindex)

    cdef long result_long(self, mx.t_db_result *result, long recordindex, long fieldindex):
        return mx.db_result_long(result, recordindex, fieldindex)

    cdef float result_float(self, mx.t_db_result *result, long recordindex, long fieldindex):
        return mx.db_result_float(result, recordindex, fieldindex)

    cdef mx.t_ptr_uint result_datetimeinseconds(self, mx.t_db_result *result, long recordindex, long fieldindex):
        return mx.db_result_datetimeinseconds(result, recordindex, fieldindex)

    cdef void util_stringtodate(self, const char *string, mx.t_ptr_uint *date):
        mx.db_util_stringtodate(string, date)

    cdef void util_datetostring(self, const mx.t_ptr_uint date, char *string):
        mx.db_util_datetostring(date, string)

    # cdef t_max_err db_query(t_database *db, t_db_result **dbresult, const char *sql, ...)
    # cdef t_max_err db_query_silent(t_database *db, t_db_result **dbresult, const char *sql, ...)

# ----------------------------------------------------------------------------
# Linklist

cdef class Linklist:
    cdef mx.t_linklist* lst

    def __cinit__(self):
        self.lst = <mx.t_linklist*>mx.linklist_new()

    def __dealloc__(self):
        mx.linklist_chuck(self.lst)  # will free list but contained objects
        #  or
        #  object_free(self.lst)  # will free all in list

    @property
    def size(self) -> int:
        return mx.linklist_getsize(self.lst)

    cdef void* getindex(self, long index):
        return mx.linklist_getindex(self.lst, index)

    cdef void chuck(self):
        mx.linklist_chuck(self.lst)

    cdef mx.t_atom_long getsize(self):
        return mx.linklist_getsize(self.lst)

    cdef mx.t_atom_long objptr2index(self, void* p):
        return mx.linklist_objptr2index(self.lst, p)

    cdef mx.t_atom_long append(self, void* o):
        return mx.linklist_append(self.lst, o)

    cdef mx.t_atom_long insertindex(self, void* o, long index):
        return mx.linklist_insertindex(self.lst, o, index)

    cdef mx.t_llelem* insertafterobjptr(self, void* o, void* objptr):
        return mx.linklist_insertafterobjptr(self.lst, o, objptr)

    cdef mx.t_llelem* insertbeforeobjptr(self, void* o, void* objptr):
        return mx.linklist_insertbeforeobjptr(self.lst, o, objptr)

    cdef mx.t_llelem* moveafterobjptr(self, void* o, void* objptr):
        return mx.linklist_moveafterobjptr(self.lst, o, objptr)

    cdef mx.t_llelem* movebeforeobjptr(self, void* o, void* objptr):
        return mx.linklist_movebeforeobjptr(self.lst, o, objptr)

    cdef mx.t_atom_long deleteindex(self, long index):
        return mx.linklist_deleteindex(self.lst, index)

    cdef long chuckindex(self, long index):
        return mx.linklist_chuckindex(self.lst, index)

    cdef long chuckobject(self, void* o):
        return mx.linklist_chuckobject(self.lst, o)

    cdef long deleteobject(self, void* o):
        return mx.linklist_deleteobject(self.lst, o)

    cdef long chuckptr(self, mx.t_llelem* p):
        return mx.linklist_chuckptr(self.lst, p)

    cdef void clear(self):
        mx.linklist_clear(self.lst)

    cdef mx.t_atom_long makearray(self, void** a, long max):
        return mx.linklist_makearray(self.lst, a, max)

    cdef void reverse(self, mx.t_linklist* x):
        mx.linklist_reverse(self.lst)

    cdef void rotate(self, long i):
        mx.linklist_rotate(self.lst, i)

    cdef void shuffle(self, mx.t_linklist* x):
        mx.linklist_shuffle(self.lst)

    cdef void swap(self, long a, long b):
        mx.linklist_swap(self.lst, a, b)

    cdef void methodall_imp(self, void* x, void* sym, void* p1, void* p2, void* p3, void* p4, void* p5, void* p6, void* p7, void* p8):
        mx.linklist_methodall_imp(x, sym, p1, p2, p3, p4, p5, p6, p7, p8)

    cdef void* methodindex_imp(self, void* x, void* i, void* s, void* p1, void* p2, void* p3, void* p4, void* p5, void* p6, void* p7):
        mx.linklist_methodindex_imp(x, i, s, p1, p2, p3, p4, p5, p6, p7)

    cdef mx.t_atom_long funall_break(self, mx.method fun, void* arg):
        return mx.linklist_funall_break(self.lst, fun, arg)

    cdef void* funindex(self, long i, mx.method fun, void* arg):
        return mx.linklist_funindex(self.lst, i, fun, arg)

    cdef void* substitute(self, void* p, void* newp):
        return mx.linklist_substitute(self.lst, p, newp)

    cdef void* next(self, void* p, void** next):
        return mx.linklist_next(self.lst, p, next)

    cdef void* prev(self, void* p, void** prev):
        return mx.linklist_prev(self.lst, p, prev)

    cdef void* last(self, void** item):
        return mx.linklist_last(self.lst, item)

    cdef void readonly(self, long readonly):
        mx.linklist_readonly(self.lst, readonly)

    cdef void flags(self, long flags):
        mx.linklist_flags(self.lst, flags)

    cdef mx.t_atom_long getflags(self, mx.t_linklist* x):
        return mx.linklist_getflags(self.lst)

    cdef long match(self, void* a, void* b):
        return mx.linklist_match(a, b)

    cdef void funall(self, mx.method fun, void* arg):
        mx.linklist_funall(self.lst, fun, arg)

    # cdef mx.t_llelem* index2ptr(self, long index):
    #     return mx.linklist_index2ptr(self.lst, index)

    # cdef long ptr2index(self, mx.t_llelem* p):
    #     return mx.linklist_ptr2index(self.lst, p)

    # cdef mx.t_llelem* insertptr(self, void* o, mx.t_llelem* p):
    #     return mx.linklist_insertptr(self.lst, o, p)

    # cdef long deleteptr(self, mx.t_llelem* p):
    #     return mx.linklist_deleteptr(self.lst, p)

    # cdef long insertnodeindex(self, mx.t_llelem* p, long index):
    #     return mx.linklist_insertnodeindex(self.lst, p, index)

    # cdef mx.t_llelem* insertnodeptr(self, mx.t_llelem* p1, mx.t_llelem* p2):
    #     return mx.linklist_insertnodeptr(self.lst, p1, p2)

    # cdef long appendnode(self, mx.t_llelem* p):
    #     return mx.linklist_appendnode(self.lst, p)

    # cdef void free(self, mx.t_llelem* elem):
    #     mx.linklistelem_free(self.lst, elem)

    # ERRORS

    # cdef t_llelem *linklistelem_new()
    # cdef long linklist_insert_sorted(t_linklist *x, void *o, long cmpfn(void *, void *))
    # cdef t_atom_long linklist_findfirst(t_linklist *x, void **o, long cmpfn(void *, void *), void *cmpdata)
    # cdef void linklist_findall(t_linklist *x, t_linklist **out, long cmpfn(void *, void *), void *cmpdata)
    # cdef void linklist_methodall(t_linklist *x, t_symbol *s, ...)
    # cdef void *linklist_methodindex(t_linklist *x, t_atom_long i, t_symbol *s, ...)
    # cdef void linklist_sort(t_linklist *x, long cmpfn(void *, void *))

# ----------------------------------------------------------------------------
# Binbuf

cdef class Binbuf:
    cdef void* buf

    def __cinit__(self):
        self.buf = mx.binbuf_new()

    def __dealloc__(self):
        mx.object_free(self.buf)

    cdef void append(self, mx.t_symbol *s, short argc, mx.t_atom *argv):
        mx.binbuf_append(self.buf, s, argc, argv)

    cdef void insert(self, mx.t_symbol *s, short argc, mx.t_atom *argv):
        mx.binbuf_insert(self.buf, s, argc, argv)

    # cdef void vinsert(self, char *fmt, ...):
    #     mx.binbuf_vinsert(self.buf, fmt, ...)

    cdef void * eval(self, short ac, mx.t_atom *av, void *to):
        return mx.binbuf_eval(self.buf, ac, av, to)

    cdef short getatom(self, long *p1, long *p2, mx.t_atom *ap):
        return mx.binbuf_getatom(self.buf, p1, p2, ap)

    cdef short text(self, char **src_text, long n):
        return mx.binbuf_text(self.buf, src_text, n)

    cdef short totext(self, char **dst_text, mx.t_ptr_size *sizep):
        return mx.binbuf_totext(self.buf, dst_text, sizep)

    cdef void set(self, mx.t_symbol *s, short argc, mx.t_atom *argv):
        mx.binbuf_set(self.buf, s, argc, argv)

    cdef void delete(self, long from_type, long to_type, long from_data, long to_data):
        mx.binbuf_delete(self.buf, from_type, to_type, from_data, to_data)

    cdef void addtext(self, char **text, long size):
        mx.binbuf_addtext(self.buf, text, size)

    cdef short readatom(self, char *outstr, char **text, long *n, long e, mx.t_atom *ap):
        mx.readatom(outstr, text, n, e, ap)

# ---------------------------------------------------------------------------
# Hashtab

cdef class Hashtab:
    cdef mx.t_hashtab* x

    def __cinit__(self, long slotcount):
        self.x = mx.hashtab_new(slotcount)

    def __dealloc__(self):
        mx.object_free(self.x)

    cdef mx.t_max_err store(self, mx.t_symbol* key, mx.t_object* val):
        return mx.hashtab_store(self.x, key, val)

    cdef mx.t_max_err storelong(self, mx.t_symbol* key, mx.t_atom_long val):
        return mx.hashtab_storelong(self.x, key, val)

    cdef mx.t_max_err storesym(self, mx.t_symbol* key, mx.t_symbol* val):
        return mx.hashtab_storesym(self.x, key, val)

    cdef mx.t_max_err store_safe(self, mx.t_symbol* key, mx.t_object* val):
        return mx.hashtab_store_safe(self.x, key, val)

    cdef mx.t_max_err storeflags(self, mx.t_symbol* key, mx.t_object* val, long flags):
        return mx.hashtab_storeflags(self.x, key, val, flags)

    cdef mx.t_max_err lookup(self, mx.t_symbol* key, mx.t_object** val):
        return mx.hashtab_lookup(self.x, key, val)

    cdef mx.t_max_err lookuplong(self, mx.t_symbol* key, mx.t_atom_long* val):
        return mx.hashtab_lookuplong(self.x, key, val)

    cdef mx.t_max_err lookupsym(self, mx.t_symbol* key, mx.t_symbol** val):
        return mx.hashtab_lookupsym(self.x, key, val)

    cdef mx.t_max_err lookupentry(self, mx.t_symbol* key, mx.t_hashtab_entry** entry):
        return mx.hashtab_lookupentry(self.x, key, entry)

    cdef mx.t_max_err lookupflags(self, mx.t_symbol* key, mx.t_object** val, long* flags):
        return mx.hashtab_lookupflags(self.x, key, val, flags)

    cdef mx.t_max_err delete(self, mx.t_symbol* key):
        return mx.hashtab_delete(self.x, key)

    cdef mx.t_max_err clear(self):
        return mx.hashtab_clear(self.x)

    cdef mx.t_max_err chuckkey(self, mx.t_symbol* key):
        return mx.hashtab_chuckkey(self.x, key)

    cdef mx.t_max_err chuck(self):
        return mx.hashtab_chuck(self.x)

    cdef mx.t_max_err methodall_imp(self, void* x, void* sym, void* p1, void* p2, void* p3, void* p4, void* p5, void* p6, void* p7, void* p8):
        return mx.hashtab_methodall_imp(self.x, sym, p1, p2, p3, p4, p5, p6, p7, p8)

    cdef mx.t_max_err funall(self, mx.method fun, void* arg):
        return mx.hashtab_funall(self.x, fun, arg)

    cdef mx.t_max_err objfunall(self, mx.method fun, void* arg):
        return mx.hashtab_objfunall(self.x, fun, arg)

    cdef mx.t_atom_long getsize(self):
        return mx.hashtab_getsize(self.x)

    cdef void print(self):
        mx.hashtab_print(self.x)

    cdef void readonly(self, long readonly):
        mx.hashtab_readonly(self.x, readonly)

    cdef void flags(self, long flags):
        mx.hashtab_flags(self.x, flags)

    cdef mx.t_atom_long getflags(self):
        return mx.hashtab_getflags(self.x)

    cdef mx.t_max_err keyflags(self, mx.t_symbol* key, long flags):
        return mx.hashtab_keyflags(self.x, key, flags)

    cdef mx.t_atom_long getkeyflags(self, mx.t_symbol* key):
        return mx.hashtab_getkeyflags(self.x, key)

    cdef mx.t_max_err getkeys(self, long* kc, mx.t_symbol*** kv):
        return mx.hashtab_getkeys(self.x, kc, kv)

    # ERRORS
    # cdef mx.t_hashtab_entry* entry_new(self, mx.t_symbol* key, mx.t_object* val):
    #      return mx.hashtab_entry_new(key, val)

    # cdef void entry_free(self, mx.t_hashtab_entry *x):
    #      mx.hashtab_entry_free(x)

    # cdef t_max_err hashtab_findfirst(t_hashtab *x, void **o, long cmpfn(void *, void *), void *cmpdata)

    # cdef t_max_err hashtab_methodall(t_hashtab *x, t_symbol *s, ...)

# ---------------------------------------------------------------------------
# Atom Array

cdef class AtomArray:
    cdef mx.t_atomarray *x
    cdef bint owner

    def __cinit__(self):
        self.x = NULL
        self.owner = False

    @staticmethod
    cdef AtomArray from_atom(mx.t_atom *av, int ac, bint owner=False):
        cdef AtomArray atom_array = AtomArray.__new__(AtomArray)
        atom_array.x = mx.atomarray_new(ac, av)
        atom_array.owner = owner
        return atom_array

    @staticmethod
    cdef AtomArray new(int size):
        cdef mx.t_atom *ptr = <mx.t_atom *>mx.sysmem_newptr(size * sizeof(mx.t_atom))
        if ptr is NULL:
            raise MemoryError
        return AtomArray.from_atom(ptr, size, owner=True)

    cdef void flags(self, long flags):
        mx.atomarray_flags(self.x, flags)

    cdef long getflags(self):
        return mx.atomarray_getflags(self.x)

    cdef mx.t_max_err setatoms(self, long ac, mx.t_atom* av):
        return mx.atomarray_setatoms(self.x, ac, av)

    cdef mx.t_max_err getatoms(self, long* ac, mx.t_atom** av):
        return mx.atomarray_getatoms(self.x, ac, av)

    cdef mx.t_max_err copyatoms(self, long* ac, mx.t_atom** av):
        return mx.atomarray_copyatoms(self.x, ac, av)

    cdef mx.t_atom_long getsize(self):
        return mx.atomarray_getsize(self.x)

    cdef mx.t_max_err getindex(self, long index, mx.t_atom* av):
        return mx.atomarray_getindex(self.x, index, av)

    # cdef mx.t_max_err setindex(self, long index, mx.t_atom* av):
    #     return mx.atomarray_setindex(self.x, index, av)

    cdef void* duplicate(self):
        return mx.atomarray_duplicate(self.x)

    cdef void* clone(self):
        return mx.atomarray_clone(self.x)

    cdef void appendatom(self, mx.t_atom* a):
        mx.atomarray_appendatom(self.x, a)

    cdef void appendatoms(self, long ac, mx.t_atom* av):
        mx.atomarray_appendatoms(self.x, ac, av)

    cdef void chuckindex(self, long index):
        mx.atomarray_chuckindex(self.x, index)

    cdef void clear(self):
        mx.atomarray_clear(self.x)

    cdef void funall(self, mx.method fun, void* arg):
        mx.atomarray_funall(self.x, fun, arg)

# ----------------------------------------------------------------------------
# PyExternal extension type

cdef class PyExternal:
    """
    Wraps the `py` external object and its methods.

    Should expose as much functionality as possible.
    """
    cdef px.t_py *obj
    cdef bytes name
    # cdef mp.t_buffer_ref *ref

    def __cinit__(self):
        """Retrieves the py object name and reference.

        PY_OBJ_NAME is set to __builtins__ at object creation
        making it available to all modules.

        Since all py objects are registered, knowing the name
        allows any module in the namespace to get a reference
        (as below) to its parent object.
        """
        PY_OBJ_NAME = getattr(__builtins__, 'PY_OBJ_NAME')
        self.name = PY_OBJ_NAME.encode('utf-8')
        self.obj = <px.t_py *>mx.object_findregistered(
            mx.CLASS_BOX, mx.gensym(self.name))

    # def get_buffer_ref(self, str s):
    #     self.ref = mp.buffer_ref_new(<mx.t_object *>self.obj, mx.gensym(s.encode('utf-8')))

    cpdef bang(self):
        px.py_bang(self.obj)

    def log(self, str s):
        px.py_log(self.obj, s.encode('utf-8'))

    def error(self, str s):
        px.py_error(self.obj, s.encode('utf-8'))

    cdef scan(self):
        px.py_scan(self.obj)

    cdef lookup(self, str name):
        cdef mx.t_hashtab* registry = px.py_get_global_registry()
        cdef mx.t_object* obj = NULL
        cdef mx.t_max_err err

        if (mx.hashtab_getsize(registry) == 0):
            self.error("registry not populated")
            return

        err = mx.hashtab_lookup(registry, str_to_sym(name), &obj)

        if ((err != mx.MAX_ERR_NONE) or (obj == NULL)):
            self.error("no object found with name")
        else:
            self.log("found object")

    def test_buffer(self, str name):
        buf = Buffer.from_name(<mx.t_object*>self.obj, name)
        buf.view()
        return buf.samplerate

    # UNTESTED
    cdef str atoms_to_pstring(self, long argc, mx.t_atom* argv):
        """atoms -> python string"""
        cdef long textsize = 0
        cdef char* text = NULL
        cdef mx.t_max_err _err = mx.atom_gettext(argc, argv, &textsize, &text,
            mx.OBEX_UTIL_ATOM_GETTEXT_DEFAULT)
        pstr = PyUnicode_FromString(text)
        mx.sysmem_freeptr(text)
        return pstr

    # UNTESTED
    cdef int pstring_to_atoms(self, str parsestr, long argc, mx.t_atom *argv) except -1:
        cdef char cparsestring[MAX_CHARS]
        cparsestring = PyUnicode_AsUTF8(parsestr)
        cdef mx.t_max_err err = mx.atom_setparse(&argc, &argv, cparsestring)
        if err != mx.MAX_ERR_NONE:  # test this!!
            raise Exception("cannot convert c parsestring to atom array")

    # UNTESTED
    cdef int cstring_to_atoms(self, char *parsestr, long argc, mx.t_atom *argv) except -1:
        cdef mx.t_max_err err = mx.atom_setparse(&argc, &argv, parsestr)
        if err != mx.MAX_ERR_NONE:  # test this!!
            raise Exception("cannot convert c parsestring to atom array")
        else:
            return 0

    cdef send(self, str name, list args):
        cdef long argc = <long>len(args) + 1
        cdef mx.t_atom argv[PY_MAX_ATOMS]
        _args = [name] + args

        if argc < 1:
            self.error("no arguments given")
            return

        if argc >= PY_MAX_ATOMS - 1:
            self.error("number of args exceeded app limit")
            return

        for i, elem in enumerate(_args):
            if type(elem) == float:
                mx.atom_setfloat(&argv[i], <double>elem)
            elif type(elem) == int:
                mx.atom_setlong((&argv[i]), <long>elem)
            elif type(elem) == str:
                mx.atom_setsym((&argv[i]), str_to_sym(elem))
            else:
                continue
        # mx.postatom(argv)
        px.py_send(self.obj, mx.gensym(""), argc, argv)

    # UNTESTED
    cdef mx.t_object * create(self, str classname, list args):
        """ implements void *newinstance(const t_symbol *s, short argc, const t_atom *argv)
        """
        atoms = Atom.from_seq(list(args))
        cdef mx.t_symbol * sym = <mx.t_symbol *>str_to_sym(classname)
        return <mx.t_object *>mx.newinstance(sym, <long>atoms.size, <mx.t_atom *>atoms.ptr)

    cdef bint table_exists(self, char* table_name):
        return px.py_table_exists(self.obj, table_name)

    cdef mx.t_max_err list_to_table(self, char* table_name, PyObject* plist):
        return px.py_list_to_table(self.obj, table_name, plist)

    cdef PyObject* table_to_list(self, char* table_name):
        return px.py_table_to_list(self.obj, table_name)

    cdef success_bang(self):
        px.py_bang_success(self.obj)

    cdef failure_bang(self):
        px.py_bang_failure(self.obj)

    cdef out_sym(self, str arg):
        mx.outlet_anything(<void*>px.get_outlet(self.obj), str_to_sym(arg), 0, NULL)

    cdef out_float(self, float arg):
        mx.outlet_float(<void*>px.get_outlet(self.obj), <double>arg)

    cdef out_int(self, int arg):
        mx.outlet_int(<void*>px.get_outlet(self.obj), <long>arg)

    cdef out_list(self, list arg):
        """note: not recursive...(yet) still cannot deal with list in list"""
        cdef long argc = <long>len(arg)
        cdef mx.t_atom argv[PY_MAX_ATOMS]

        if argc >= PY_MAX_ATOMS :
            self.error("number of args exceeded app limit")
            return

        for i, elem in enumerate(arg):
            if type(elem) == float:
                mx.atom_setfloat(&argv[i], <double>elem)
            elif type(elem) == int:
                mx.atom_setlong((&argv[i]), <long>elem)
            elif type(elem) == str:
                mx.atom_setsym((&argv[i]), str_to_sym(elem))
            else:
                continue

        mx.outlet_list(<void*>px.get_outlet(self.obj), mx.gensym("list"), argc, argv)

    cdef out_dict(self, dict arg):
        """note: not recursive...(yet) still cannot deal with dict in dict"""
        res = []
        for k, v in arg.items():
            res.append(k)
            res.append(':')
            if type(v) in [list, set, tuple]:
                for i in v:
                    res.append(i)
            else:
                res.append(v)
        self.out_list(res)

    cdef out(self, object arg):
        if isinstance(arg, float):
            self.out_float(arg)
        elif isinstance(arg, int):
            self.out_int(arg)
        elif isinstance(arg, str):
            self.out_sym(arg)
        elif isinstance(arg, list):
            self.out_list(arg)
        elif isinstance(arg, dict):
            self.out_dict(<dict>arg)
        else:
            return

    cdef mx.t_max_err method_binbuf(self, mx.t_symbol* s, void* buf, mx.t_atom* rv):
        return mx.object_method_binbuf(<mx.t_object*>self.obj, s, buf, rv)


# ----------------------------------------------------------------------------
# numpy c-api import example

if INCLUDE_NUMPY:

    # @cython.boundscheck(False)
    def zadd(in1, in2):
        cdef double complex[:] a = in1.ravel()
        cdef double complex[:] b = in2.ravel()

        out = np.empty(a.shape[0], np.complex64)
        cdef double complex[:] c = out.ravel()

        for i in range(c.shape[0]):
            c[i].real = a[i].real + b[i].real
            c[i].imag = a[i].imag + b[i].imag

        return out

# ----------------------------------------------------------------------------
# helper functions


def get_globals():
    return list(globals().keys())


def bang():
    ext = PyExternal()
    ext.bang()


def success_bang():
    ext = PyExternal()
    ext.success_bang()


def failure_bang():
    ext = PyExternal()
    ext.failure_bang()


def out_sym(s='hello outlet!'):
    ext = PyExternal()
    ext.out(s)


def out_int(n=100):
    ext = PyExternal()
    ext.out(n)


def out_float(n=12.75):
    ext = PyExternal()
    ext.out(n)


def out_list(xs=None):
    if not xs:
        xs = [1, 'a', 'c', 4, 5]
    ext = PyExternal()
    ext.out(xs)


def out_dict(**kwargs):
    if not kwargs:
        kwargs = {'a': [1, 2, 'a'], 'b': 1.3, 'c': 100, 'd': 'e'}
    ext = PyExternal()
    ext.out(kwargs)


def send(name='mrfloat', value=9.5):
    ext = PyExternal()
    ext.send(name, [value])


def lookup(name):
    ext = PyExternal()
    ext.lookup(name)


def post(str s):
    mx.post(s.encode('utf-8'))


def error(str s):
    mx.error(s.encode('utf-8'))

# ----------------------------------------------------------------------------
# test functions and variables


txt = "Hello MAX!"

greeting = 'Hello World'


def test_atom():
    ext = PyExternal()
    a1 = Atom.from_seq([1, 2.5, b'hello', 'world'])
    ext.out(a1.to_list())


def test_atom_create():
    atom = Atom.from_seq([440])
    atom.create_object('cycle~')


def test_dict():
    d = Dictionary()
    d['myfloat'] = 10.1
    d['myint'] = 3
    d['hello'] = 'world'
    return d.getentrycount()


def test_buffer(str name):
    ext = PyExternal()
    return ext.test_buffer(name)


cpdef public str hello():
    return greeting


def random(int n):
    import random
    return random.randint(0, n)


def echo(*args):
    return args


def total(*args):
    return sum(args)


# ----------------------------------------------------------------------------
# PyExternal extension type (obj pointer retrieved via uintptr_t

cdef class PyMxObject:
    cdef px.t_py *x

    def __cinit__(self):
        self.x = <px.t_py*>px.py_get_object_ref()

    cpdef bang(self):
        px.py_bang(self.x)

def test_ref():
    ext = PyMxObject()
    ext.bang()
