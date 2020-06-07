# api.pyx
"""
From the cython docs:

There are two kinds of function definition in Cython:

1. Python functions are defined using the def statement, as in Python. 
   They take Python objects as parameters and return Python objects.

2. C functions are defined using the new cdef statement.
   They take either Python objects or C values as parameters, and can
   return either Python objects or C values.

Because of this limitation, you can use extension types to wrap arbitrary
C data structures and provide a Python-like interface to them.
"""

from cpython cimport unicode
#from cpython.unicode import PyUnicode_FromString

cimport api_max as mx # api is a cython keyword!
cimport api_py as px

import numpy
import numpy as np


DEF MAX_CHARS = 32767





txt = "Hey MAX!"

greeting = 'Hello World'

# name = lambda: getattr(globals(), 'PY_NAME')


cpdef public str hello():
    return greeting


def random(int n):
    return np.random.rand(n)


def echo(*args):
    return args


def total(*args):
    return sum(args)


def post(str s):
    mx.post(s.encode('utf-8'))


def error(str s):
    mx.error(s.encode('utf-8'))


cdef class PyAtom:
    """A wrapper class for max t_atom arrays"""
    cdef long argc
    cdef mx.t_atom *argv
    cdef bint ptr_owner

    def __cinit__(self):
        self.argc = 0
        self.argv = NULL
        self.ptr_owner = False

    def __dealloc__(self):
        # De-allocate if not null and flag is set
        if self.argv is not NULL and self.ptr_owner is True:
            mx.sysmem_freeptr(self.argv)
            self.argc = 0
            self.argv = NULL


    cdef int from_cstr(self, char *parsestr) except -1:
        cdef mx.t_max_err err = mx.atom_setparse(&self.argc, &self.argv, parsestr)
        if err != mx.MAX_ERR_NONE: # test this!!
            raise Exception("cannot convert c parsestring to atom array")
        else:
            return 0

    cpdef from_pstr(self, str parsestr):
        cdef char cparsestring[MAX_CHARS]
        cparsestring = unicode.PyUnicode_AsUTF8(parsestr)
        cdef mx.t_max_err err = mx.atom_setparse(&self.argc, &self.argv, cparsestring)
        if err != mx.MAX_ERR_NONE: # test this!!
            raise Exception("cannot convert c parsestring to atom array")

    cpdef str to_pstr(self):
        """ atoms -> python string """
        cdef long textsize = 0
        cdef char* text = NULL
        cdef mx.t_max_err err = mx.atom_gettext(self.argc, self.argv, &textsize, &text, 
            mx.OBEX_UTIL_ATOM_GETTEXT_DEFAULT)
        pstr = unicode.PyUnicode_FromString(text)
        mx.sysmem_freeptr(text)
        return pstr


cdef class PyExternal:
    cdef px.t_py *obj

    def __cinit__(self, bytes name=b'__main__'):
        self.obj = <px.t_py *>mx.object_findregistered(
            mx.CLASS_BOX, mx.gensym(name))

    cpdef bang(self):
        px.py_bang(self.obj)

    # cpdef send(self, bytes msg, list args):
    #     cdef int length = len(args)

    #     mx.object_method_typed(self.obj, mx.gensym(msg), argc, argv, NULL)
        #t_max_err object_method_parse(t_object *x, t_symbol *s, const char *parsestr, t_atom *rv)


def test():
    ext = PyExternal()
    ext.bang()





