# api.pyx
"""api: max api wrapped by cython for use by `py` external

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

Note that any function or symbol from the max api must be prefixed
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


This separation of namespaces is obviously very useful in this case. 


## TODO

implement the following:

- [ ] newinstance() -- privately instanciates objects
- [ ] typedmess() -- sends typed messages to objects (given object ids)


"""

# ----------------------------------------------------------------------------
# imports
 
cimport cython
from cpython cimport PyFloat_AsDouble
from cpython cimport PyLong_AsLong
from cpython.ref cimport PyObject

from libc.stdlib cimport malloc, free
from libc.string cimport strcpy, strlen

cimport api_max as mx # api is a cython keyword!
cimport api_msp as mp
cimport api_py as px

# ----------------------------------------------------------------------------
# conditional imports

DEF INCLUDE_NUMPY = True

if INCLUDE_NUMPY:
    import numpy as np
    cimport numpy as np
    np.import_array()

# ----------------------------------------------------------------------------
# constants

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




# ----------------------------------------------------------------------------
# Atom extension type
#
# All type-casting to and from non-max types should be encapsulated here

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
        return (self.ptr + idx).a_type  == mx.A_SYM

    cdef bint is_long(self, int idx=0):
        return (self.ptr + idx).a_type  == mx.A_LONG

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
    cdef Atom from_list(list lst):
        cdef int size = len(lst)
        cdef mx.t_atom *ptr = <mx.t_atom *>mx.sysmem_newptr(size * sizeof(mx.t_atom))
        if ptr is NULL:
            raise MemoryError

        cdef int i
        for i, obj in enumerate(lst):
            
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
# PyExternal extension type
# 
# Wrapper around the `py` external object and its methods. Should expose as much
# functionality as possible.
# 


cdef class PyExternal:
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
        cdef mx.t_hashtab* registry = px.get_global_registry()
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

    # UNTESTED
    cdef str atoms_to_pstring(self, long argc, mx.t_atom* argv):
        """atoms -> python string"""
        cdef long textsize = 0
        cdef char* text = NULL
        cdef mx.t_max_err err = mx.atom_gettext(argc, argv, &textsize, &text, 
            mx.OBEX_UTIL_ATOM_GETTEXT_DEFAULT)
        pstr = PyUnicode_FromString(text)
        mx.sysmem_freeptr(text)
        return pstr

    # UNTESTED
    cdef int pstring_to_atoms(self, str parsestr, long argc, mx.t_atom *argv) except -1:
        cdef char cparsestring[MAX_CHARS]
        cparsestring = PyUnicode_AsUTF8(parsestr)
        cdef mx.t_max_err err = mx.atom_setparse(&argc, &argv, cparsestring)
        if err != mx.MAX_ERR_NONE: # test this!!
            raise Exception("cannot convert c parsestring to atom array")

    # UNTESTED
    cdef int cstring_to_atoms(self, char *parsestr, long argc, mx.t_atom *argv) except -1:
        cdef mx.t_max_err err = mx.atom_setparse(&argc, &argv, parsestr)
        if err != mx.MAX_ERR_NONE: # test this!!
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
        atoms = Atom.from_list(list(args))
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

        mx.outlet_list(<void*>px.get_outlet(self.obj), mx.gensym("list"),
            argc, argv)

    cdef out_dict(self, dict arg):
        """note: not recursive...(yet) still cannot deal with dict in dict"""
        res = []
        for k,v in arg.items():
            res.append(k)
            res.append(':')
            if type(v) in [list, set, tuple]:
                for i in v:
                    res.append(i)
            else:
                res.append(v)
        self.out_list(res)

    cdef out(self, object arg):
        if isinstance(arg, float): self.out_float(arg)
        elif isinstance(arg, int): self.out_int(arg)
        elif isinstance(arg, str): self.out_sym(arg)
        elif isinstance(arg, list): self.out_list(arg)
        elif isinstance(arg, dict): self.out_dict(<dict>arg)
        else:
            return

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

def out_list(xs=[1,'a','c',4,5]):
    ext = PyExternal()
    ext.out(xs)

