from api cimport *

cdef extern from "py.h":
    cdef int PY_MAX_ATOMS
    # cdef char *PY_NAME
    cdef char *PY_NAMESPACE

    ctypedef struct t_py

    cdef void py_bang(t_py *x)
    cdef void py_import(t_py *x, t_symbol *s)
    cdef void py_eval(t_py *x, t_symbol *s, long argc, t_atom *argv)
    cdef void py_exec(t_py *x, t_symbol *s, long argc, t_atom *argv)
    cdef void py_execfile(t_py *x, t_symbol *s, long argc, t_atom *argv)
    cdef void py_run(t_py *x, t_symbol *s, long argc, t_atom *argv)
    cdef void py_dblclick(t_py *x)
    cdef void *py_new(t_symbol *s, long argc, t_atom *argv)
    cdef void py_free(t_py *x)
    cdef void py_init(t_py *x)




txt = 'Hello from Max!'

greeting = 'Hello World'

# name = lambda: getattr(globals(), 'PY_NAME')

cpdef public str hello():
    return greeting

cpdef public str py_post(str s):
    post(s.encode('utf-8'))
    return s


cdef class PyExternal:
    cdef t_py *obj

    def __cinit__(self, bytes name):
        self.obj = <t_py *>object_findregistered(
            gensym(PY_NAMESPACE),
            gensym(name))

    cpdef bang(self):
        py_bang(self.obj)


def test():
    key = 'PY_NAME'
    if key in globals():
        s = globals()[key]
        ext = PyExternal(bytes(s))
        ext.bang()
    return 'nope'
        # return __MAXMSP__NAME






