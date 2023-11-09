# api.pyx
""" This is a cython 'builtin' module which wraps parts of the Max/MSP c-api
for the `py` external.

- imports
- compile-time conditional imports
- compile-time constants
- python c-api imports
- helper cdef functions
- extension classes
    - [x] MaxObject
    - [x] Atom
    - [x] Table
    - [x] Buffer
    - [x] Dictionary
    - [x] Database
    - [ ] Linklist
    - [x] Binbuf
    - [x] Atombuf
    - [ ] Hashtab
    - [ ] AtomArray
    - [x] Patcher
    - [x] Box
    - [x] PyExternal
- helper functions


see: `py-js/source/projects/py/api.md` for further details
"""

# ----------------------------------------------------------------------------
# imports
from collections import namedtuple


from cython.view cimport array as cvarray
from cpython.ref cimport PyObject
from cpython cimport Py_buffer
from libc.stdint cimport uintptr_t
from libc.string cimport strcpy, strlen

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

    gensym(str s) -> t_symbol*
    """
    return mx.gensym(string.encode('utf-8'))


cdef str sym_to_str(mx.t_symbol* symbol):
    """converts a max symbol to a python string"""
    return symbol.s_name.decode()


cdef mx.t_symbol* bytes_to_sym(bytes string):
    """converts a python string to a t_symbol*"""
    return mx.gensym(string)


cdef bytes sym_to_bytes(mx.t_symbol* symbol):
    """converts a max symbol to a python string"""
    return <bytes>symbol.s_name

# ============================================================================
# Named Tuples

Rgb = namedtuple('Rgb', ['red', 'green', 'blue'])
Rgba = namedtuple('Rgba', ['red', 'green', 'blue', 'alpha'])
Rect = namedtuple('Rect', ['x', 'y', 'width', 'height'])

# ============================================================================
# EXTENSION TYPES





# ----------------------------------------------------------------------------
# api.MaxObject


cdef class MaxObject:
    """A wrapper for a Max t_object
    """
    cdef public classname
    cdef mx.t_object *ptr
    cdef bint ptr_owner

    def __cinit__(self):
        self.ptr = NULL
        self.ptr_owner = False

    def __dealloc__(self):
        # De-allocate if not null and flag is set
        if self.ptr is not NULL and self.ptr_owner is True:
            mx.object_free(self.ptr)
            self.ptr = NULL

    def __init__(self, classname: str, *args, namespace: str = "box"):
        cdef Atom atom = Atom(*args)
        self.ptr_owner = True
        self.ptr = <mx.t_object*>mx.object_new_typed(
            str_to_sym(namespace), str_to_sym(classname), atom.size, atom.ptr)

    @staticmethod
    def from_str(classname: str, parsestr: str, namespace: str = "box") -> MaxObject:
        """Create a new object with one or more atoms parsed from a C-string. 

        The object's new method must have an A_GIMME signature.
        """
        cdef MaxObject obj = MaxObject.__new__(MaxObject)
        obj.ptr_owner = True
        obj.ptr = <mx.t_object*>mx.object_new_parse(
            str_to_sym(namespace), str_to_sym(classname), parsestr.encode("utf8"))
        return obj

    @staticmethod
    cdef MaxObject from_ptr(mx.t_object *ptr, bint owner=False):
        # Call to __new__ bypasses __init__ constructor
        cdef MaxObject obj = MaxObject.__new__(MaxObject)
        obj.ptr = ptr
        obj.ptr_owner = owner
        return obj

    def method_exists(self, str name) -> bool:
        """checks if named method exists"""
        if mx.getfn(<mx.t_object *>self.ptr, str_to_sym(name)):
            return True
        return False

    def _method_noargs(self, str name):
        """object method call with no arguments"""
        cdef mx.t_max_err err = mx.object_method_typed(
            <mx.t_object *>self.ptr, str_to_sym(name), 0, NULL, NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def _method_args(self, str name, *args):
        """strongly typed object method call with arguments"""
        cdef Atom atom = Atom(*args)
        cdef mx.t_max_err err = mx.object_method_typed(
            <mx.t_object *>self.ptr, str_to_sym(name), atom.size, atom.ptr, NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def _method_parsestr(self, str name, str parsestr):
        """combines object_method_typed() + atom_setparse() to define method arguments."""
        cdef mx.t_max_err err = mx.object_method_parse(
            <mx.t_object *>self.ptr, str_to_sym(name), parsestr.encode('utf8'), NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def _method_float(self, str name, float number):
        """wrapper for object_method_typed() that passes a single float as an argument"""
        cdef mx.t_max_err err = mx.object_method_float(
            <mx.t_object *>self.ptr, str_to_sym(name), number, NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def _method_double(self, str name, double number):
        """wrapper for object_method_typed() that passes a single double as an argument"""
        cdef mx.t_max_err err = mx.object_method_double(
            <mx.t_object *>self.ptr, str_to_sym(name), number, NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def _method_long(self, str name, long number):
        """wrapper for object_method_typed() that passes a single long as an argument"""
        cdef mx.t_max_err err = mx.object_method_long(
            <mx.t_object *>self.ptr, str_to_sym(name), number, NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def _method_sym(self, str name, str symbol):
        """wrapper for object_method_typed() that passes a single t_symbol as an argument"""
        cdef mx.t_max_err err = mx.object_method_sym(
            <mx.t_object *>self.ptr, str_to_sym(name), str_to_sym(symbol), NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def call(self, str name, *args, parse=False):
        """general call object method function (strongly typed)"""
        if len(args) == 0:
            return self._method_noargs(name)
        elif len(args) == 1:
            if isinstance(args[0], str):
                if parse:
                    return self._method_parsestr(name, args[0])
                else:
                    return self._method_sym(name, args[0])
            elif isinstance(args[0], float):
                return self._method_double(name, args[0])
            elif isinstance(args[0], int):
                return self._method_long(name, args[0])
            elif isinstance(args[0], list):
                return self.call(name, *args[0])
            elif isinstance(args[0], tuple):
                return self.call(name, *args[0])
            elif isinstance(args[0], set):
                return self.call(name, *args[0])
        else:
            return self._method_args(name, *args)

    def get_attr_sym(self, str name) -> str:
        """Retrieves the value of an attribute, given its parent object and name."""
        cdef mx.t_symbol *attr_sym = <mx.t_symbol *>mx.object_attr_getsym(
             <mx.t_object *>self.ptr, str_to_sym(name))
        return sym_to_str(attr_sym)

    def set_attr_sym(self, str name, str value):
        """Sets the value of an attribute, given its parent object and name."""
        cdef mx.t_max_err err = mx.object_attr_setsym(<mx.t_object *>self.ptr, 
            str_to_sym(name), str_to_sym(value))
        if err != mx.MAX_ERR_NONE:
            return error(f"could not set attr '{name}' value '{value}'");

    def get_attr_long(self, str name) -> int:
        """Retrieves the value of an attribute, given its parent object and name."""
        return mx.object_attr_getlong(<mx.t_object *>self.ptr, str_to_sym(name))

    def set_attr_long(self, str name, int value):
        """Sets the value of an attribute, given its parent object and name."""
        cdef mx.t_max_err err = mx.object_attr_setlong(<mx.t_object *>self.ptr, 
            str_to_sym(name), value)
        if err != mx.MAX_ERR_NONE:
            return error(f"could not set attr '{name}' value '{value}'");

    def get_attr_float(self, str name) -> float:
        """Retrieves the value of an attribute, given its parent object and name."""
        return mx.object_attr_getfloat (<mx.t_object *>self.ptr, str_to_sym(name))

    def set_attr_float(self, str name, float value):
        """Sets the value of an attribute, given its parent object and name."""
        cdef mx.t_max_err err = mx.object_attr_setfloat(<mx.t_object *>self.ptr,
            str_to_sym(name), value)
        if err != mx.MAX_ERR_NONE:
            return error(f"could not set attr '{name}' value '{value}'");

    def get_attr_char(self, str name) -> bool:
        """Retrieves the value of an attribute, given its parent and name"""
        return mx.object_attr_getchar(<mx.t_object *>self.ptr, str_to_sym(name))

    def set_attr_char(self, str name, bint value):
        """Sets the value of an attribute, given its parent object and name."""
        cdef mx.t_max_err err = mx.object_attr_setchar(<mx.t_object *>self.ptr,
            str_to_sym(name), value)
        if err != mx.MAX_ERR_NONE:
            return error(f"could not set attr '{name}' value '{value}'");

    def set_attr_from_str(self, str name, str value):
        """Set an attribute value with one or more atoms parsed from a C-string."""
        cdef mx.t_max_err err = mx.object_attr_setparse(<mx.t_object *>self.ptr, 
            str_to_sym(name), value.encode('utf-8'))

    cdef mx.t_object * clone(self):
        """return clone of object"""
        return <mx.t_object *>mx.object_clone(<mx.t_object *>self.ptr)

# ----------------------------------------------------------------------------
# api.Atom

cdef class Atom:
    """A wrapper class for a Max t_atom
    """
    cdef mx.t_atom *ptr
    cdef bint ptr_owner
    cdef public long size

    def __cinit__(self):
        self.ptr_owner = False

    def __dealloc__(self):
        # De-allocate if not null and flag is set
        if self.ptr is not NULL and self.ptr_owner is True:
            mx.sysmem_freeptr(self.ptr)
            self.ptr = NULL

    def __init__(self, *args):
        if len(args)==1 and isinstance(args[0], list):
            args = args[0]
        self.size = len(args)
        self.ptr = <mx.t_atom *>mx.sysmem_newptr(self.size * sizeof(mx.t_atom))
        self.ptr_owner=True
        if self.ptr is NULL:
            raise MemoryError

        cdef int i
        for i, obj in enumerate(args):
            self[i] = obj

    def __iter__(self):
        return iter(self.to_list())

    def __len__(self):
        return self.size

    def __getitem__(self, int idx) -> object:
        if self.is_symbol(idx):
            return self.get_string(idx)

        elif self.is_float(idx):
            return self.get_float(idx)

        elif self.is_long(idx):
            return self.get_long(idx)

        else:
            raise TypeError

    def __setitem__(self, int idx, object value):
        if isinstance(value, str):
            self.set_symbol(idx, value)
        elif isinstance(value, float):
            self.set_float(idx, value)
        elif isinstance(value, int):
            self.set_long(idx, value)
        elif isinstance(value, bytes):
            self.set_bytes(idx, value)
        else:
            raise TypeError

    def set_float(self, int idx, float f):
        """Inserts a float into a t_atom and change its type to A_FLOAT."""
        mx.atom_setfloat(self.ptr + idx, f)

    def get_float(self, int idx=0) -> float:
        """Retrieves a float value from a t_atom."""
        return <float>mx.atom_getfloat(self.ptr + idx)

    def set_long(self, int idx, long x):
        """Inserts an integer into a t_atom and change its type to A_LONG."""
        mx.atom_setlong(self.ptr + idx, x)

    def get_long(self, int idx=0) -> long:
        """Retrieves a long integer value from a t_atom."""
        return <long>mx.atom_getlong(self.ptr + idx)

    def get_int(self, int idx=0) -> int:
        """Retrieves an integer value from a t_atom."""
        return <int>mx.atom_getlong(self.ptr + idx)

    def set_symbol(self, int idx, str symbol):
        """Inserts a t_symbol∗ into a t_atom and change its type to A_SYM."""
        mx.atom_setsym(self.ptr + idx, str_to_sym(symbol))

    cdef mx.t_symbol *get_symbol(self, int idx=0):
        """Retrieves a t_symbol ∗ value from a t_atom."""
        return mx.atom_getsym(self.ptr + idx)

    def get_string(self, int idx=0) -> str:
        """Retrieves a string value from a symbol t_atom."""
        return sym_to_str(self.get_symbol(idx))

    def set_bytes(self, int idx, bytes x):
        """Inserts an integer into a t_atom and change its type to A_LONG."""
        mx.atom_setsym(self.ptr + idx, bytes_to_sym(x))

    def get_bytes(self, int idx=0) -> bytes:
        """Retrieves a long integer value from a t_atom."""
        return sym_to_bytes(mx.atom_getsym(self.ptr + idx))

    cdef bint is_symbol(self, int idx=0):
        """check if index points to a symbol"""
        return (self.ptr + idx).a_type == mx.A_SYM

    cdef bint is_long(self, int idx=0):
        """check if index points to a long"""
        return (self.ptr + idx).a_type == mx.A_LONG

    cdef bint is_float(self, int idx=0):
        """check if index points to a float"""
        return (self.ptr + idx).a_type == mx.A_FLOAT

    def to_list(self) -> list:
        cdef int i
        _res = []
        for i in range(self.size):
            if self.is_symbol(i):
                _res.append(self.get_string(i))
            elif self.is_long(i):
                _res.append(self.get_int(i))
            elif self.is_float(i):
                _res.append(self.get_float(i))
        return _res

    def to_string(self) -> str:
        """Convert an array of atoms into a C-string.
        """

        cdef long textsize = 0
        cdef char *text = NULL
        cdef mx.t_max_err err
        cdef bytes result
        
        err = mx.atom_gettext(self.size, <mx.t_atom *>self.ptr,  &textsize, 
            &text, mx.OBEX_UTIL_ATOM_GETTEXT_DEFAULT)
        if (err == mx.MAX_ERR_NONE and textsize and text):
           result = <bytes>text

        if (text):
            mx.sysmem_freeptr(text)

        return result.decode()

    # if INCLUDE_NUMPY:
    #     def to_np_array(self):
    #         return np.array(self.to_list(), dtype=np.float64)

    def display(self):
        cdef int i
        for i in range(self.size):
            if self.is_float(i):
                print("is_float:", i)
            elif self.is_symbol(i):
                print("is_symbol:", i)
                s = self.get_string(i)
                print("string:", type(s))
            else:
                print("other:", i)

    @staticmethod
    cdef Atom from_ptr(mx.t_atom *ptr, long size, bint owner=False):
        # Call to __new__ bypasses __init__ constructor
        cdef Atom atom = Atom.__new__(Atom)
        atom.ptr = ptr
        atom.ptr_owner = owner
        atom.size = size
        return atom

    @staticmethod
    cdef Atom new(long size):
        """create an empty Atom instance with an aribitrary length"""
        cdef mx.t_atom *ptr = <mx.t_atom *>mx.sysmem_newptr(size * sizeof(mx.t_atom))
        if ptr is NULL:
            raise MemoryError
        return Atom.from_ptr(ptr, size, owner=True)

    @staticmethod
    def from_str(parsestr: str) -> Atom:
        """Parse a string into an Atom instance."""
        cdef mx.t_atom *ptr = NULL
        cdef long size = 0
        cdef mx.t_max_err err = mx.atom_setparse(&size, &ptr, parsestr.encode("utf8"))
        if err != mx.MAX_ERR_NONE:
            raise TypeError
        return Atom.from_ptr(ptr, size, owner=True)

    @staticmethod
    def from_seq(seq: object) -> Atom:
        # if not seq:
        #     return
        cdef long size = len(seq)
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
                error(f"cannot convert: {obj}")
                continue

        return Atom.from_ptr(ptr, size, owner=True)

    def is_string(self, int idx = 0) -> bool:
        """Determines whether or not an atom represents a t_string object."""
        return bool(mx.atomisstring(self.ptr + idx))

    def is_atomarray(self, int idx = 0) -> bool:
        """Determines whether or not an atom represents a t_atomarray object."""
        return bool(mx.atomisatomarray(self.ptr + idx))

    def is_dictionary(self, int idx = 0) -> bool:
        """Determines whether or not an atom represents a t_dictionary object."""
        return bool(mx.atomisdictionary(self.ptr + idx))

    cdef resize_ptr(self, mx.t_ptr_size new_size):
        """Resize an existing pointer."""
        self.ptr = <mx.t_atom *>mx.sysmem_resizeptr(<mx.t_atom *>self.ptr, 
            new_size * sizeof(mx.t_atom))

    cdef mx.t_max_err setchar_array(self, long ac, long count, unsigned char *vals):
        """Assign an array of char values to an array of atoms.
        """
        if ac > self.size:
            self.resize_ptr(ac)
        return mx.atom_setchar_array(ac, self.ptr, count, vals)

    cdef mx.t_max_err getchar_array(self, long count, unsigned char *vals):
        """Fetch an array of char values from an array of atoms.
        """
        return mx.atom_getchar_array(self.size, self.ptr, count, vals)

    cdef mx.t_max_err setlong_array(self, long ac, long count, mx.t_atom_long *vals):
        """Assign an array of long values to an array of atoms.
        """
        if ac > self.size:
            self.resize_ptr(ac)
        return mx.atom_setlong_array(ac, self.ptr, count, vals)

    cdef mx.t_max_err getlong_array(self, long count, mx.t_atom_long *vals):
        """Fetch an array of long values from an array of atoms.
        """
        return mx.atom_getlong_array(self.size, self.ptr, count, vals)

    cdef mx.t_max_err setfloat_array(self, long ac, long count, float *vals):
        """Assign an array of float values to an array of atoms.
        """
        if ac > self.size:
            self.resize_ptr(ac)
        return mx.atom_setfloat_array(ac, self.ptr, count, vals)

    cdef mx.t_max_err getfloat_array(self, long count, float *vals):
        """Fetch an array of float values from an array of atoms.
        """
        return mx.atom_getfloat_array(self.size, self.ptr, count, vals)

    cdef mx.t_max_err setdouble_array(self, long ac, long count, double *vals):
        """Assign an array of double values to an array of atoms.
        """
        if ac > self.size:
            self.resize_ptr(ac)
        return mx.atom_setdouble_array(ac, self.ptr, count, vals)

    cdef mx.t_max_err getdouble_array(self, long count, double *vals):
        """Fetch an array of double values from an array of atoms.
        """
        return mx.atom_getdouble_array(self.size, self.ptr, count, vals)

    cdef mx.t_max_err setsym_array(self, long ac, long count, mx.t_symbol **vals):
        """Assign an array of t_symbol values to an array of atoms.
        """
        if ac > self.size:
            self.resize_ptr(ac)
        return mx.atom_setsym_array(ac, self.ptr, count, vals)

    cdef mx.t_max_err getsym_array(self, long count, mx.t_symbol **vals):
        """Fetch an array of t_symbol values from an array of atoms.
        """
        return mx.atom_getsym_array(self.size, self.ptr, count, vals)

    cdef mx.t_max_err setatom_array(self, long ac, long count, mx.t_atom *vals):
        """Assign an array of t_atom values to an array of atoms.
        """
        if ac > self.size:
            self.resize_ptr(ac)
        return mx.atom_setatom_array(ac, self.ptr, count, vals)

    cdef mx.t_max_err getatom_array(self, long count, mx.t_atom *vals):
        """Fetch an array of t_symbol values from an array of atoms.
        """
        return mx.atom_getatom_array(self.size, self.ptr, count, vals)

    cdef mx.t_max_err setobj_array(self, long ac, long count, mx.t_object **vals):
        """Assign an array of t_object values to an array of atoms.
        """
        if ac > self.size:
            self.resize_ptr(ac)
        return mx.atom_setobj_array(ac, self.ptr, count, vals)

    cdef mx.t_max_err getobj_array(self, long count, mx.t_object **vals):
        """Fetch an array of t_object values from an array of atoms.
        """
        return mx.atom_getobj_array(self.size, self.ptr, count, vals)

    cdef mx.t_max_err setparse(self, const char *parsestr):
        """Parse a C-string into an array of atoms.
        """
        return mx.atom_setparse(&self.size, <mx.t_atom **>&self.ptr, parsestr)

    cdef mx.t_max_err setbinbuf(self, void *buf):
        """set binbuf content into an array of atoms.
        """
        return mx.atom_setbinbuf(&self.size, <mx.t_atom **>&self.ptr, buf)

    cdef mx.t_max_err setattrval(self, mx.t_symbol *attrname, mx.t_object *obj):
        """set attribute value into an array of atoms.
        """
        return mx.atom_setattrval(&self.size, <mx.t_atom **>&self.ptr, attrname, obj)

    cdef mx.t_max_err setobjval(self, mx.t_object *obj):
        """set object value into an array of atoms.
        """
        return mx.atom_setobjval(&self.size, <mx.t_atom **>&self.ptr, obj)


# ----------------------------------------------------------------------------
# api.Table

# TODO: ??? create new table using         
# self.obj = <mx.t_object*>mx.object_new_typed(
#       mx.CLASS_BOX, mx.gensym("table"), argc, argv)

cdef class Table:
    """A wrapper class to acess a pre-existing Max table
    """
    cdef public str name
    cdef long **storage
    cdef readonly long size

    def __cinit__(self):
        self.name = None
        self.storage = NULL
        self.size = 0

    def __init__(self, str name):
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
# api.Buffer

cdef class Buffer:
    """A wrapper class for a Max buffer~ object"""
    cdef public str name
    cdef mp.t_buffer_obj *obj
    cdef mp.t_buffer_ref *ref
    cdef bint is_locked
    cdef float* samples
    cdef float[:] s_buffer # cython memoryview
    cdef Py_ssize_t shape
    cdef Py_ssize_t strides

    def __cinit__(self):
        self.name = None
        self.obj = NULL
        self.ref = NULL
        self.samples = NULL
        self.is_locked = False

    def __dealloc__(self):
        # De-allocate if not null
        if self.ref is not NULL:
            mx.object_free(self.ref)
            self.ref = NULL

    def __init__(self, str name, str filename, int duration = -1, int channels = 1):
        """create a buffer from scratch given name and file to load

        params:
            name: str
            filename: str
            duration: int (ms) (default -1)
            channels: int      (default 1)
        """
        cdef mx.t_atom[4] argv
        cdef int argc = 4
        mx.atom_setsym(argv, str_to_sym(name))
        mx.atom_setsym(argv+1, str_to_sym(filename))        
        mx.atom_setlong(argv+2, duration)
        mx.atom_setlong(argv+3, channels)
        self.obj = <mx.t_object*>mx.object_new_typed(
            mx.CLASS_BOX, mx.gensym("buffer~"), argc, argv)
        self.ref = mp.buffer_ref_new(self.obj, str_to_sym(name))

    def __getbuffer__(self, Py_buffer *buffer, int flags):
        cdef Py_ssize_t itemsize = sizeof(float)
        cdef Py_ssize_t buffersize = <Py_ssize_t>mp.buffer_getframecount(self.obj)

        self.shape = buffersize
        self.strides = itemsize

        self.locksamples()

        buffer.buf = self.samples
        buffer.obj = self
        buffer.len = buffersize * itemsize   # product(shape) * itemsize
        buffer.itemsize = itemsize
        buffer.readonly = 0
        buffer.ndim = 1  
        buffer.format = "f"                     # float not double!
        buffer.shape = &self.shape
        buffer.strides = &self.strides
        buffer.suboffsets = NULL                # for pointer arrays only
        buffer.internal = NULL                  # see References

    def __releasebuffer__(self, Py_buffer *buffer):
        self.unlocksamples()

    @staticmethod
    cdef Buffer from_name(mx.t_object *x, str name):
        """Create a reference to a buffer~ object by name."""
        # Call to __new__ bypasses __init__ constructor
        cdef Buffer buffer = Buffer.__new__(Buffer)
        buffer.ref = mp.buffer_ref_new(x, str_to_sym(name))
        assert(mp.buffer_ref_exists(buffer.ref))
        buffer.name = name
        buffer.obj = mp.buffer_ref_getobject(buffer.ref)
        return buffer


    @staticmethod
    cdef Buffer new(mx.t_object *x, str name, str filename, int duration = -1, int channels = 1):
        """create a buffer from scratch given name and file to load

        params:
            name: str
            filename: str
            duration: int (ms) (default -1)
            channels: int      (default 1)
        """
        # create a buffer
        cdef mx.t_atom[4] argv
        cdef int argc = 4
        mx.atom_setsym(argv, str_to_sym(name))
        mx.atom_setsym(argv+1, str_to_sym(filename))        
        mx.atom_setlong(argv+2, duration)
        mx.atom_setlong(argv+3, channels)
        cdef mx.t_object *b = <mx.t_object*>mx.object_new_typed(
            mx.CLASS_BOX, mx.gensym("buffer~"), argc, argv)
        argc = 3
        mx.atom_setsym(argv, str_to_sym(filename))
        mx.atom_setlong(argv+1, 0) # starting time
        mx.atom_setlong(argv+2, channels)
        mx.typedmess(b, mx.gensym("replace"), argc, argv)

        # now retrieve buffer by name
        return Buffer.from_name(x, name)

    @staticmethod
    cdef Buffer empty(mx.t_object *x, str name, int duration_ms, int channels=1):
        """create a new buffer

        params:
            name: str
            duration: int (ms)
            channels: int      (default 1)
        """
        # create a buffer
        cdef mx.t_atom argv[3];

        # check if another buffer exists with same name
        cdef mp.t_buffer_ref* ref = mp.buffer_ref_new(x, str_to_sym(name))
        # TODO: rethink (overly safe?)
        # if mp.buffer_ref_exists(ref):
        #     return error(f"buffer with name {name} already exists, and was overwritten")

        mx.atom_setsym(argv + 0, str_to_sym(name))
        mx.atom_setlong(argv + 1, duration_ms);
        mx.atom_setlong(argv + 2, channels);
        cdef mx.t_object *b = <mx.t_object*>mx.object_new_typed(
            mx.CLASS_BOX, mx.gensym("buffer~"), 3, argv)

        # now retrieve buffer by name
        return Buffer.from_name(x, name)

    def refresh(self):
        """reset buffer object from ref"""
        self.obj = mp.buffer_ref_getobject(self.ref)

    def change(self, str msg, *args) -> bool:
        """generic structural change method

        May only be used for structural changes to the buffer.

        >>> buf.change("sizeinsamps", 20000)
        """
        cdef Atom atom = Atom.from_seq(args)

        if (self.obj):
            # mp.buffer_edit_begin(self.obj)
            self.buffer_edit_begin()
            mx.object_method_typed(
                <mx.t_object*>self.obj, str_to_sym(msg), atom.size, atom.ptr, NULL)
            # mp.buffer_edit_end(self.obj, 1)
            self.buffer_edit_end()
            self.setdirty()
            return True

        return False

    def framecount(self):
        """Get how many frames long the buffer content is in samples."""
        return mp.buffer_getframecount(self.obj)

    def set_framecount(self, int frames):
        """resize buffer by number of samples (frames)"""
 
        if frames == self.framecount:
            return

        if self.change("sizeinsamps", frames):
            if frames != self.framecount:
                error(f"Could not resize {self.name} "
                      f"frames: {frames} != {self.framecount}")
            assert(frames == self.framecount)
            return

        return error("Resize on null buffer")

    framecount = property(framecount, set_framecount)


    def samplerate(self):
        """Get the buffer's native sample rate in samples per second."""
        return mp.buffer_getsamplerate(self.obj)

    def set_samplerate(self, int samplerate):
        """change buffer samplerate (Hz)"""
        if samplerate == self.samplerate:
            return

        if self.change("sr", samplerate):
            if samplerate != self.samplerate:
                error(f"Could not set samplerate to buffer {self.name} "
                      f"{samplerate} != {self.samplerate}")
            assert(samplerate == self.samplerate)
            return

        return error("Resize on null buffer")

    samplerate = property(samplerate, set_samplerate)


    def duration(self) -> float:
        """Get the buffer's duration in seconds."""
        return self.n_samples / self.samplerate

    def set_duration(self, float duration):
        """resize buffer duration (seconds)"""
        self.set_duration_ms(1000 * duration)

    duration = property(duration, set_duration)


    def duration_ms(self):
        """Get the buffer's duration in milliseconds."""
        return self.duration * 1000

    def set_duration_ms(self, int duration):
        """resize buffer duration (milliseconds)"""

        if duration == int(self.duration_ms):
            return

        if self.change("size", duration):
            return

        return error("Resize on null buffer")

    duration_ms = property(duration_ms, set_duration_ms)


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
    def millisamplerate(self):
        """Get the buffer's native sample rate in samples per millisecond."""
        return mp.buffer_getmillisamplerate(self.obj)

    @property
    def n_samples(self):
        """Get the number of samples in the buffer."""
        return self.framecount

    # @property
    def filename(self) -> str:
        """Retrieve the name of the last file to be read by a buffer~."""
        return sym_to_str(mp.buffer_getfilename(self.obj))

    def set_filename(self, str filename):
        """set filename, uses 'replace <filename>' to set buffer length, contents from file"""
        self.replace(filename)

    filename = property(filename, set_filename)

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
        if self.samples:
            self.samples = NULL
            self.is_locked = False

    def buffer_edit_begin(self):
        """begin buffer_edit block

        Use `buffer_edit` functions to collapse all operations of locking heavy `b_mutex`,
        setting b_valid flag, waiting on lightweight atomic b_inuse, etc.
        """
        mp.buffer_edit_begin(self.obj)

    def buffer_edit_end(self, int valid=1):
        """end buffer_edit block"""
        mp.buffer_edit_end(self.obj, valid)

    # TODO: add start:end slice
    def get_samples(self):
        """get samples as a memoryview"""
        cdef int i;
        self.s_buffer = cvarray(
            shape=(self.n_samples,), itemsize=sizeof(float), format="f")
        self.locksamples()
        for i in range(self.n_samples):
            self.s_buffer[i] = self.samples[i]
        self.unlocksamples()
        cdef float[:] res = self.s_buffer
        return res

    # float32 is the default in Max/MSP
    def set_samples(self, float[:] samples):
        """set samples from a memoryview"""
        # assert samples.shape[0] <= self.n_samples
        cdef long n_samples = <Py_ssize_t>samples.shape[0]
        cdef int i
        # resize buffer to samples.shape[0]
        self.set_framecount(n_samples)
        self.locksamples()
        for i in range(n_samples):
            self.samples[i] = samples[i]
        self.unlocksamples()

    def send(self, str msg, *args):
        """generic message sender

        Used for all message methods except those modifying
        the buffer structure (size, framecount, ..)
        i.e. not for structural changes which require
        re-allocation of memory.

        >>> buf.send("fill", "sin", 24)
        """
        if not args:
            self._send_single(msg)
        else:
            self._send_multi(msg, *args)

    def _send_single(self, str msg):
        """generic message sender for a one word message
        """
        if (self.obj):
            mx.object_method_typed(
                <mx.t_object*>self.obj, str_to_sym(msg), 0, NULL, NULL)

    def _send_multi(self, str msg, *args):
        """generic message sender for a msg with a list of arguments

        >>> buf.send("fill", "sin", 24)
        """
        cdef Atom atom = Atom.from_seq(args)
        if (self.obj):
            self.locksamples()
            mx.object_method_typed(
                <mx.t_object*>self.obj, str_to_sym(msg), atom.size, atom.ptr, NULL)
            self.unlocksamples()
            self.setdirty()

    def bang(self):
        """redraw buffer display"""
        self.send("bang")

    def apply(self, *args):
        """apply a function to buffer contents

        funcs:
            triangle, hamming, hanning, blackman, welch, (kaiser beta) (windowing)
            gain
            offset
            getdelta
        """
        self.send("apply", *args)

    def clear(self):
        """erase contents of buffer"""
        self.send("clear")

    def clearlow(self):
        """erase contents of buffer via a low priority thread"""
        self.send("clearlow")

    def crop(self, int start, int end):
        """trim audio data in buffer and resize it accordingly"""
        self.change("crop", start, end)

    def duplicate(self, str name):
        """import contents of named buffer"""
        self.change("duplicate", name)

    def enumerate(self):
        """lists (on console) all objects referencing buffer"""
        self.send("enumerate")

    def fill(self, *args):
        """generic fill method

        >>> buf.fill("sin", 24)
        """
        self.send("fill", *args)

    def import_(self, path: str, start: int = 0, duration: int = -1, channels: int = 0):
        """file import

        params:
            filename: str
            start: float (ms)
            duration: float (ms) (negative resizes buffer)
            channels
        """
        if channels:
            self.change("import", path, start, duration, channels)
        else:
            self.change("import", path, start, duration)

    def importreplace(self, path: str, start: int = 0, channels: int = 0):
        """same as import but imports are performed with automatic duration and channel resizing enabled by default
        """
        if channels:
            self.change("importreplace", path, start, channels)
        else:
            self.change("importreplace", path, start)

    def rename(self, str name):
        """combine 'name' and 'set' in one method

        renames buffer to new <name> and tells other objects to refer to it by new name
        """
        self.send("set", name)
        self.send("name", name)

    def normalize(self, float amount):
        """normalize audio in buffer"""
        self.send("normalize", amount)

    def open(self):
        """open sample display"""
        self.send("open")

    def printmodtime(self):
        """posts last modification time to console"""
        self.send("printmodtime")

    read = import_ # read is a synonym for import

    replace = importreplace # replace is a synonym for importreplace

    def close(self):
        """close view window"""
        self.send("wclose")

    def write(self, str path):
        """write contents of buffer to audio file"""
        if path.endswith(".wav"):
            self.send("writewave", path)
        elif path.endswith(".aiff"):
            self.send("writeaiff", path)
        elif path.endswith(".raw"):
            self.send("writeraw", path)
        elif path.endswith(".flac"):
            self.send("writeflac", path)


