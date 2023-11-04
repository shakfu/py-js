# api_py.pxd

from cpython.ref cimport PyObject
cimport api_max as mx

from libc.stdint cimport uintptr_t

cdef extern from "py.h":

    cdef int PY_MAX_ATOMS

    ctypedef struct t_py

    cdef void py_info(t_py* x, char* fmt, ...)
    cdef void py_debug(t_py* x, char* fmt, ...)
    cdef void py_error(t_py* x, char* fmt, ...)

    # api module helpers
    cdef mx.t_hashtab* py_get_global_registry()
    cdef uintptr_t py_get_object_ref()

    # Path helpers

    cdef mx.t_symbol* py_locate_path_to_external(t_py* x)
    cdef mx.t_max_err py_locate_path_from_symbol(t_py* x, mx.t_symbol* s)

    # Side-effect helpers

    cdef void py_bang(t_py* x)
    cdef void py_bang_success(t_py* x)
    cdef void py_bang_failure(t_py* x)
    cdef void* get_outlet(t_py* x)

    # Core Method Helpers

    cdef mx.t_max_err py_eval_text(t_py* x, long argc, mx.t_atom* argv, int offset)

    # Common handlers

    cdef void py_handle_error(t_py* x, char* fmt, ...)
    cdef mx.t_max_err py_handle_float_output(t_py* x, PyObject* pval)
    cdef mx.t_max_err py_handle_long_output(t_py* x, PyObject* pval)
    cdef mx.t_max_err py_handle_string_output(t_py* x, PyObject* pval)
    cdef mx.t_max_err py_handle_list_output(t_py* x, PyObject* pval)
    cdef mx.t_max_err py_handle_dict_output(t_py* x, PyObject* pval)
    cdef mx.t_max_err py_handle_output(t_py* x, PyObject* pval)

    # Core Python Methods

    cdef mx.t_max_err py_import(t_py* x, mx.t_symbol* s)
    cdef mx.t_max_err py_eval(t_py* x, mx.t_symbol* s, long argc, mx.t_atom* argv)
    cdef mx.t_max_err py_exec(t_py* x, mx.t_symbol* s, long argc, mx.t_atom* argv)
    cdef mx.t_max_err py_execfile(t_py* x, mx.t_symbol* s)

    # Extra Python Methods

    cdef mx.t_max_err py_assign(t_py* x, mx.t_symbol* s, long argc, mx.t_atom* argv)
    cdef mx.t_max_err py_call(t_py* x, mx.t_symbol* s, long argc, mx.t_atom* argv)
    cdef mx.t_max_err py_code(t_py* x, mx.t_symbol* s, long argc, mx.t_atom* argv)
    cdef mx.t_max_err py_pipe(t_py* x, mx.t_symbol* s, long argc, mx.t_atom* argv)
    cdef mx.t_max_err py_anything(t_py* x, mx.t_symbol* s, long argc, mx.t_atom* argv)

    # Information Methods

    cdef void py_metadata(t_py* x)
    cdef void py_count(t_py* x)
    cdef void py_assist(t_py* x, void* b, long m, long a, char* s)
    cdef void py_appendtodict(t_py* x, mx.t_dictionary* dict)

    # Time-based Methods

    cdef mx.t_max_err py_task(t_py* x)
    cdef mx.t_max_err py_sched(t_py* x, mx.t_symbol* s, long argc, mx.t_atom* argv)

    # Interobject Methods

    cdef mx.t_max_err py_send(t_py* x, mx.t_symbol* s, long argc, mx.t_atom* argv)
    cdef void py_scan(t_py* x)
    cdef long py_scan_callback(t_py* x, mx.t_object* obj)

    # Code editor Methods

    cdef void py_read(t_py* x, mx.t_symbol* s)
    cdef void py_load(t_py* x, mx.t_symbol* s) # read(f) -> execfile(f)
    cdef void py_doread(t_py* x, mx.t_symbol* s, long argc, mx.t_atom* argv)
    cdef void py_dblclick(t_py* x)
    cdef void py_run(t_py* x)
    cdef void py_edclose(t_py* x, char** text, long size)
    cdef mx.t_max_err py_edsave(t_py* x, char** text, long size)

    # Datastructure support methods

    # table
    cdef bint py_table_exists(t_py* x, char* table_name)
    cdef mx.t_max_err py_list_to_table(t_py* x, char* table_name, PyObject* plist)
    cdef PyObject* py_table_to_list(t_py* x, char* table_name)
