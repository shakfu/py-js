# api.pyx
"""

The main place to create wrappers and utilities to access max's api

See below for examples of this.

- [x] mx.object_method_typed(self.obj, mx.gensym(msg), argc, argv, NULL)
- [ ] t_max_err object_method_parse(t_object *x, t_symbol *s, const char *parsestr, t_atom *rv)


"""
#cimport cython
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
            for i in range(self.argc):
                mx.sysmem_freeptr(&self.argv[i])
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


cdef class PyExternal:
    cdef px.t_py *obj
    cdef bytes name

    def __cinit__(self):
        """Retrieves the py object name and reference.

        PY_OBJ_NAME is set to __builtins__ at object creation
        making it available to all modules.

        Since all py objects are registered, knowing the name
        allows any module in the namespace to get a reference
        (as below) to its parent object (-:
        """
        PY_OBJ_NAME = getattr(__builtins__, 'PY_OBJ_NAME')
        self.name = PY_OBJ_NAME.encode('utf-8')
        self.obj = <px.t_py *>mx.object_findregistered(
            mx.CLASS_BOX, mx.gensym(self.name))

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

        err = mx.hashtab_lookup(registry, 
            mx.gensym(name.encode('utf-8')), &obj)

        if ((err != mx.MAX_ERR_NONE) or (obj == NULL)):
            self.error("no object found with name")
        else:
            self.log("found object")

    # Wooo!!!
    cdef send0(self, str name, list args):
        _args = [name] + args
        cdef PyAtom atom = PyAtom.from_list(_args)
        px.py_send(self.obj, mx.gensym(""), atom.argc, atom.argv)

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
                mx.atom_setsym((&argv[i]), mx.gensym(elem.encode('utf-8')))
            else:
                continue

        px.py_send(self.obj, mx.gensym(""), argc, argv)


    cdef send4(self, str name, str msg, list args):
        cdef mx.t_object* obj = NULL
        cdef mx.t_symbol* msg_sym = mx.gensym(msg.encode('utf-8'))
        cdef mx.t_hashtab* registry = px.get_global_registry()
        cdef mx.t_max_err err
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
            self.error("send failed")
            mx.outlet_bang(<void*>self.obj.p_outlet_middle)
        else:
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

    cdef out_dict(self, dict arg):
        "note: not recursive... still cannot deal with dict inside dict"
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
        # BUG: below cause crash? not sure why
        elif isinstance(arg, dict): self.out_dict(<dict>arg)
        else:
            return

def get_globals():
    return list(globals().keys())

def test():
    ext = PyExternal()
    ext.bang()

def success():
    ext = PyExternal()
    ext.success()

def fail():
    ext = PyExternal()
    ext.fail()

def out_sym(s='hello outlet!'):
    ext = PyExternal()
    ext.out(s)

def out_int(n=100):
    ext = PyExternal()
    ext.out(n)

def out_float(n=12.75):
    ext = PyExternal()
    ext.out(n)

def out_list(xs=[1,2,3,4,5]):
    ext = PyExternal()
    ext.out(xs)

def out_dict(d={'a':[1,2,'a'], 'b':1.3, 'c': 100, 'd':'e'}):
    ext = PyExternal()
    ext.out_dict(d)

def out_dict2():
    d={'a':[1,2,'a'], 'b':1.3, 'c': 100, 'd':'e'}
    ext = PyExternal()
    ext.out(d)

def test_send0(name, value=9.5):
    ext = PyExternal()
    ext.send(name, [value])

def test_send(name, value=11.5):
    ext = PyExternal()
    ext.send(name, [value])
    # del ext

def test_send4(name, msg='float', value=14.5):
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


def post(str s):
    mx.post(s.encode('utf-8'))


def error(str s):
    mx.error(s.encode('utf-8'))