# ----------------------------------------------------------------------------
# api.Dictionary

# TODO: dict to api.Dict conversion

cdef class Dictionary:
    """A wrapper class for a Max t_dictionary
    """
    cdef mx.t_dictionary *d
    cdef dict type_map

    def __cinit__(self):
        self.d = NULL
        self.type_map = None

    def __init__(self):
        self.d = mx.dictionary_new()
        self.type_map = dict()

    def __dealloc__(self):
        # De-allocate if not null
        if self.d is not NULL:
            mx.object_free(self.d)
            self.d = NULL

    def __setitem__(self, str key, object value):
        if isinstance(value, float):
            self.type_map[key] = 'float'
            self.set_float(key, <double>value)
        elif isinstance(value, int):
            self.type_map[key] = 'long'
            self.set_long(key, <int>value)
        elif isinstance(value, str):
            self.type_map[key] = 'str'
            self.set_sym(key, value)
        elif isinstance(value, list):
            self.type_map[key] = 'list'
            self.set_atoms(key, value)
        elif isinstance(value, bytes):
            self.type_map[key] = 'bytes'
            self.set_bytes(key, value)
        else:
            raise TypeError

    def __getitem__(self, str key):
        return {
            'float': self.get_float,
            'long': self.get_long,
            'str': self.get_sym,
            'bytes': self.get_bytes,
            'list': self.get_atoms,
        }[self.type_map[key]](key)

    def set_long(self, str key, int value):
        """Add a long integer value to the dictionary."""
        return mx.dictionary_appendlong(self.d, str_to_sym(key), value)

    def set_float(self, str key, double value):
        """Add a double-precision float value to the dictionary."""
        return mx.dictionary_appendfloat(self.d, str_to_sym(key), value)

    def set_sym(self, str key, str value):
        """Add a t_symbol* value to the dictionary."""
        return mx.dictionary_appendsym(self.d, str_to_sym(key), str_to_sym(value))

    def set_atom(self, str key, object obj):
        """Add a t_atom* value to the dictionary."""
        cdef Atom atom = Atom(obj)
        return mx.dictionary_appendatom(self.d, str_to_sym(key), atom.ptr)

    def set_atoms(self, str key, *args):
        """Add an array of atoms to the dictionary."""
        cdef Atom atom = Atom(*args)
        return mx.dictionary_appendatoms(self.d, str_to_sym(key), atom.size, atom.ptr)

    def set_bytes(self, str key, bytes value):
        """Add a c-string to the dictionary."""
        return mx.dictionary_appendstring(self.d, str_to_sym(key), value)

    cdef mx.t_max_err appendatomarray(self, mx.t_symbol* key, mx.t_object* value):
        """Add an Atom Array object to the dictionary."""
        return mx.dictionary_appendatomarray(self.d, key, value)

    cdef mx.t_max_err appenddictionary(self, mx.t_symbol* key, mx.t_object* value):
        """Add a dictionary object to the dictionary."""
        return mx.dictionary_appenddictionary(self.d, key, value)

    cdef mx.t_max_err appendobject(self, mx.t_symbol* key, mx.t_object* value):
        """Add an object to the dictionary."""
        return mx.dictionary_appendobject(self.d, key, value)

    cdef mx.t_max_err appendobject_flags(self, mx.t_symbol* key, mx.t_object* value, long flags):
        return mx.dictionary_appendobject_flags(self.d, key, value, flags)

    cdef mx.t_max_err appendbinbuf(self, mx.t_symbol* key, void* value):
        return mx.dictionary_appendbinbuf(self.d, key, value)

    def get_long(self, str key) -> int:
        """Retrieve a long integer from the dictionary."""
        cdef mx.t_atom_long value
        cdef mx.t_max_err err = mx.dictionary_getlong(self.d, str_to_sym(key), &value)
        if err == mx.MAX_ERR_NONE:
            return <int>value
        return error(f"could not get long value from dict with key {key}")

    def get_float(self, str key) -> float:
        """Retrieve a double-precision float from the dictionary."""
        cdef double value
        cdef mx.t_max_err err = mx.dictionary_getfloat(self.d, str_to_sym(key), &value)
        if err == mx.MAX_ERR_NONE:
            return <float>value
        return error(f"could not get float value from dict with key {key}")

    def get_sym(self, str key) -> str:
        """Retrieve a t_symbol* as a python string from the dictionary."""
        cdef mx.t_symbol* value
        cdef mx.t_max_err err = mx.dictionary_getsym(self.d, str_to_sym(key), &value)
        if err == mx.MAX_ERR_NONE:
            return sym_to_str(value)
        return error(f"could not get symbol as str from dict with key {key}")

    def get_bytes(self, str key) -> bytes:
        """Retrieve a bytes object from the dictionary."""
        cdef const char* value
        cdef mx.t_max_err err = mx.dictionary_getstring(self.d, str_to_sym(key), &value)
        if err == mx.MAX_ERR_NONE:
            return <bytes>value
        return error(f"could not get bytes from dict with key {key}")

    def get_atoms(self, str key) -> list:
        """Retrieve the address of a t_atom array of in the dictionary."""
        cdef long argc
        cdef mx.t_atom* argv
        cdef Atom atom
        cdef mx.t_max_err err = mx.dictionary_getatoms(self.d, str_to_sym(key), &argc, &argv)
        if err == mx.MAX_ERR_NONE:
            atom = Atom.from_ptr(argv, argc)
            return atom.to_list()
        return error(f"could not get atoms from dict with key {key}")

    # cdef mx.t_max_err getstring(self, mx.t_symbol* key, const char** value):
    #     """Retrieve a C-string pointer from the dictionary."""
    #     return mx.dictionary_getstring(self.d, key, value)

    cdef mx.t_max_err getatom(self, mx.t_symbol* key, mx.t_atom* value):
        """Copy a t_atom from the dictionary."""
        return mx.dictionary_getatom(self.d, key, value)

    # cdef mx.t_max_err getatoms(self, mx.t_symbol* key, long* argc, mx.t_atom** argv):
    #     """Retrieve the address of a t_atom array of in the dictionary."""
    #     return mx.dictionary_getatoms(self.d, key, argc, argv)

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

    def has_string_value(self, str key) -> bool:
        """Test a key to set if the data stored with that key contains a t_string object."""
        return mx.dictionary_entryisstring(self.d, str_to_sym(key))

    def has_atomarray_value(self, str key) -> bool:
        """Test a key to set if the data stored with that key contains a t_atomarray object."""
        return mx.dictionary_entryisatomarray(self.d, str_to_sym(key))

    def has_dictionary_value(self, str key) -> bool:
        """Test a key to set if the data stored with that key contains a t_dictionary object."""
        return mx.dictionary_entryisdictionary(self.d, str_to_sym(key))

    def has_entry(self, str key) -> bool:
        """Test a key to set if it exists in the dictionary."""
        return mx.dictionary_hasentry(self.d, str_to_sym(key))

    def getentrycount(self) -> long:
        """Return the number of keys in a dictionary."""
        return mx.dictionary_getentrycount(self.d)

    cdef mx.t_max_err getkeys(self, long* numkeys, mx.t_symbol*** keys):
        """Retrieve all of the key names stored in a dictionary."""
        return mx.dictionary_getkeys(self.d, numkeys, keys)

    cdef mx.t_max_err getkeys_ordered(self, long* numkeys, mx.t_symbol*** keys):
        return mx.dictionary_getkeys_ordered(self.d, numkeys, keys)

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

    # MAX_SDK BUG
    # cdef mx.t_max_err copy_to_existing(self, mx.t_dictionary* dc):
    #     return mx.dictionary_copy_to_existing(self.d, dc)

    cdef mx.t_max_err merge_to_existing(self, mx.t_dictionary* dc):
        return mx.dictionary_merge_to_existing(self.d, dc)

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
# api.Database

