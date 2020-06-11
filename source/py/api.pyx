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


from cpython cimport PyFloat_AsDouble
from cpython cimport PyLong_AsLong
from cpython.ref cimport PyObject

from libc.stdlib cimport malloc
from libc.string cimport strcpy, strlen

cimport api_max as mx # api is a cython keyword!
cimport api_py as px

import numpy
import numpy as np


DEF MAX_CHARS = 32767
DEF PY_MAX_ATOMS = 128


cdef extern from "Python.h":
    const char* PyUnicode_AsUTF8(object unicode)
    unicode PyUnicode_FromString(const char *u)



cdef class PyAtom:
    """A wrapper class for max t_atom arrays"""
    cdef long argc
    cdef mx.t_atom *argv
    cdef bint ptr_owner
    cdef char is_allocated

    def __cinit__(self):
        self.argc = 0
        self.argv = NULL
        self.ptr_owner = False
        self.is_allocated = 0

    def __dealloc__(self):
        """De-allocate if not null and flag is set"""
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
        cparsestring = PyUnicode_AsUTF8(parsestr)
        cdef mx.t_max_err err = mx.atom_setparse(&self.argc, &self.argv, cparsestring)
        if err != mx.MAX_ERR_NONE: # test this!!
            raise Exception("cannot convert c parsestring to atom array")

    cpdef str to_pstr(self):
        """atoms -> python string"""
        cdef long textsize = 0
        cdef char* text = NULL
        cdef mx.t_max_err err = mx.atom_gettext(self.argc, self.argv, &textsize, &text, 
            mx.OBEX_UTIL_ATOM_GETTEXT_DEFAULT)
        pstr = PyUnicode_FromString(text)
        mx.sysmem_freeptr(text)
        return pstr

    @staticmethod
    cdef PyAtom from_atom(long argc, mx.t_atom *argv, bint owner=False):
        """Factory function to create PyAtom objects from t_atom pointer.

        Setting `owner` flag to `True` causes the extension type to 
        `free` the structure pointed to by `argv` when the wrapper 
        object is deallocated.
        """
        # Call to __new__ bypasses __init__ constructor
        cdef PyAtom instance = PyAtom.__new__(PyAtom)
        instance.argc = argc
        instance.argv = argv
        instance.ptr_owner = owner
        return instance

    @staticmethod
    cdef PyAtom new(long argc = 0):
        """Factory function to create PyAtom objects with
        newly allocated t_atom"""
        # cdef long argc = 1
        cdef mx.t_atom *argv
        argv = <mx.t_atom *>mx.sysmem_newptr(sizeof(mx.t_atom *) * argc)
        if argv is NULL:
            raise MemoryError
        return PyAtom.from_atom(argc, argv, owner=True)


    @staticmethod
    cdef PyAtom from_list(list elements):
        """Factory function to create PyAtom objects from a python list
        """
        cdef long argc = <long>len(elements)
        cdef mx.t_atom *argv
        argv = <mx.t_atom *>mx.sysmem_newptr(sizeof(mx.t_atom *) * argc)
        for i, elem in enumerate(elements):
            if type(elem) == float:
                mx.atom_setfloat(&argv[i], <double>elem)
            elif type(elem) == int:
                mx.atom_setlong((&argv[i]), <long>elem)
            elif type(elem) == str:
                mx.atom_setsym((&argv[i]), mx.gensym(elem.encode('utf-8')))
            else:
                continue
        return PyAtom.from_atom(argc, argv, owner=True)

# cdef log(px.t_py *obj, str s):
#     mx.object_post(<mx.t_object *>self.obj, s.encode('utf-8'))
#     # mx.post(s.encode('utf-8'))

# cdef error(px.t_py *obj, str s):
#     mx.object_error(<mx.t_object *>self.obj, s.encode('utf-8'))


