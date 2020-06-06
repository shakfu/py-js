cimport api_max as mx # api is a cython keyword!
cimport api_py as px

import numpy
import numpy as np



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



cdef class PyExternal:
    cdef px.t_py *obj

    def __cinit__(self, bytes name=b'__main__'):
        self.obj = <px.t_py *>mx.object_findregistered(
            mx.CLASS_BOX, mx.gensym(name))

    cpdef bang(self):
        px.py_bang(self.obj)

    # cpdef send(self, str msg, list args):

    #     mx.object_method_typed(self.obj, msg_sym, argc, argv, NULL);


def test():
    ext = PyExternal()
    ext.bang()