cdef class Database:
    cdef mx.t_database *db
    cdef mx.t_symbol* db_name
    cdef bytes db_path

    # def __cinit__(self, str db_name, str db_path):
    #     self.db_name = str_to_sym(db_name)
    #     self.db_path = db_path.encode('utf-8')
    #     mx.db_open(self.db_name, self.db_path, &self.db)

    def __cinit__(self):
        self.db = NULL
        self.db_name = NULL
        self.db_path = None

    def __init__(self, str db_name, str db_path):
        self.db_name = str_to_sym(db_name)
        self.db_path = db_path.encode('utf-8')
        # mx.db_open(self.db_name, self.db_path, &self.db)

    def __dealloc__(self):
        mx.db_close(&self.db)
        self.db = NULL
        self.db_name = NULL
        self.db_path = None

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
# api.Linklist

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

# ----------------------------------------------------------------------------
# api.Binbuf

cdef class Binbuf:
    cdef mx.t_binbuf* ptr

    def __cinit__(self):
        self.ptr = <mx.t_binbuf*>mx.binbuf_new()
        if not self.ptr:
            raise MemoryError

    def __dealloc__(self):
        mx.object_free(self.ptr)

    def append(self, str receiver, *args):
        """Append t_atoms to a Binbuf without modifying them."""
        _args = [receiver] + list(args)
        cdef Atom atom = Atom(*_args)
        mx.binbuf_append(self.ptr, NULL, atom.size, atom.ptr)

    def insert(self, *args):
        """Used if you want to saving your object into a Binbuf.

        It assumes the message is part of a file that will later be evaluated, such as a 
        Patcher file. The first argument argv[0] will be the receiver of the 
        message and must be a Symbol.
        """
        cdef Atom atom = Atom(*args)
        assert isinstance(args[0], str), f"binbuf.insert error: first argument as receiver must be a symbol"
        mx.binbuf_insert(self.ptr, NULL, atom.size, atom.ptr)

    # cdef void vinsert(self, char *fmt, ...):
    #     mx.binbuf_vinsert(self.ptr, fmt, ...)

    def eval(self):
        # mx.critical_enter(<mx.t_critical>0)
        self.eval_msg_to(0, NULL, NULL)
        # mx.critical_exit(<mx.t_critical>0)

    cdef void * eval_msg_to(self, short ac, mx.t_atom *av, void *to):
        """evaluate a Max message in a Binbuf, passing it arguments.

        Evaluates the message in a Binbuf with arguments in argv, and sends
        it to receiver.
        """
        return mx.binbuf_eval(self.ptr, ac, av, to)

    def add_text(self, str text):
        """Used binbuf_text() to convert a text handle to a Binbuf.

        binbuf_text() parses the text and converts it into binary format. 
        Use it to evaluate a text file or text line entry into a  Binbuf.
        """
        cdef char* src_text = <char *>mx.sysmem_newptr((len(text)+1) * sizeof(char))
        cdef long n = len(text) + 1
        cdef short err = 0

        strcpy(src_text, text.encode('utf8'))
        err = mx.binbuf_text(self.ptr, &src_text, n)
        mx.sysmem_freeptr(src_text)
        if err:
            return error("binbuf.add_text failed")

    def eval_to_clipboard(self, str text):
        """evaluate a Max message in a Binbuf and sends it the Max clipboard
        
        Thanks to @11OLSEN for the solution, see:
        https://cycling74.com/forums/on-the-current-utility-of-binbufs-and-atombufs
        """ 
        cdef mx.t_object *max_obj = NULL
        cdef mx.t_object* clipboard = <mx.t_object*>mx.object_new(
            mx.gensym("nobox"), mx.gensym("clipboard"))
        self.add_text(text)
        mx.object_obex_lookup(self.ptr, mx.gensym("max"), &max_obj)
        mx.object_method(clipboard, mx.gensym("frombinbuf"), self.ptr)
        mx.object_method(max_obj, mx.gensym("newfromclipboard"))
        mx.object_free(clipboard)

    def to_text(self) -> str:
        """Convert a Binbuf into a text handle.

        Backslashes are added to protect literal commas and semicolons 
        contained in symbols. The pseudo-types are converted into commas, 
        semicolons, or dollar-sign and number, without backslashes preceding 
        them. binbuf_text can read the output of binbuf_totext and
        make the same Binbuf.
        """
        cdef mx.t_handle contents = mx.sysmem_newhandle(0)
        cdef mx.t_ptr_size size = 0
        cdef bytes result
        if mx.binbuf_totext(<mx.t_binbuf *>self.ptr, contents, &size):
            error("could convert binbuf to text")
            raise MemoryError

        if size:
            result = <bytes>contents[0]
        mx.sysmem_freehandle(contents)
        return result.decode()

    cdef short getatom(self, long *p1, long *p2, mx.t_atom *ap):
        return mx.binbuf_getatom(self.ptr, p1, p2, ap)

    cdef void set(self, mx.t_symbol *s, short argc, mx.t_atom *argv):
        mx.binbuf_set(self.ptr, s, argc, argv)

    cdef void delete(self, long from_type, long to_type, long from_data, long to_data):
        mx.binbuf_delete(self.ptr, from_type, to_type, from_data, to_data)

    cdef void addtext(self, char **text, long size):
        mx.binbuf_addtext(self.ptr, text, size)

    cdef short readatom(self, char *outstr, char **text, long *n, long e, mx.t_atom *ap):
        """Use readatom() to read a single t_atom from a text buffer."""
        mx.readatom(outstr, text, n, e, ap)

