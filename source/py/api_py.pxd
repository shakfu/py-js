# api_py.pxd

from cpython.ref cimport PyObject
cimport api_max as mx

cdef extern from "py.h":
    cdef int PY_MAX_ATOMS
    
    int py_global_obj_count
    mx.t_hashtab* py_global_registry
    
    ctypedef struct t_py:
        # mx.t_object p_ob
        mx.t_symbol* p_name
        mx.t_symbol* p_pythonpath
        mx.t_bool p_debug
        PyObject* p_globals
        mx.t_patcher* p_patcher
        mx.t_box* p_box
        mx.t_object* p_code_editor
        char** p_code
        long p_code_size
        mx.t_symbol* p_code_filepath
        mx.t_bool p_autoload
        void* p_outlet_right
        void* p_outlet_middle
        void* p_outlet_left
    
    cdef void py_log(t_py* x, char* fmt, ...)
    cdef void py_error(t_py* x, char* fmt, ...)

    cdef void py_bang(t_py *x)

    cdef void py_scan(t_py* x)
    cdef void py_send(t_py* x, mx.t_symbol* s, long argc, mx.t_atom* argv)
    cdef void py_lookup(t_py* x, mx.t_symbol* s)

    cdef void py_read(t_py* x, mx.t_symbol* s)

    cdef void py_send_from_seq(t_py* x, PyObject* seq)

    cdef mx.t_atom* py_list_to_atom(t_py* x, PyObject* plist);
    cdef void py_list_to_atom2(t_py* x, PyObject* plist, long* argc, mx.t_atom** argv);