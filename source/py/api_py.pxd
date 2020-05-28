# api_py.pxd

cimport api_max as mx

cdef extern from "py.h":
    cdef int PY_MAX_ATOMS
    cdef char *PY_NAMESPACE
    ctypedef struct t_py
    cdef void py_bang(t_py *x)
    cdef void py_import(t_py *x, mx.t_symbol *s)
    cdef void py_eval(t_py *x, mx.t_symbol *s, long argc, mx.t_atom *argv)
    cdef void py_exec(t_py *x, mx.t_symbol *s, long argc, mx.t_atom *argv)
    cdef void py_execfile(t_py *x, mx.t_symbol *s, long argc, mx.t_atom *argv)
    cdef void py_run(t_py *x, mx.t_symbol *s, long argc, mx.t_atom *argv)
    cdef void py_dblclick(t_py *x)
    cdef void *py_new(mx.t_symbol *s, long argc, mx.t_atom *argv)
    cdef void py_free(t_py *x)
    cdef void py_init(t_py *x)