# ---------------------------------------------------------------------------
# api.Atombuf

cdef class Atombuf:
    """An alternative to Binbufs for temporary storage of atoms.
    """
    cdef mx.t_atombuf* ptr
    cdef bint ptr_owner

    def __cinit__(self):
        self.ptr = NULL
        self.ptr_owner = False

    def __dealloc__(self):
        # De-allocate if not null and flag is set
        if self.ptr is not NULL and self.ptr_owner is True:
            mx.atombuf_free(self.ptr)
            self.ptr = NULL

    def __init__(self, *args):
        cdef Atom atom = Atom(*args) 
        self.ptr = <mx.t_atombuf*>mx.atombuf_new(atom.size, atom.ptr)
        self.ptr_owner = True
        if not self.ptr:
            raise MemoryError

    @staticmethod
    cdef Atombuf from_ptr(mx.t_atombuf *ptr, bint owner=False):
        cdef Atombuf atombuf = Atombuf.__new__(Atombuf)
        atombuf.ptr = ptr
        atombuf.ptr_owner = owner
        return atombuf

    @staticmethod
    def new() -> Atombuf:
        """create an empty Atombuf instance"""
        cdef mx.t_atombuf *ptr = <mx.t_atombuf*>mx.atombuf_new(0, NULL)
        if ptr is NULL:
            raise MemoryError
        return Atombuf.from_ptr(ptr, owner=True)

    def add_text(self, str text):
        """Adds a text -- convert text to a t_atom array in a t_atombuf.

        To use this routine to create a new Atombuf from the text buffer, 
        first create a new empty t_atombuf with a call to atombuf_new(0,NULL).
        """
        cdef char* src_text = <char *>mx.sysmem_newptr((len(text)+1) * sizeof(char))
        cdef long n = len(text) + 1
        strcpy(src_text, text.encode('utf8'))
        mx.atombuf_text(&self.ptr, &src_text, n)
        mx.sysmem_freeptr(src_text)

    def to_text(self) -> str:
        """Convert an atombuf into a text handle.
        """
        cdef mx.t_handle contents = mx.sysmem_newhandle(0)
        cdef long size = 0
        cdef bytes result
        if mx.atombuf_totext(<mx.t_atombuf *>self.ptr, contents, &size):
            error("could convert atombuf to text")
            raise MemoryError

        if size:
            result = <bytes>contents[0]
        mx.sysmem_freehandle(contents)
        return result.decode()


    def to_list(self) -> list:
        """convert contents of atombuf to a list"""
        cdef Atom atom = Atom.from_ptr(self.ptr.a_argv, self.ptr.a_argc)
        return atom.to_list()