def out_dict(**kwargs):
    if not kwargs:
        kwargs = {'a':[1,2,'a'], 'b':1.3, 'c': 100, 'd':'e'}
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




# ============================================================================
# Max datastructure helper functions
# 
# Should ideally be refactored to cython extensions types.
# 
# ----------------------------------------------------------------------------
# table 

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
# buffer

cdef class Buffer:
    """A wrapper class for a Max t_buffer_obj
    """
    cdef mp.t_buffer_obj *obj
    cdef mp.t_buffer_ref *ref
    # cdef mx.t_object *tobj # t_object with ref to buffer
    cdef bint is_locked
    cdef float* samples

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
    cdef Buffer from_name(mx.t_object *x, mx.t_symbol *name):
        """Create a reference to a buffer~ object by name."""
        # Call to __new__ bypasses __init__ constructor
        cdef Buffer buffer = Buffer.__new__(Buffer)
        buffer.ref = mp.buffer_ref_new(x, name)
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
        """Query a buffer~ to find out how many channels are present in the buffer content."""
        return mp.buffer_getchannelcount(self.obj)

    @property
    def framecount(self):
        """Query a buffer~ to find out how many frames long the buffer content is in samples."""
        return mp.buffer_getframecount(self.obj)

    @property
    def samplerate(self):
        """Query a buffer~ to find out its native sample rate in samples per second."""
        return mp.buffer_getsamplerate(self.obj)

    @property
    def millisamplerate(self):
        """Query a buffer~ to find out its native sample rate in samples per millisecond."""
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
        """Release your claim on the buffer~ contents so that other objects may read/write to the buffer~."""
        mp.buffer_unlocksamples(self.obj)
        self.is_locked = False

# ----------------------------------------------------------------------------
# dictionary


