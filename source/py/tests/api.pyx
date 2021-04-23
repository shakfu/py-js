# api.pyx

#cimport cython
from cpython cimport PyFloat_AsDouble
from cpython cimport PyLong_AsLong
from cpython.ref cimport PyObject

from libc.stdlib cimport malloc
from libc.string cimport strcpy, strlen

cimport api_max as mx # api is a cython keyword!

DEF MAX_CHARS = 32767
DEF PY_MAX_ATOMS = 128

cdef extern from "Python.h":
    const char* PyUnicode_AsUTF8(object unicode)
    unicode PyUnicode_FromString(const char *u)

def post(str s):
    mx.post(s.encode('utf-8'))


def error(str s):
    mx.error(s.encode('utf-8'))

cpdef public str hello():
    return greeting

def echo(*args):
    return args

def total(*args):
    return sum(args)

txt = "Hello MAX!"

greeting = 'Hello World'