# ---------------------------------------------------------------------------
# api.Hashtab

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
# api.AtomArray

cdef class AtomArray:
    cdef mx.t_atomarray *ptr
    cdef bint owner

    def __cinit__(self):
        self.ptr = NULL
        self.owner = False

    def __init__(self, *args):
        cdef Atom atom = Atom(*args)
        self.ptr = mx.atomarray_new(atom.size, atom.ptr)
        self.owner = True

    @staticmethod
    cdef AtomArray from_atom(mx.t_atom *av, int ac, bint owner=False):
        cdef AtomArray atom_array = AtomArray.__new__(AtomArray)
        atom_array.ptr = mx.atomarray_new(ac, av)
        atom_array.owner = owner
        return atom_array

    @staticmethod
    cdef AtomArray new(int size):
        cdef mx.t_atom *ptr = <mx.t_atom *>mx.sysmem_newptr(size * sizeof(mx.t_atom))
        if ptr is NULL:
            raise MemoryError
        return AtomArray.from_atom(ptr, size, owner=True)

    cdef void flags(self, long flags):
        mx.atomarray_flags(self.ptr, flags)

    cdef long getflags(self):
        return mx.atomarray_getflags(self.ptr)

    cdef mx.t_max_err setatoms(self, long ac, mx.t_atom* av):
        return mx.atomarray_setatoms(self.ptr, ac, av)

    cdef mx.t_max_err getatoms(self, long* ac, mx.t_atom** av):
        return mx.atomarray_getatoms(self.ptr, ac, av)

    cdef mx.t_max_err copyatoms(self, long* ac, mx.t_atom** av):
        return mx.atomarray_copyatoms(self.ptr, ac, av)

    cdef mx.t_atom_long getsize(self):
        return mx.atomarray_getsize(self.ptr)

    cdef mx.t_max_err getindex(self, long index, mx.t_atom* av):
        return mx.atomarray_getindex(self.ptr, index, av)

    # cdef mx.t_max_err setindex(self, long index, mx.t_atom* av):
    #     return mx.atomarray_setindex(self.ptr, index, av)

    cdef void* duplicate(self):
        return mx.atomarray_duplicate(self.ptr)

    cdef void* clone(self):
        return mx.atomarray_clone(self.ptr)

    cdef void appendatom(self, mx.t_atom* a):
        mx.atomarray_appendatom(self.ptr, a)

    cdef void appendatoms(self, long ac, mx.t_atom* av):
        mx.atomarray_appendatoms(self.ptr, ac, av)

    cdef void chuckindex(self, long index):
        mx.atomarray_chuckindex(self.ptr, index)

    cdef void clear(self):
        mx.atomarray_clear(self.ptr)

    cdef void funall(self, mx.method fun, void* arg):
        mx.atomarray_funall(self.ptr, fun, arg)


# ----------------------------------------------------------------------------
# api.Patcher