cdef class Dictionary:
    """A wrapper class for a Max t_dictionary
    """
    cdef mx.t_dictionary *d

    def __cinit__(self):
        self.d = mx.dictionary_new()

    def __dealloc__(self):
        # De-allocate if not null
        if self.d is not NULL:
            mx.object_free(self.d)
            self.d = NULL

    def __setitem__(self, str key, object value): pass
    def __getitem__(self, str key): pass

    # cdef mx.t_dictionary* sprintf(self, char* fmt, ...):
    #     return mx.dictionary_sprintf(char* fmt, ...)

    cdef mx.t_max_err appendlong(self, mx.t_symbol* key, mx.t_atom_long value):
        return mx.dictionary_appendlong(self.d, key, value)

    cdef mx.t_max_err appendfloat(self, mx.t_symbol* key, double value):
        return mx.dictionary_appendfloat(self.d, key, value)

    cdef mx.t_max_err appendsym(self, mx.t_symbol* key, mx.t_symbol* value):
        return mx.dictionary_appendsym(self.d, key, value)

    cdef mx.t_max_err appendatom(self, mx.t_symbol* key, mx.t_atom* value):
        return mx.dictionary_appendatom(self.d, key, value)

    cdef mx.t_max_err appendstring(self, mx.t_symbol* key, const char* value):
        return mx.dictionary_appendstring(self.d, key, value)

    cdef mx.t_max_err appendatoms(self, mx.t_symbol* key, long argc, mx.t_atom* argv):
        return mx.dictionary_appendatoms(self.d, key, argc, argv)

    cdef mx.t_max_err appendatomarray(self, mx.t_symbol* key, mx.t_object* value):
        return mx.dictionary_appendatomarray(self.d, key, value)

    cdef mx.t_max_err appenddictionary(self, mx.t_symbol* key, mx.t_object* value):
       return mx.dictionary_appenddictionary(self.d, key, value)

    cdef mx.t_max_err appendobject(self, mx.t_symbol* key, mx.t_object* value):
        return mx.dictionary_appendobject(self.d, key, value)

    cdef mx.t_max_err appendobject_flags(self, mx.t_symbol* key, mx.t_object* value, long flags):
        return mx.dictionary_appendobject_flags(self.d, key, value, flags)

    cdef mx.t_max_err appendbinbuf(self, mx.t_symbol* key, void* value):
        return mx.dictionary_appendbinbuf(self.d, key, value)
 
    cdef mx.t_max_err getlong(self, mx.t_symbol* key, mx.t_atom_long* value):
        return mx.dictionary_getlong(self.d, key, value)

    cdef mx.t_max_err getfloat(self, mx.t_symbol* key, double* value):
        return mx.dictionary_getfloat(self.d, key, value)

    cdef mx.t_max_err getsym(self, mx.t_symbol* key, mx.t_symbol** value):
        return mx.dictionary_getsym(self.d, key, value)

    cdef mx.t_max_err getatom(self, mx.t_symbol* key, mx.t_atom* value):
        return mx.dictionary_getatom(self.d, key, value)

    cdef mx.t_max_err getstring(self, mx.t_symbol* key, const char** value):
        return mx.dictionary_getstring(self.d, key, value)

    cdef mx.t_max_err getatoms(self, mx.t_symbol* key, long* argc, mx.t_atom** argv):
        return mx.dictionary_getatoms(self.d, key, argc, argv)

    cdef mx.t_max_err getatoms_ext(self, mx.t_symbol* key, long stringstosymbols, long* argc, mx.t_atom** argv):
        return mx.dictionary_getatoms_ext(self.d, key, stringstosymbols, argc, argv)

    cdef mx.t_max_err copyatoms(self, mx.t_symbol* key, long* argc, mx.t_atom** argv):
        return mx.dictionary_copyatoms(self.d, key, argc, argv)

    cdef mx.t_max_err getatomarray(self, mx.t_symbol* key, mx.t_object** value):
        return mx.dictionary_getatomarray(self.d, key, value)

    cdef mx.t_max_err getdictionary(self, mx.t_symbol* key, mx.t_object** value):
        return mx.dictionary_getdictionary(self.d, key, value)

    cdef mx.t_max_err get_ex(self, mx.t_symbol* key, long* ac, mx.t_atom** av, char* errstr):
        return mx.dictionary_get_ex(self.d, key, ac, av, errstr)

    cdef mx.t_max_err getobject(self, mx.t_symbol* key, mx.t_object** value):
        return mx.dictionary_getobject(self.d, key, value)

    cdef bint has_string_value(self, mx.t_symbol* key):
        return mx.dictionary_entryisstring(self.d, key)

    cdef bint has_atomarray_value(self, mx.t_symbol* key):
        return mx.dictionary_entryisatomarray(self.d, key)

    cdef bint has_dictionary_value(self, mx.t_symbol* key):
        return mx.dictionary_entryisdictionary(self.d, key)

    cdef bint has_entry(self, mx.t_symbol* key):
        return mx.dictionary_hasentry(self.d, key)

    def getentrycount(self) -> long:
        return mx.dictionary_getentrycount(self.d)

    cdef mx.t_max_err getkeys(self, long* numkeys, mx.t_symbol*** keys):
        return mx.dictionary_getkeys(self.d, numkeys, keys)

    cdef mx.t_max_err getkeys_ordered(self, long* numkeys, mx.t_symbol*** keys):
        return mx.dictionary_getkeys_ordered(self.d, numkeys, keys)

    cdef void freekeys(self, long numkeys, mx.t_symbol** keys):
        mx.dictionary_freekeys(self.d, numkeys, keys)

    cdef mx.t_max_err deleteentry(self, mx.t_symbol* key):
         return mx.dictionary_deleteentry(self.d, key)

    cdef mx.t_max_err chuckentry(self, mx.t_symbol* key):
         return mx.dictionary_chuckentry(self.d, key)

    cdef mx.t_max_err clear(self):
         return mx.dictionary_clear(self.d)

    cdef mx.t_dictionary* clone(self):
        return mx.dictionary_clone(self.d)

    cdef mx.t_max_err clone_to_existing(self, mx.t_dictionary* dc):
        return mx.dictionary_clone_to_existing(self.d, dc)

    # cdef mx.t_max_err copy_to_existing(self, mx.t_dictionary* dc):
    #     return mx.dictionary_copy_to_existing(self.d, dc)

    cdef mx.t_max_err merge_to_existing(self, mx.t_dictionary* dc):
        return mx.dictionary_merge_to_existing(self.d, dc)

    cdef mx.t_max_err copy_nonunique_to_existing(self, mx.t_dictionary* dc):
        return mx.dictionary_copy_nonunique_to_existing(self.d, dc)

    # cdef mx.dictionary_funall(self, method fun, void* arg)
    #     mx.dictionary_funall(self, method fun, void* arg)

    cdef mx.t_symbol* entry_getkey(self, mx.t_dictionary_entry* x):
        return mx.dictionary_entry_getkey(x)

    cdef void entry_getvalue(self, mx.t_dictionary_entry* x, mx.t_atom* value):
        mx.dictionary_entry_getvalue(x, value)

    cdef void entry_getvalues(self, mx.t_dictionary_entry* x, long* argc, mx.t_atom** argv):
        mx.dictionary_entry_getvalues(x, argc, argv)

    cdef mx.t_max_err copyunique(self, mx.t_dictionary* copyfrom):
        return mx.dictionary_copyunique(self.d, copyfrom)

    cdef mx.t_max_err getdeflong(self, mx.t_symbol* key, mx.t_atom_long* value, mx.t_atom_long dfn):
        return mx.dictionary_getdeflong(self.d, key, value, dfn)

    cdef mx.t_max_err getdeffloat(self, mx.t_symbol* key, double* value, double dfn):
        return mx.dictionary_getdeffloat(self.d, key, value, dfn)

    cdef mx.t_max_err getdefsym(self, mx.t_symbol* key, mx.t_symbol** value, mx.t_symbol* dfn):
        return mx.dictionary_getdefsym(self.d, key, value, dfn)

    cdef mx.t_max_err getdefatom(self, mx.t_symbol* key, mx.t_atom* value, mx.t_atom* dfn):
        return mx.dictionary_getdefatom(self.d, key, value, dfn)

    cdef mx.t_max_err getdefstring(self, mx.t_symbol* key, const char** value, char* dfn):
        return mx.dictionary_getdefstring(self.d, key, value, dfn)

    cdef mx.t_max_err getdefatoms(self, mx.t_symbol* key, long* argc, mx.t_atom** argv, mx.t_atom* dfn):
        return mx.dictionary_getdefatoms(self.d, key, argc, argv, dfn)

    cdef mx.t_max_err copydefatoms(self, mx.t_symbol* key, long* argc, mx.t_atom** argv, mx.t_atom* dfn):
        return mx.dictionary_copydefatoms(self.d, key, argc, argv, dfn)

    cdef mx.t_max_err dump(self, long recurse, long console):
        return mx.dictionary_dump(self.d, recurse, console)

    cdef mx.t_max_err copyentries(self, mx.t_dictionary *dst, mx.t_symbol **keys):
        return mx.dictionary_copyentries(self.d, dst, keys)

    # cdef mx.t_dictionary* sprintf(self, char* fmt, ...):
    #     return mx.dictionary_sprintf(char* fmt, ...)

    cdef long transaction_lock(self):
        return mx.dictionary_transaction_lock(self.d)

    cdef long transaction_unlock(self):
        return mx.dictionary_transaction_unlock(self.d)

    # FIXME: add staticmethod as well
    cdef mx.t_max_err dictionary_read(self, const char* filename, short path, mx.t_dictionary** d):
        return mx.dictionary_read(filename, path, d)

    cdef mx.t_max_err dictionary_write(self, const char* filename, short path):
        return mx.dictionary_write(self.d, filename, path)



# ----------------------------------------------------------------------------
# numpy c-api import example

if INCLUDE_NUMPY:

    #@cython.boundscheck(False)
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
# test functions and variables

txt = "Hello MAX!"

greeting = 'Hello World'


def test_atom():
    ext = PyExternal()
    a1 = Atom.from_list([1, 2.5, b'hello', 'world'])
    ext.out(a1.to_list())


cpdef public str hello():
    return greeting

def random(int n):
    import random
    return random.randint(0, n)

def echo(*args):
    return args

def total(*args):
    return sum(args)