cdef class PyExternal:
    cdef px.t_py *obj


    def __cinit__(self, bytes name=b'__main__'):
        self.obj = <px.t_py *>mx.object_findregistered(
            mx.CLASS_BOX, mx.gensym(name))

    cpdef bang(self):
        px.py_bang(self.obj)

    def log(self, str s):
        px.py_log(self.obj, s.encode('utf-8'))

    def error(self, str s): # FIX: name collision with error in ext.h
        px.py_error(self.obj, s.encode('utf-8'))

    # CRITICAL: STILL CRASHING, works but then crashes!!
    cdef send(self, str name, list args):
        _args = [name] + args
        cdef PyAtom atom = PyAtom.from_list(_args)
        # msg = "send".encode('utf-8')
        px.py_send(self.obj, mx.gensym("list"), atom.argc, atom.argv)
        # mx.sysmem_freeptr(atom.argv)

    cdef send2(self, str name, list args):
        _args = [name] + args
        px.py_send_from_seq(self.obj, <PyObject*>_args)

    # CRITICAL: STILL CRASHING, works but then crashes!!
    # cdef send3(self, str name, list args):
    #     _args = [name] + args
    #     cdef long argc = <long>len(_args)
    #     cdef mx.t_atom* argv = px.py_list_to_atom(self.obj, <PyObject*>_args)
    #     px.py_send(self.obj, mx.gensym("list"), argc, argv)
    #     # mx.sysmem_freeptr(argv)



    cdef scan(self):
        px.py_scan(self.obj)

    cdef lookup(self, str name):
        cdef mx.t_hashtab* registry = px.get_global_registry()
        cdef mx.t_object* obj = NULL
        cdef mx.t_max_err err

        if (mx.hashtab_getsize(registry) == 0):
            self.error("registry not populated")
            return

        err = mx.hashtab_lookup(registry, 
            mx.gensym(name.encode('utf-8')), &obj)

        if ((err != mx.MAX_ERR_NONE) or (obj == NULL)):
            self.error("no object found with name")
        else:
            self.log("found object")


    cdef send4(self, str name, str msg, list args):
        cdef mx.t_object* obj = NULL
        # cdef char* obj_name = NULL
        cdef mx.t_symbol* msg_sym = NULL
        cdef mx.t_hashtab* registry = px.get_global_registry()
        cdef mx.t_max_err err

        # obj_name = name.encode('utf-8')
        msg_sym = mx.gensym(msg.encode('utf-8'))

        cdef mx.t_atom argv[PY_MAX_ATOMS]
        cdef long argc = <long>len(args)

        if argc < 1:
            self.error("no arguments given")
            return

        if argc >= PY_MAX_ATOMS:
            self.error("number of args exceeded app limit")
            return

        for i, elem in enumerate(args):
            if type(elem) == float:
                mx.atom_setfloat(&argv[i], <double>elem)
            elif type(elem) == int:
                mx.atom_setlong((&argv[i]), <long>elem)
            elif type(elem) == str:
                mx.atom_setsym((&argv[i]), mx.gensym(elem.encode('utf-8')))
            else:
                continue

        # if registry is empty, scan it
        if (mx.hashtab_getsize(registry) == 0):
            self.log("registry empty, scanning...")
            self.scan()

        # lookup name in registry
        err = mx.hashtab_lookup(registry, mx.gensym(name.encode('utf-8')), &obj)

        if ((err != mx.MAX_ERR_NONE) or (obj == NULL)):
            self.error("no object found with name")
            return

        err = mx.object_method_typed(obj, msg_sym, argc, argv, NULL)

        if (err != mx.MAX_ERR_NONE):
            # fail
            self.error("send failed")
            mx.outlet_bang(<void*>self.obj.p_outlet_middle)
        else:
            # success
            self.log("send succeeded")
            mx.outlet_bang(<void*>self.obj.p_outlet_right)


    cdef success(self):
        mx.outlet_bang(<void*>self.obj.p_outlet_right)

    cdef fail(self):
        mx.outlet_bang(<void*>self.obj.p_outlet_middle)

    cdef out_sym(self, str arg):
        mx.outlet_anything(<void*>self.obj.p_outlet_left, 
            mx.gensym(arg.encode('utf-8')), 0, NULL)

    cdef out_float(self, float arg):
        mx.outlet_float(<void*>self.obj.p_outlet_left, <double>arg)

    cdef out_int(self, int arg):
        mx.outlet_int(<void*>self.obj.p_outlet_left, <long>arg)

    cdef out_list(self, list arg):
        cdef PyAtom atom = PyAtom.from_list(arg)
        mx.outlet_list(<void*>self.obj.p_outlet_left, mx.gensym("list"),
            atom.argc, atom.argv)

    cdef out(self, object arg):
        if isinstance(arg, float): self.out_float(arg)
        elif isinstance(arg, int): self.out_int(arg)
        elif isinstance(arg, str): self.out_sym(arg)
        elif isinstance(arg, list): self.out_list(arg)
        else:
            return

    #  mx.object_method_typed(self.obj, mx.gensym(msg), argc, argv, NULL)
    #  t_max_err object_method_parse(t_object *x, t_symbol *s, const char *parsestr, t_atom *rv)





def test():
    ext = PyExternal()
    ext.bang()

def success():
    ext = PyExternal()
    ext.success()

def fail():
    ext = PyExternal()
    ext.fail()

def out_sym():
    ext = PyExternal()
    ext.out('hello outlet!')

def out_int():
    ext = PyExternal()
    ext.out(100)

def out_float():
    ext = PyExternal()
    ext.out(12.75)

def out_list():
    ext = PyExternal()
    ext.out([1,2,3,4,5])

def sendtest(name, value=11.5):
    ext = PyExternal()
    ext.send(name, [value])
    # del ext

def sendtest2(name, value=12.5):
    ext = PyExternal()
    ext.send2(name, [value])
    # del ext

def sendtest3(name, value=13.5):
    ext = PyExternal()
    ext.send3(name, [value])
    # del ext

def sendtest4(name, msg='float', value=14.5):
    ext = PyExternal()
    ext.send4(name, msg, [value])
    # del ext

def lookup(name):
    ext = PyExternal()
    ext.lookup(name)



# ext = PyExternal()

txt = "Hey MAX!"

greeting = 'Hello World'


cpdef public str hello():
    return greeting


def random(int n):
    return np.random.rand(n)


def echo(*args):
    return args


def total(*args):
    return sum(args)


# def post(str s):
#     mx.post(s.encode('utf-8'))


# def error(str s):
#     mx.error(s.encode('utf-8'))