cdef class Patcher:
    """A wrapper class for a Max patcher
    """
    cdef mx.t_object *ptr
    cdef bint owner

    def __cinit__(self):
        self.ptr = NULL
        self.owner = False

    @staticmethod
    cdef Patcher from_object(mx.t_object *x):
        """Create a reference to a pstcher object from object."""
        cdef Patcher patcher = Patcher.__new__(Patcher)
        mx.object_obex_lookup(x, mx.gensym("#P"), &patcher.ptr)
        if patcher.ptr is NULL:
            raise MemoryError
        return patcher

    @staticmethod
    cdef Patcher from_ptr(mx.t_object *x, bint owner=False):
        cdef Patcher patcher = Patcher.__new__(Patcher)
        patcher.ptr = x
        patcher.owner = owner
        return patcher

    def is_patcher(self) -> bool:
        """determine if a t_object is a patcher"""
        return bool(mx.jpatcher_is_patcher(self.ptr))

    def get_sym_attr(self, name) -> str:
        cdef mx.t_symbol *attr_sym = <mx.t_symbol *>mx.object_attr_getsym(
            self.ptr, str_to_sym(name))
        return sym_to_str(attr_sym)

    def get_long_attr(self, name) -> int:
        return mx.object_attr_getlong(self.ptr, str_to_sym(name))

    def get_char_attr(self, name) -> bool:
        return mx.object_attr_getchar(self.ptr, str_to_sym(name))

    # array props

    def get_arr_attr(self, str name) -> list:
        """t_max_err object_attr_getvalueof(void *x, t_symbol *s, long *argc, t_atom **argv)"""
        cdef mx.t_atom *argv = NULL
        cdef long argc = 0

        mx.object_attr_getvalueof(self.ptr, str_to_sym(name), &argc, &argv)   
        atom = Atom.from_ptr(argv, argc)
        result = atom.to_list()
        mx.sysmem_freeptr(argv)
        return result

    # char/bool props

    @property
    def locked(self) -> bool:
        return self.get_char_attr("locked")

    @property
    def bglocked(self) -> bool:
        return self.get_char_attr("bglocked")

    @property
    def presentation(self) -> bool:
        return self.get_char_attr("presentation")

    @property
    def openinpresentation(self) -> bool:
        return self.get_char_attr("openinpresentation")

    @property
    def cansave(self) -> bool:
        return self.get_char_attr("cansave")

    @property
    def dirty(self) -> bool:
        return self.get_char_attr("dirty")

    @property
    def toolbarvisible(self) -> bool:
        return self.get_char_attr("toolbarvisible")

    # sym props

    @property
    def title(self) -> str:
        return self.get_sym_attr("title")

    @property
    def fulltitle(self) -> str:
        return self.get_sym_attr("fulltitle")

    @property
    def name(self) -> str:
        return self.get_sym_attr("name")

    @property
    def filename(self) -> str:
        return self.get_sym_attr("filename")

    @property
    def filepath(self) -> str:
        return self.get_sym_attr("filepath")

    # int/long props

    @property
    def count(self) -> int:
        return self.get_long_attr("count")

    @property
    def fgcount(self) -> int:
        return self.get_long_attr("fgcount")

    @property
    def bgcount(self) -> int:
        return self.get_long_attr("bgcount")

    @property
    def fileversion(self) -> int:
        return self.get_long_attr("fileversion")

    @property
    def numviews(self) -> int:
        return self.get_long_attr("numviews")

    @property
    def numwindowviews(self) -> int:
        return self.get_long_attr("numwindowviews")

    @property
    def default_fontface(self) -> int:
        return self.get_long_attr("default_fontface")

    @property
    def toolbarheight(self) -> int:
        return self.get_long_attr("toolbarheight")

    # array props

    @property
    def rect(self) -> list:
        return self.get_arr_attr("rect")

    cdef mx.t_object *newobject_sprintf(self, str text):
        """Create a new object in a specified patcher with values using a 
        combination of attribute and sprintf syntax.
        """
        return <mx.t_object *>mx.newobject_sprintf(
            <mx.t_object *>self.ptr, text.encode('utf-8'))

    def add_box(self, maxclass: str, x: float, y: float) -> bool:
        cdef mx.t_object *obj = self.newobject_sprintf(
            f"@maxclass {maxclass} @patching_position {x} {y}"
        )
        if obj is not NULL:
            return True
        return False

    def add_textbox(self, text: str, x: float, y: float, maxclass='newobj') -> bool:
        cdef mx.t_object *obj = self.newobject_sprintf(
           f'@maxclass {maxclass} @text "{text}" @patching_position {x} {y}'
        )
        if obj is not NULL:
            return True
        return False

    cdef mx.t_object *newobject_fromboxtext(self, str text):
        """Create an object from the passed in text.

        The passed in text is in the same format as would be typed into an object box.
        It can be used for UI objects or text objects so this is the simplest way to 
        create objects from C.
        """
        return mx.newobject_fromboxtext(self.ptr, text.encode('utf-8'))

    def add_tbox(self, str text) -> bool:
        """Create an object from the passed in text."""
        cdef mx.t_object *obj = self.newobject_fromboxtext(text)
        if obj is not NULL:
            return True
        return False

    def _method_noargs(self, str name):
        """object method call with no arguments"""
        cdef mx.t_max_err err = mx.object_method_typed(
            <mx.t_object *>self.ptr, str_to_sym(name), 0, NULL, NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def _method_args(self, str name, *args):
        """strongly typed object method call with arguments"""
        cdef Atom atom = Atom(*args)
        cdef mx.t_max_err err = mx.object_method_typed(
            <mx.t_object *>self.ptr, str_to_sym(name), atom.size, atom.ptr, NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def _method_parsestr(self, str name, str parsestr):
        """combines object_method_typed() + atom_setparse() to define method arguments."""
        cdef mx.t_max_err err = mx.object_method_parse(
            <mx.t_object *>self.ptr, str_to_sym(name), parsestr.encode('utf8'), NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def _method_float(self, str name, float number):
        """wrapper for object_method_typed() that passes a single float as an argument"""
        cdef mx.t_max_err err = mx.object_method_float(
            <mx.t_object *>self.ptr, str_to_sym(name), number, NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def _method_double(self, str name, double number):
        """wrapper for object_method_typed() that passes a single double as an argument"""
        cdef mx.t_max_err err = mx.object_method_double(
            <mx.t_object *>self.ptr, str_to_sym(name), number, NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def _method_long(self, str name, long number):
        """wrapper for object_method_typed() that passes a single long as an argument"""
        cdef mx.t_max_err err = mx.object_method_long(
            <mx.t_object *>self.ptr, str_to_sym(name), number, NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def _method_sym(self, str name, str symbol):
        """wrapper for object_method_typed() that passes a single t_symbol as an argument"""
        cdef mx.t_max_err err = mx.object_method_sym(
            <mx.t_object *>self.ptr, str_to_sym(name), str_to_sym(symbol), NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def call(self, str name, *args, parse=False):
        """general call object method function (strongly typed)"""
        if len(args) == 0:
            return self._method_noargs(name)
        elif len(args) == 1:
            if isinstance(args[0], str):
                if parse:
                    return self._method_parsestr(name, args[0])
                else:
                    return self._method_sym(name, args[0])
            elif isinstance(args[0], float):
                return self._method_double(name, args[0])
            elif isinstance(args[0], int):
                return self._method_long(name, args[0])
            elif isinstance(args[0], list):
                return self.call(name, *args[0])
            elif isinstance(args[0], tuple):
                return self.call(name, *args[0])
            elif isinstance(args[0], set):
                return self.call(name, *args[0])
        else:
            return self._method_args(name, *args)

    # patcher scripting

    def set_dirty(self):
        """set dirty bit in the window (user will be asked to save changes)"""
        self.call("dirty")

    def set_clean(self):
        """reverse setting dirty bit"""
        self.call("clean")

    def save(self):
        """save current patcher"""
        self.call("write")

    def set_title(self, title: str):
        """set current patcher's title"""
        self.call("title", f'"{title}"')

    def front(self):
        """bring the window to the front, or open it and bring it to the front"""
        self.call("front")

    def set_active_tab(self, name: str):
        """set named tab to be active"""
        self.call("setactivetab", name)

    def fullsize(self, on: bool):
        """switch window fullsize on and off"""
        self.call("fullsize", on)

    def dispose(self):
        """Closes the patcher or destroy the subpatcher (with thispatcher)."""
        self.script("dispose")

    # TODO patcher window commands


    # box scripting

    def script(self, *args):
        """calls a patcher script command"""
        self.call("script", *args)

    ## assigning varnames

    def assign_name_by_index(self, var1: str, maxclass: str, index: int):
        """mame the nth box instance of the class"""
        self.script("nth", var1, maxclass, index)

    def assign_name_by_text(self, var1: str, text: str):
        """name a box with created from the exact provided text"""
        self.script("class", var1, text)

    ## object creation / deletion

    def newobject(self, varname: str, x: int, y: int, maxclass: str, *args):
        """creates an object with varname from args"""
        self.script("newobject", maxclass, "@varname", varname, "@patching_position", x, y, *args)

    def newdefault(self, varname: str, x: int, y: int, maxclass: str, *args):
        """Creates a new named object with default properties in a patcher window."""
        self.script("newdefault", varname, x, y, maxclass, *args)

    def new(self, varname: str, maxclass: str, x: int, y: int, w: int, h: int=0, *args):
        """Creates a new object in a patcher window and gives it a name.
        
        The format of the arguments (after the class name) are based
        on the legacy Max file format.
        """
        self.script("new", varname, maxclass, x, y, w, h, *args)

    def delete(self, var1: str):
        """delete a named box"""
        self.script("delete", var1)

    ## connecting patchlines

    def connect(self, var1: str, outlet: int, var2: str, inlet: int):
        """connect two named objects"""
        self.script("connect", var1, outlet, var2, inlet)

    def disconnect(self, var1: str, outlet: int, var2: str, inlet: int):
        """diconnect an existing connection between two named variabless"""
        self.script("disconnect", var1, outlet, var2, inlet)

    def connectcolor(self, var1: str, outlet: int, var2: str, inlet: int, color: int):
        """Modify the color of an existing patch cord, setting it to one of Max's 16 standard colors."""
        assert color < 16, "color index is only from 0 to 15 inclusive"
        self.script("connectcolor", var1, outlet, var2, inlet, color)

    ## sending messages

    def send(self, var1: str, *args):
        """send an message to the object contained by a named box"""
        self.script("send", var1, *args)

    def sendbox(self, var1: str, *args):
        """send a message to a a named box

        There is currently only one object, bpatcher, in which the object and box
        are different objects.
        """
        self.script("sendbox", var1, *args)

    def sendpatchline(self, from_var: str, outlet: int, to_var: str, inlet: int, *args):
        """sends a message to a patchline specified from the connection."""
        self.script("sendpatchline", from_var, outlet, to_var, inlet, *args)

    ## box visibility / responsiveness

    def hide(self, var1: str):
        """hide a named box"""
        self.script("hide", var1)

    def show(self, var1: str):
        """show a hidden named box"""
        self.script("show", var1)

    def ignore_click(self, var1: str):
        """make a named box ignore clicks"""
        self.script("ignoreclick", var1)

    def respond_to_click(self, var1: str):
        """make a named box respond to clicks"""
        self.script("respondtoclick", var1)

    ## moving / resizing boxes

    def move(self, var1: str, x: int, y: int):
        """move a named box"""
        self.script("move", x, y)

    def offset(self, var1: str, x: int, y: int):
        """relative 'offset' move of named box"""
        self.script("offset", x, y)

    def offset_from(self, var2: str, var1: str, from_bottom_right: int,
                    from_top_left: int, x_distance: int, y_distance: int = 0):
        """relative 'offset' move of named box relative to another"""
        self.script("offsetfrom", var2, var1,
            from_bottom_right, from_top_left, x_distance, y_distance)

    def size(self, var1: str, width: int, height: int):
        """resize a named box"""
        self.script("size", var1, width, height)

    def send_to_back(self, var1: str):
        """send named box to back layer a """
        self.script("sendtoback", var1)

    def bring_to_front(self, var1: str):
        """bring named box to front layer"""
        self.script("bringtofront", var1)

    def background(self, var1: str):
        """bring named box to background??"""
        self.script("background", var1)

    # box related

    def get_count(self) -> bool:
        """Determine the number of boxes in a patcher."""
        return mx.jpatcher_get_count(self.ptr)

    def get_presentation(self) -> bool:
        """Determine whether a patcher is currently in presentation mode."""
        return mx.jpatcher_get_presentation(self.ptr)

    cdef mx.t_object* get_first_box(self):
        """Get the first box in a patcher."""
        return mx.jpatcher_get_firstobject(self.ptr)

    cdef mx.t_object* get_last_box(self):
        """Get the last box in a patcher."""
        return mx.jpatcher_get_lastobject(self.ptr)

    cdef mx.t_object* get_first_line(self):
        """Get the first line (patch-cord) in a patcher."""
        return mx.jpatcher_get_firstline(self.ptr)

    cdef mx.t_object* get_first_view(self):
        """Get the first view (jpatcherview) for a given patcher."""
        return mx.jpatcher_get_firstview(self.ptr)

    # def get_view(self) -> MaxObject:
    #     """Get the first view (jpatcherview) for a given patcher."""
    #     cdef mx.t_object* view = mx.jpatcher_get_firstview(self.ptr)
    #     return MaxObject.from_ptr(<mx.t_object*>view)

    def get_title(self) -> str:
        """Retrieve a patcher's title."""
        cdef mx.t_symbol* title = mx.jpatcher_get_name(self.ptr)
        return sym_to_str(title)

    def get_name(self) -> str:
        """Retrieve a patcher's name."""
        cdef mx.t_symbol* name = mx.jpatcher_get_name(self.ptr)
        return sym_to_str(name)

    def get_filepath(self) -> str:
        """get the patchers filepath"""
        cdef mx.t_symbol* fpath = mx.jpatcher_get_filepath(self.ptr)
        return sym_to_str(fpath)

    def get_filename(self) -> str:
        """Retrieve a patcher's file name."""
        cdef mx.t_symbol* fname = mx.jpatcher_get_filename(self.ptr)
        return sym_to_str(fname)

    cdef char get_dirty(self):
        """Determine whether a patcher's dirty bit has been set."""
        return mx.jpatcher_get_dirty(self.ptr)

    # cdef mx.t_max_err set_dirty(self, char c):
    #     """Set a patcher's dirty bit."""
    #     return mx.jpatcher_set_dirty(self.ptr, c)

    def get_bglocked(self) -> bool:
        """Determine whether a patcher's background layer is locked."""
        return mx.jpatcher_get_bglocked(self.ptr)

    def get_bgcolor(self) -> Rgba:
        """Retrieve a patcher's unlocked background color."""
        cdef mx.t_jrgba c
        cdef mx.t_max_err err = mx.jpatcher_get_bgcolor(self.ptr, &c)
        if err == mx.MAX_ERR_NONE:
            return Rgba(c.red, c.green, c.blue, c.alpha)
        return error("could not get patcher bgcolor")

    def set_bgcolor(self, c: Rgba):
        """Set a patcher's unlocked background color."""
        cdef mx.t_jrgba rgba = c
        cdef mx.t_max_err err = mx.jpatcher_set_bgcolor(self.ptr, &rgba)
        if err != mx.MAX_ERR_NONE:
            return error("could not set patcher bgcolor")

    def get_gridsize(self) -> tuple:
        """Retrieve a patcher's grid size."""
        cdef double grid_size_x
        cdef double grid_size_y
        cdef mx.t_max_err err = mx.jpatcher_get_gridsize(
            self.ptr, &grid_size_x, &grid_size_y)
        if err == mx.MAX_ERR_NONE:
            return (grid_size_x, grid_size_y)
        return error("could not get patcher gridsize")

    def set_gridsize(self, grid_size_x: float, grid_size_y: float):
        """Set a patcher's grid size."""
        cdef mx.t_max_err err = mx.jpatcher_set_gridsize(
            self.ptr, grid_size_x, grid_size_y)
        if err != mx.MAX_ERR_NONE:
            return error("could not set patcher gridsize")

    def get_rect(self) -> Rect:
        """Query a patcher to determine its location and size."""
        cdef mx.t_rect r
        cdef mx.t_max_err err = mx.jpatcher_get_rect(self.ptr, &r)
        if err == mx.MAX_ERR_NONE:
            return Rect(r.x, r.y, r.width, r.height)
        return error("could not get patcher rect")

    def set_rect(self, r: Rect):
        """Set a patcher's location and size."""
        cdef mx.t_rect rect = r
        cdef mx.t_max_err err = mx.jpatcher_set_rect(self.ptr, &rect)
        if err != mx.MAX_ERR_NONE:
            return error("could not set patcher rect")

    cdef void deleteobj(self, mx.t_jbox *b):
        """Delete an object that is in a patcher."""
        mx.jpatcher_deleteobj(self.ptr, b)

    def get_fileversion(self) -> int:
        """Return the file version of the patcher."""
        return mx.jpatcher_get_fileversion(self.ptr)

    def get_currentfileversion(self) -> int:
        """Return the file version for any new patchers, e.g. the current version created by Max."""
        return mx.jpatcher_get_currentfileversion()

    cdef mx.t_object* get_parentpatcher(self):
        """Given a patcher, return its parent patcher."""
        return mx.jpatcher_get_parentpatcher(self.ptr)

    cdef mx.t_object* get_toppatcher(self):
        """Given a patcher, return the top-level patcher for the tree in which it exists."""
        return mx.jpatcher_get_toppatcher(self.ptr)

    cdef mx.t_object* get_hubholder(self):
        """Given a patcher, return the patcher that will be responsible for holding the parameter hub."""
        return mx.jpatcher_get_hubholder(self.ptr)

    cdef mx.t_symbol *uniqueboxname(self, mx.t_symbol *classname):
        """Generate a unique name for a box in patcher."""
        return mx.jpatcher_uniqueboxname(self.ptr, classname)


# ----------------------------------------------------------------------------
# api.Box

cdef class Box:
    """Wraps a Max t_jbox user-interface object"""
    cdef mx.t_object *ptr
    cdef mx.t_object *patcherview

    def __cinit__(self):
        self.ptr = NULL
        self.patcherview = NULL

    @staticmethod
    cdef Box from_ptr(mx.t_object *x):
        """Create a box object from a box pointer"""
        cdef Box box = Box.__new__(Box)
        mx.object_obex_lookup(x, mx.gensym("#P"), &box.ptr)
        if box.ptr is NULL:
            raise MemoryError
        return box

    def get_rect_for_view(self) -> Rect:
        """Find the rect for a box in a given patcherview."""
        cdef mx.t_rect pr
        cdef mx.t_max_err err = mx.jbox_get_rect_for_view(self.ptr, self.patcherview, &pr)
        if err != mx.MAX_ERR_NONE:
            return error("could not get rect from patcherview's box")
        return Rect(pr.x, pr.y, pr.width, pr.height)

    def set_rect_for_view(self, rect: Rect):
        """Change the rect for a box in a given patcherview."""
        cdef mx.t_rect pr = rect
        cdef mx.t_max_err err = mx.jbox_set_rect_for_view(self.ptr, self.patcherview, &pr)
        if err != mx.MAX_ERR_NONE:
           return error("could not set rect for box in patcherview")

    def get_rect_for_sym(self, which: str) -> Rect:
        """Find the rect for a box with a given attribute name."""
        cdef mx.t_rect pr
        cdef mx.t_max_err err = mx.jbox_get_rect_for_sym(self.ptr, str_to_sym(which), &pr)
        if err != mx.MAX_ERR_NONE:
           return error("could not get rect for box given attribute name")
        return Rect(pr.x, pr.y, pr.width, pr.height)

    def set_rect_for_sym(self, which: str, rect: Rect):
        """Change the rect for a box with a given attribute name."""
        cdef mx.t_rect pr  = rect
        cdef mx.t_max_err err = mx.jbox_set_rect_for_sym(self.ptr, str_to_sym(which), &pr)
        if err != mx.MAX_ERR_NONE:
           return error("could not set rect for box with given attribute name")

    def set_rect(self, rect: Rect):
        """Set both the presentation rect and the patching rect."""
        cdef mx.t_rect pr  = rect
        cdef mx.t_max_err err = mx.jbox_set_rect(self.ptr, &pr)
        if err != mx.MAX_ERR_NONE:
           return error("could not set rect for box")

    def get_patching_rect(self):
        """Retrieve the patching rect of a box."""
        cdef mx.t_rect pr
        cdef mx.t_max_err err = mx.jbox_get_patching_rect(self.ptr, &pr)
        if err != mx.MAX_ERR_NONE:
           return error("could not get patching rect for box")
        return Rect(pr.x, pr.y, pr.width, pr.height)

    def set_patching_rect(self, rect: Rect):
        """Change the patching rect of a box."""
        cdef mx.t_rect pr  = rect
        cdef mx.t_max_err err = mx.jbox_set_patching_rect(self.ptr, &pr)
        if err != mx.MAX_ERR_NONE:
           return error("could not set patching rect for box")

    def get_presentation_rect(self):
        """Retrieve the presentation rect of a box."""
        cdef mx.t_rect pr
        cdef mx.t_max_err err = mx.jbox_get_presentation_rect(self.ptr, &pr)
        if err != mx.MAX_ERR_NONE:
           return error("could not get presentation rect for box")
        return Rect(pr.x, pr.y, pr.width, pr.height)

    def set_presentation_rect(self, rect: Rect):
        """Change the presentation rect of a box."""
        cdef mx.t_rect pr  = rect
        cdef mx.t_max_err err = mx.jbox_set_presentation_rect(self.ptr, &pr)
        if err != mx.MAX_ERR_NONE:
           return error("could not set presentation rect for box")

    def set_position(self, x: float, y: float):
        """Set the position of a box for both presentation and patching views."""
        cdef mx.t_pt pos  = (x, y)
        cdef mx.t_max_err err = mx.jbox_set_position(self.ptr, &pos)
        if err != mx.MAX_ERR_NONE:
           return error("could not set the position of a box for both views")

    def get_patching_position(self) -> tuple:
        """Fetch the position of a box for the patching view."""
        cdef mx.t_pt pos
        cdef mx.t_max_err err = mx.jbox_get_patching_position(self.ptr, &pos)
        if err != mx.MAX_ERR_NONE:
           return error("could not get patching position for box")
        return (pos.x, pos.y)

    def set_presentation_position(self, x: float, y: float):
        """Set the position of a box for the presentation view."""
        cdef mx.t_pt pos  = (x, y)
        cdef mx.t_max_err err = mx.jbox_set_presentation_position(self.ptr, &pos)
        if err != mx.MAX_ERR_NONE:
           return error("could not set the position of a box for presentation view")

    def set_size(self, width: float, height: float):
        """Set the size of a box for both the presentation and patching views."""
        cdef mx.t_size size  = (width, height)
        cdef mx.t_max_err err = mx.jbox_set_size(self.ptr, &size)
        if err != mx.MAX_ERR_NONE:
           return error("could not set the size of a box for both views")

    def get_patching_size(self) -> tuple:
        """Fetch the size of a box for the patching view."""
        cdef mx.t_size size
        cdef mx.t_max_err err = mx.jbox_get_patching_size(self.ptr, &size)
        if err != mx.MAX_ERR_NONE:
           return error("could not get patching size for box")
        return (size.width, size.height)

    def set_patching_size(self, width: float, height: float):
        """Set the size of a box for the patching view."""
        cdef mx.t_size size  = (width, height)
        cdef mx.t_max_err err = mx.jbox_set_patching_size(self.ptr, &size)
        if err != mx.MAX_ERR_NONE:
           return error("could not set the size of a box for the patching view.")

    def get_presentation_size(self) -> tuple:
        """Fetch the size of a box for the presentation view."""
        cdef mx.t_size size
        cdef mx.t_max_err err = mx.jbox_get_presentation_size(self.ptr, &size)
        if err != mx.MAX_ERR_NONE:
           return error("could not get presentation size for box")
        return (size.width, size.height)

    def set_presentation_size(self, width: float, height: float):
        """Set the size of a box for the presentation view."""
        cdef mx.t_size size  = (width, height)
        cdef mx.t_max_err err = mx.jbox_set_presentation_size(self.ptr, &size)
        if err != mx.MAX_ERR_NONE:
           return error("could not set the size of a box for the presentation view.")

    def get_maxclass(self) -> str:
        """Retrieve the name of the class of the box's object."""
        cdef mx.t_symbol* name = mx.jbox_get_maxclass(self.ptr)
        return sym_to_str(name)

    def get_object(self) -> MaxObject:
        """Retrieve the box's object."""
        cdef mx.t_object* obj = mx.jbox_get_object(self.ptr)
        return MaxObject.from_ptr(<mx.t_object*>obj)

    def get_patcher(self) -> Patcher:
        """Retrieve a box's patcher."""
        cdef mx.t_object* patcher = mx.jbox_get_patcher(self.ptr)
        if patcher is not NULL:
            return Patcher.from_ptr(<mx.t_object*>patcher)
        error("box does not have an associated patcher""")

    def get_nextobject(self) -> Box:
        """The next box in the patcher's (linked) list of boxes."""
        cdef mx.t_object* next_box = mx.jbox_get_nextobject(self.ptr)
        return Box.from_ptr(next_box)

    def get_prevobject(self) -> Box:
        """The prior box in the patcher's (linked) list of boxes."""
        cdef mx.t_object* next_box = mx.jbox_get_prevobject(self.ptr)
        return Box.from_ptr(next_box)

    def is_hidden(self) -> bool:
        """Return true if box is hidden"""
        return bool(mx.jbox_get_hidden(self.ptr))

    def set_hidden(self, on: bool):
        """Set a box hidden attribute"""
        cdef mx.t_max_err err = mx.jbox_set_hidden(self.ptr, on)
        if err != mx.MAX_ERR_NONE:
           return error("could not set box's hidden attribute")

    def get_fontname(self) -> str:
        """Retrieve a box's 'fontname' attribute."""
        cdef mx.t_symbol* name = mx.jbox_get_fontname(self.ptr)
        return sym_to_str(name)

    def set_fontname(self, name: str):
        """Set a box's 'fontname' attribute."""
        cdef mx.t_max_err err = mx.jbox_set_fontname(self.ptr, str_to_sym(name))
        if err != mx.MAX_ERR_NONE:
           return error("could not set box's fontname attribute")

    def get_fontsize(self) -> float:
        """Retrieve a box's 'fontsize' attribute."""
        return mx.jbox_get_fontsize(self.ptr)

    def set_fontsize(self, size: float):
        """Set a box's 'fontsize' attribute."""
        cdef mx.t_max_err err = mx.jbox_set_fontsize(self.ptr, size)
        if err != mx.MAX_ERR_NONE:
           return error("could not set box's fontsize attribute")

    def get_color(self) -> Rgba:
        """Retrieve a box's 'color' attribute."""
        cdef mx.t_jrgba c
        cdef mx.t_max_err err = mx.jbox_get_color(self.ptr, &c)
        if err == mx.MAX_ERR_NONE:
            return Rgba(c.red, c.green, c.blue, c.alpha)
        return error("could not get box's color")

    def set_color(self, color: Rgba):
        """Set a box's 'color' attribute."""
        cdef mx.t_jrgba c = color
        cdef mx.t_max_err err = mx.jbox_set_color(self.ptr, &c)
        if err != mx.MAX_ERR_NONE:
           return error("could not set box's color")
 
    def get_varname(self) -> str:
        """Retrieve a box's scripting name."""
        cdef mx.t_symbol* varname = mx.jbox_get_varname(self.ptr)
        return sym_to_str(varname)

    def set_varname(self, varname: str):
        """set a box's scripting name."""
        cdef mx.t_max_err err = mx.jbox_set_varname(self.ptr, str_to_sym(varname))
        if err != mx.MAX_ERR_NONE:
           return error("could not set box's scripting name")

    def get_id(self) -> str:
        """Retrieve a box's unique id."""
        cdef mx.t_symbol* _id = mx.jbox_get_id(self.ptr)
        return sym_to_str(_id)


# ----------------------------------------------------------------------------
# api.MaxApp

cdef class MaxApp:
    """a class to enable messages to the 'max' application"""
    cdef mx.t_object *ptr

    def __cinit__(self):
        self.ptr = <mx.t_object*>mx.object_new(
            mx.gensym("nobox"), mx.gensym("max"))

    def __dealloc__(self):
        if self.ptr:
            mx.object_free(self.ptr)

    def _method_noargs(self, str name):
        """object method call with no arguments"""
        cdef mx.t_max_err err = mx.object_method_typed(
            <mx.t_object *>self.ptr, str_to_sym(name), 0, NULL, NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def _method_args(self, str name, *args):
        """strongly typed object method call with arguments"""
        cdef Atom atom = Atom(*args)
        cdef mx.t_max_err err = mx.object_method_typed(
            <mx.t_object *>self.ptr, str_to_sym(name), atom.size, atom.ptr, NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def _method_parsestr(self, str name, str parsestr):
        """combines object_method_typed() + atom_setparse() to define method arguments."""
        cdef mx.t_max_err err = mx.object_method_parse(
            <mx.t_object *>self.ptr, str_to_sym(name), parsestr.encode('utf8'), NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def _method_float(self, str name, float number):
        """wrapper for object_method_typed() that passes a single float as an argument"""
        cdef mx.t_max_err err = mx.object_method_float(
            <mx.t_object *>self.ptr, str_to_sym(name), number, NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def _method_double(self, str name, double number):
        """wrapper for object_method_typed() that passes a single double as an argument"""
        cdef mx.t_max_err err = mx.object_method_double(
            <mx.t_object *>self.ptr, str_to_sym(name), number, NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def _method_long(self, str name, long number):
        """wrapper for object_method_typed() that passes a single long as an argument"""
        cdef mx.t_max_err err = mx.object_method_long(
            <mx.t_object *>self.ptr, str_to_sym(name), number, NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def _method_sym(self, str name, str symbol):
        """wrapper for object_method_typed() that passes a single t_symbol as an argument"""
        cdef mx.t_max_err err = mx.object_method_sym(
            <mx.t_object *>self.ptr, str_to_sym(name), str_to_sym(symbol), NULL)
        if err == mx.MAX_ERR_NONE:
            return
        return error(f"method '{name}' call failed")

    def call(self, str name, *args, parse=False):
        """general call object method function (strongly typed)"""
        if len(args) == 0:
            return self._method_noargs(name)
        elif len(args) == 1:
            if isinstance(args[0], str):
                if parse:
                    return self._method_parsestr(name, args[0])
                else:
                    return self._method_sym(name, args[0])
            elif isinstance(args[0], float):
                return self._method_double(name, args[0])
            elif isinstance(args[0], int):
                return self._method_long(name, args[0])
            elif isinstance(args[0], list):
                return self.call(name, *args[0])
            elif isinstance(args[0], tuple):
                return self.call(name, *args[0])
        else:
            return self._method_args(name, *args)

    def clearmaxwindow(self):
        self.call("clearmaxwindow")

    def midilist(self):
        self.call("midilist")

    def clean(self):
        self.call("clean")

    def maxwindow(self):
        self.call("maxwindow")

    def paths(self):
        self.call("paths")

    def clearmaxwindow(self):
        self.call("clearmaxwindow")

    def externaleditor(self, str name):
        self.call("externaleditor". str_to_sym(name))


# ----------------------------------------------------------------------------
# api.PyExternal

cdef class PyExternal:
    """Wraps the `py` external object and its methods.

    Should expose as much functionality as possible.
    """
    cdef px.t_py *ptr
    cdef bytes name

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
        self.ptr = <px.t_py *>mx.object_findregistered(
            mx.CLASS_BOX, mx.gensym(self.name))

    def bang(self):
        """Send bang out of left (default) outlet"""
        px.py_bang(self.ptr)

    def bang_success(self):
        """signal success by banging out of right outliet outlet"""
        px.py_bang_success(self.ptr)

    def bang_failure(self):
        """signal failure by banging out of middle outlet"""
        px.py_bang_failure(self.ptr)

    def log_info(self, str msg):
        """log info using object_post"""
        px.py_info(self.ptr, msg.encode('utf8'))

    def log_debug(self, str msg):
        """log debug using object_post"""
        px.py_debug(self.ptr, msg.encode('utf8'))

    def log_error(self, str s):
        """log error using object_error"""
        px.py_error(self.ptr, s.encode('utf8'))

    def scan(self):
        """scanned patcher for named objects"""
        px.py_scan(self.ptr)

    def lookup(self, str name) -> bool:
        """lookup varname in object registry"""
        cdef mx.t_hashtab* registry = px.py_get_global_registry()
        cdef mx.t_object* obj = NULL
        cdef mx.t_max_err err

        if (mx.hashtab_getsize(registry) == 0):
            self.log_error("registry not populated")
            return

        err = mx.hashtab_lookup(registry, str_to_sym(name), &obj)

        if ((err != mx.MAX_ERR_NONE) or (obj == NULL)):
            self.log_error("no object found with name")
            return False
        else:
            self.log_debug("found object")
            return True

    def get_patcher(self) -> Patcher:
        """get containing patcher"""
        patcher = Patcher.from_object(<mx.t_object*>self.ptr)
        return patcher

    def get_buffer(self, name: str) -> Buffer:
        """retrieve buffer by name"""
        buf = Buffer.from_name(<mx.t_object*>self.ptr, name)
        return buf

    def create_buffer(self, name: str, sample_file: str) -> Buffer:
        """create buffer with name from file"""
        buf = Buffer.new(<mx.t_object*>self.ptr, name, sample_file)
        return buf

    def create_empty_buffer(self, str name, int duration_ms):
        """creates empty named buffer with duration in milliseconds"""
        buf = Buffer.empty(<mx.t_object*>self.ptr, name, duration_ms)
        return buf

    # def send(self, str name, list args):
    #     """general message send to receiver"""
    #     cdef long argc = <long>len(args) + 1
    #     cdef mx.t_atom argv[PY_MAX_ATOMS]
    #     _args = [name] + args
    #     cdef Atom atom = Atom(*_args)
    #     assert isinstance(args[0], str), "send first arg must be str name of receiver"
    #     px.py_send(self.ptr, mx.gensym(""), atom.size, atom.ptr)

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
        px.py_send(self.ptr, mx.gensym(""), argc, argv)

    cdef bint table_exists(self, str table_name):
        return px.py_table_exists(self.ptr, table_name.encode('utf-8'))

    cdef mx.t_max_err list_to_table(self, char* table_name, PyObject* plist):
        return px.py_list_to_table(self.ptr, table_name, plist)

    cdef PyObject* table_to_list(self, char* table_name):
        return px.py_table_to_list(self.ptr, table_name)



    cdef out_sym(self, str arg):
        mx.outlet_anything(<void*>px.get_outlet(self.ptr), str_to_sym(arg), 0, NULL)

    cdef out_float(self, float arg):
        mx.outlet_float(<void*>px.get_outlet(self.ptr), <double>arg)

    cdef out_int(self, int arg):
        mx.outlet_int(<void*>px.get_outlet(self.ptr), <long>arg)

    cdef out_list(self, list arg):
        """note: not recursive...(yet) still cannot deal with list in list"""
        cdef Atom atom = Atom.from_seq(arg)
        cdef int i

        for i, elem in enumerate(arg):
            if type(elem) == float:
                atom.set_float(i, <double>elem)
            elif type(elem) == int:
                atom.set_long(i, <long>elem)
            elif type(elem) == str:
                atom.set_symbol(i, elem)
            else:
                continue
        mx.outlet_list(<void*>px.get_outlet(self.ptr),
            mx.gensym("list"), atom.size, atom.ptr)

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

    def out(self, arg: object):
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
        return mx.object_method_binbuf(<mx.t_object*>self.ptr, s, buf, rv)



# ----------------------------------------------------------------------------
# Alternative external extension type (obj pointer retrieved via uintptr_t

cdef class PyMxObject:
    cdef px.t_py *x

    def __cinit__(self):
        self.x = <px.t_py*>px.py_get_object_ref()

    cpdef bang(self):
        px.py_bang(self.x)

def test_ref():
    ext = PyMxObject()
    ext.bang()


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


## general helpers

def hello():
    ext = PyExternal()
    ext.out("Hello World")

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

def out(object obj):
    ext = PyExternal()
    ext.out(obj)

def out2(object obj):
    cdef px.t_py* x = <px.t_py*>px.py_get_object_ref()
    px.py_handle_output(x, <PyObject *>obj)

def send(name, *args):
    ext = PyExternal()
    ext.send(name, list(args))

def lookup(name):
    ext = PyExternal()
    ext.lookup(name)

def post(str s):
    mx.post(s.encode('utf-8'))

def error(str s):
    mx.error(s.encode('utf-8'))


## get object helpers

def get_patcher() -> Patcher:
    ext = PyExternal()
    patcher = ext.get_patcher()
    return patcher
    
def get_buffer(name: str) -> Buffer:
    ext = PyExternal()
    buf = ext.get_buffer(name)
    return buf

def get_max():
    cdef mx.t_object *maxobj = <mx.t_object*>mx.object_new(
            mx.gensym("nobox"), mx.gensym("max"))
    if maxobj is NULL:
        error("could not get max object")




## buffer helpers

def create_buffer(name: str, sample_file: str) -> Buffer:
    ext = PyExternal()
    buf = ext.create_buffer(name, sample_file)
    return buf

def create_empty_buffer(name: str, duration_ms: int) -> Buffer:
    ext = PyExternal()
    buf = ext.create_empty_buffer(name, duration_ms)
    return buf

