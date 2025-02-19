# api_jit.pxd
"""
'x' -> the header has been fully exposed to cython

'-' -> the header is not explicitly exposed to cython and presently
       not required for the external. It is exposed to non-cython c code
       via the primary includes in "ext.h"

'p' -> partial analyzed but not yet included in api_msp.pxd

' ' -> an empty box means it is planned

- [ ] max_types.h
- [ ] ext_mess.h
- [ ] jit.error.h
- [ ] jit.max.h
- [ ] jit.common.h
- [ ] jit.cpost.h
- [ ] jit.critical.h
- [ ] jit.math.h
- [ ] max.jit.mop.h


"""

cdef extern from "max_types.h":
    ctypedef unsigned int t_uint            # an unsigned int as defined by the architecture / platform  @ingroup misc
    ctypedef char t_int8                    # a 1-byte int  @ingroup misc
    ctypedef unsigned char t_uint8          # an unsigned 1-byte int  @ingroup misc
    ctypedef short t_int16                  # a 2-byte int  @ingroup misc
    ctypedef unsigned short t_uint16        # an unsigned 2-byte int  @ingroup misc
    ctypedef int t_int32                    # a 4-byte int  @ingroup misc
    ctypedef unsigned int t_uint32          # an unsigned 4-byte int  @ingroup misc
    ctypedef long long t_int64              # an 8-byte int  @ingroup misc
    ctypedef unsigned long long t_uint64    # an unsigned 8-byte int  @ingroup misc
    ctypedef t_uint32 t_fourcc              # an integer of suitable size to hold a four char code / identifier  @ingroup misc

    ctypedef unsigned long long t_ptr_uint  # an unsigned pointer-sized int  @ingroup misc
    ctypedef long long t_ptr_int            # a pointer-sized int  @ingroup misc
    ctypedef double t_atom_float            # the type that is an A_FLOAT in a #t_atom  @ingroup misc
    ctypedef t_ptr_uint t_getbytes_size     # like size_t but for getbytes()  @ingroup misc
    ctypedef t_ptr_int t_int                # an integer  @ingroup misc
    ctypedef t_ptr_uint t_ptr_size          # unsigned pointer-sized value for counting (like size_t)  @ingroup misc
    ctypedef t_ptr_int t_atom_long          # the type that is an A_LONG in a #t_atom  @ingroup misc
    ctypedef t_atom_long t_max_err          # an integer value suitable to be returned as an error code  @ingroup misc
    ctypedef char **t_handle                # a handle (address of a pointer)  @ingroup misc
    ctypedef char *t_ptr                    # a pointer  @ingroup misc
    ctypedef t_uint8 t_bool                 # a true/false variable  @ingroup misc
    ctypedef t_int16 t_filepath             # i.e. path/vol in file APIs identifying a folder  @ingroup misc


cdef extern from "ext_mess.h":
    ctypedef void *(*method)(void *)

    ctypedef struct t_symbol:
        # The symbol.
        char *s_name            # name: a c-string
        t_object *s_thing       # possible binding to a t_object

    ctypedef struct t_messlist:
        # A list of symbols and their corresponding methods complete with typechecking information. 
        t_symbol *m_sym         # Name of the message
        method m_fun            # Method associated with the message
        char m_type[81]         # Argument type information

    ctypedef struct t_class
    ctypedef struct t_outlet
    ctypedef struct t_inlet

    ctypedef struct t_object:
        # The structure for the head of any object which wants to have inlets or outlets or support attributes.
        t_messlist *o_messlist  # list of messages and methods. The -1 entry of the message list of an object contains a pointer to its #t_class entry.
                                # (also used as freelist link)
        t_inlet *o_inlet        # list of inlets
        t_outlet *o_outlet      # list of outlets

    ctypedef union word:
        t_atom_long w_long
        t_atom_float w_float
        t_symbol *w_sym
        #object *w_obj

    ctypedef struct t_atom:
        # An atom is a typed datum.
        short a_type
        word a_w


cdef extern from "jit.error.h":
    ctypedef t_atom_long t_jit_err


cdef extern from "jit.max.h":
    ctypedef t_object    t_jit_object       # object header @ingroup jitter
    ctypedef t_class     t_max_class
    ctypedef t_object    t_max_object
    ctypedef t_messlist  t_max_messlist

    void *max_jit_object_alloc(t_class *mclass, t_symbol *jitter_classname)
    void max_jit_object_free(void *x)
    void max_jit_class_obex_setup(t_class *mclass, long oboffset)
    t_jit_err max_jit_class_addattr(t_class *mclass, void *attr)
    void max_jit_class_wrap_standard(t_class *mclass, t_class *jclass, long flags)
    void max_jit_class_wrap_addmethods(t_class *mclass, t_class *jclass)
    void max_jit_class_wrap_addmethods_flags(t_class *mclass, t_class *jclass, long flags)
    void max_jit_class_wrap_ob3d_inletinfo(t_class *mclass, t_class *jclass, long flags)
    void max_jit_class_wrap_attrlist2methods(t_class *mclass, t_class *jclass)
    void max_jit_class_addmethod_defer(t_class *mclass, method m, char *s)
    void max_jit_class_addmethod_defer_low(t_class *mclass, method m, char *s)
    void max_jit_class_addmethod_usurp(t_class *mclass, method m, char *s)
    void max_jit_class_addmethod_usurp_low(t_class *mclass, method m, char *s) 

    # instance specific wrapping
    t_jit_err max_jit_object_addattr(t_object *x, void *attr)
    void max_jit_object_wrap_standard(t_object *mob, t_object *job, long flags)
    void max_jit_object_wrap_complete(t_object *mob, t_object *job, long flags)
    void max_jit_object_wrap_addmethods(t_object *mob, t_object *job)
    void max_jit_object_wrap_addmethods_flags(t_object *mob, t_object *job, long flags)
    void max_jit_object_wrap_attrlist2methods(t_object *mob, t_object *job)
    void max_jit_object_addmethod_defer(t_object *x, method m, char *s)
    void max_jit_object_addmethod_defer_low(t_object *x, method m, char *s)
    void max_jit_object_addmethod_usurp(t_object *x, method m, char *s)
    void max_jit_object_addmethod_usurp_low(t_object *x, method m, char *s)
        
    void max_jit_object_attr_dump(void *x)
    long max_jit_attr_args_offset(short ac, t_atom *av)
    void max_jit_attr_args(void *x, short ac, t_atom *av)
    void max_jit_attr_set(void *x, t_symbol *s, short ac, t_atom *av)
        
    void *max_jit_obex_attrlist_get(void *x)
    t_jit_err max_jit_obex_attr_set(void *x, t_symbol *s, long ac, t_atom *av)
    t_jit_err max_jit_obex_attr_get(void *x, t_symbol *s, long *ac, t_atom **av)
    void max_jit_obex_attr_getdump(void *x, t_symbol *s, short argc, t_atom *argv)
    t_jit_err max_jit_obex_set(void *x, void *p)
    void *max_jit_obex_get(void *x)
    void *max_jit_obex_jitob_get(void *x)
    void max_jit_obex_jitob_set(void *x, void *jitob)
    void *max_jit_obex_usurplist_get(void *x)
    void max_jit_obex_usurplist_set(void *x, void *usurplist)
    void *max_jit_obex_proxylist_get(void *x)
    void max_jit_obex_proxylist_set(void *x, void *proxylist)
    long max_jit_obex_inletnumber_get(void *x)
    void max_jit_obex_inletnumber_set(void *x, long inletnumber)
    t_jit_err max_jit_obex_proxy_new(void *x, long c)
    t_jit_err max_jit_obex_proxy_resize(void *x, long count)
    t_jit_err max_jit_obex_proxy_deletetail(void *x)
    t_jit_err max_jit_obex_proxy_append(void *x, long c)
    void max_jit_obex_dumpout_set(void *x, void *outlet)
    void *max_jit_obex_dumpout_get(void *x)
    void max_jit_obex_dumpout(void *x, t_symbol *s, short argc, t_atom *argv)
    void *max_jit_obex_adornmentlist_get(void *x)
    void max_jit_obex_adornmentlist_set(void *x, void *adornmentlist)
    void *max_jit_obex_adornment_get(void *x, t_symbol *classname)
    t_jit_err max_jit_obex_addadornment(void *x,void *adornment)
    void max_jit_obex_gimmeback(void *x, t_symbol *s, long ac, t_atom *av)
    void max_jit_obex_gimmeback_dumpout(void *x, t_symbol *s, long ac, t_atom *av)

    t_atom_long max_jit_method_is_attr(void *x, t_symbol *s)
    t_atom_long max_jit_method_is_undocumented(void *x, t_symbol *s)
    t_atom_long max_jit_method_is_groupreference(void *x, t_symbol *s)

    long max_jit_getqueuestate()


cdef extern from "jit.common.h":
    ctypedef struct t_jit_attr:
        t_jit_object    ob             # common object header
        t_symbol        *name          # attribute name
        t_symbol        *type          # attribute type (char, long, float32, float64, symbol, atom, or obj)
        long            flags          # flags for public/private get/set methods
        method          get            # override default get method
        method          set            # override default set method
        void            *filterget     # filterobject for get method
        void            *filterset     # filterobject for set method
        void            *reserved      # for future use

    ctypedef enum:
        JIT_MATRIX_MAX_DIMCOUNT   = 32 # maximum dimension count
        JIT_MATRIX_MAX_PLANECOUNT = 32 # maximum plane count

    ctypedef struct t_jit_matrix_info:
        # Matrix information struct. Used to get/set multiple matrix attributes at once.
        long            size           # in bytes (0xFFFFFFFF=UNKNOWN)
        t_symbol        *type          # primitifve type (char, long, float32, or float64)
        long            flags          # flags to specify data reference, handle, or tightly packed
        long            dimcount       # number of dimensions
        long            dim[JIT_MATRIX_MAX_DIMCOUNT]       # dimension sizes
        long            dimstride[JIT_MATRIX_MAX_DIMCOUNT] # stride across dimensions in bytes
        long            planecount     # number of planes

    ctypedef struct t_matrix_conv_info:
        # Matrix conversion struct. Used to copy data from one matrix to another with special characteristics.
        long    flags                                  # flags for whether or not to use interpolation, or source/destination dimensions
        long    planemap[JIT_MATRIX_MAX_PLANECOUNT]    # plane mapping
        long    srcdimstart[JIT_MATRIX_MAX_DIMCOUNT]   # source dimension start 
        long    srcdimend[JIT_MATRIX_MAX_DIMCOUNT]     # source dimension end
        long    dstdimstart[JIT_MATRIX_MAX_DIMCOUNT]   # destination dimension start    
        long    dstdimend[JIT_MATRIX_MAX_DIMCOUNT]     # destination dimension end

    t_atom_long jit_method_true(void *x)
    t_atom_long jit_method_false(void *x)

    void *jit_class_new(const char *name, method mnew, method mfree, long size, ...)
    t_jit_err jit_class_free(void *c)
    t_jit_err jit_class_register(void *c)
    t_jit_err jit_class_addmethod(void *c, method m, const char *name, ...)
    t_jit_err jit_class_addattr(void *c, t_jit_object *attr)
    t_jit_err jit_class_addadornment(void *c, t_jit_object *o)
    t_jit_err jit_class_addinterface(void *c, void *interfaceclass, long byteoffset, long flags)
    void *jit_class_adornment_get(void *c, t_symbol *classname)
    t_symbol *jit_class_nameget(void *c)
    void *jit_class_findbyname(t_symbol *classname)
    long jit_object_classname_compare(void *x, t_symbol *name)
    method jit_class_method(void *c, t_symbol *methodname)
    void *jit_class_attr_get(void *c, t_symbol *attrname)
    t_jit_err jit_class_addtypedwrapper(void *c, method m, char *name, ...)
    t_messlist *jit_class_typedwrapper_get(void *c, t_symbol *s)
    t_jit_err jit_class_method_addargsafe(void *c, char *argname, char *methodname)
    t_symbol *jit_class_method_argsafe_get(void *c, t_symbol *s)

    void *jit_object_alloc(void *c)
    void *jit_object_new(t_symbol *classname, ...)
    #ifdef C74_X64
    #define jit_object_new(...) C74_VARFUN(jit_object_new_imp, __VA_ARGS__)
    #endif
    void *jit_object_new_imp(void *classname, void *p1, void *p2, void *p3, void *p4, void *p5, void *p6, void *p7, void *p8, void *dummy)
    t_jit_err jit_object_free(void *x)
    void *jit_object_method(void *x, t_symbol *s, ...)
    #ifdef C74_X64
    #define jit_object_method(...) C74_VARFUN(jit_object_method_imp, __VA_ARGS__)
    #endif
    void *jit_object_method_imp(void *x, void *s, void *p1, void *p2, void *p3, void *p4, void *p5, void *p6, void *p7, void *p8)
    void *jit_object_method_typed(void *x, t_symbol *s, long ac, t_atom *av, t_atom *rv)
    method jit_object_getmethod(void *x, t_symbol *s)
    t_symbol *jit_object_classname(void *x)
    void * jit_object_register(void *x, t_symbol *s)
    void *jit_object_findregistered(t_symbol *s)
    t_symbol *jit_object_findregisteredbyptr(void *x)
    t_jit_err jit_object_unregister(void *x)
    void *jit_object_attach(t_symbol *s, void *x)
    t_jit_err jit_object_detach(t_symbol *s, void *x)
    t_jit_err jit_object_notify(void *x, t_symbol *s, void *data)
    void *jit_object_class(void *x)
    long jit_object_attr_usercanget(void *x,t_symbol *s)
    long jit_object_attr_usercanset(void *x,t_symbol *s)
    void *jit_object_attr_get(void *x, t_symbol *attrname)
    t_jit_err jit_object_importattrs(void *x, t_symbol *s, long argc, t_atom *argv)
    t_jit_err jit_object_exportattrs(void *x, t_symbol *s, long argc, t_atom *argv)
    t_jit_err jit_object_exportsummary(void *x, t_symbol *s, long argc, t_atom *argv)
    t_symbol *jit_object_method_argsafe_get(void *x, t_symbol *s)

    # memory functions
    void *jit_getbytes(long size)
    void jit_freebytes(void *ptr,long size)
    void **jit_handle_new(long size)
    void jit_handle_free(void **handle)
    long jit_handle_size_get(void **handle)
    t_jit_err jit_handle_size_set(void **handle, long size)
    long jit_handle_lock(void **handle, long lock)
    void jit_copy_bytes(void *dest, const void *src, long bytes)
    long jit_freemem()
    char *jit_newptr(long size)
    void jit_disposeptr(char *ptr)

    # atom functions
    t_jit_err jit_atom_setlong(t_atom *a, t_atom_long b)
    t_jit_err jit_atom_setfloat(t_atom *a, double b)
    t_jit_err jit_atom_setsym(t_atom *a, t_symbol *b)              
    t_jit_err jit_atom_setobj(t_atom *a, void *b)
    t_atom_long jit_atom_getlong(t_atom *a)
    double jit_atom_getfloat(t_atom *a)
    t_symbol *jit_atom_getsym(t_atom *a)
    void *jit_atom_getobj(t_atom *a)
    long jit_atom_getcharfix(t_atom *a)
    # the following are useful for setting the values _only_ if there is an arg
    # rather than setting it to 0 or _jit_sym_nothing
    long jit_atom_arg_getlong(t_atom_long *c, long idx, long ac, t_atom *av)
    long jit_atom_arg_getfloat(float *c, long idx, long ac, t_atom *av)
    long jit_atom_arg_getdouble(double *c, long idx, long ac, t_atom *av)
    long jit_atom_arg_getsym(t_symbol **c, long idx, long ac, t_atom *av)

    # matrix info utils
    t_jit_err jit_matrix_info_default(t_jit_matrix_info *info)
    long jit_matrix_info_typesize(t_jit_matrix_info *minfo) 

    # mop utils
    t_jit_err jit_mop_single_type(void *x, t_symbol *s)
    t_jit_err jit_mop_single_planecount(void *x, long c)
    t_jit_err jit_mop_methodall(void *x, t_symbol *s, ...)
    t_jit_err jit_mop_input_nolink(void *mop, long c)
    t_jit_err jit_mop_output_nolink(void *mop, long c)
    t_jit_err jit_mop_ioproc_copy_adapt(void *mop, void *mop_io, void *matrix)
    t_jit_err jit_mop_ioproc_copy_trunc(void *mop, void *mop_io, void *matrix)
    t_jit_err jit_mop_ioproc_copy_trunc_zero(void *mop, void *mop_io, void *matrix)
    t_symbol *jit_mop_ioproc_tosym(void *ioproc)

    # attr functions
    long max_jit_attr_args_offset(short ac, t_atom *av)
    void max_jit_attr_args(void *x, short ac, t_atom *av)
    # for easy access of simple attributes
    t_atom_long jit_attr_getlong(void *x, t_symbol *s)
    t_jit_err jit_attr_setlong(void *x, t_symbol *s, t_atom_long c)
    t_atom_float jit_attr_getfloat(void *x, t_symbol *s)
    t_jit_err jit_attr_setfloat(void *x, t_symbol *s, t_atom_float c)
    t_symbol *jit_attr_getsym(void *x, t_symbol *s)
    t_jit_err jit_attr_setsym(void *x, t_symbol *s, t_symbol *c)
    long jit_attr_getlong_array(void *x, t_symbol *s, long max, t_atom_long *vals)
    t_jit_err jit_attr_setlong_array(void *x, t_symbol *s, long count, t_atom_long *vals)
    # long jit_attr_getchar_array(void *x, t_symbol *s, long max, uchar *vals)
    # t_jit_err jit_attr_setchar_array(void *x, t_symbol *s, long count, uchar *vals)
    long jit_attr_getfloat_array(void *x, t_symbol *s, long max, float *vals)
    t_jit_err jit_attr_setfloat_array(void *x, t_symbol *s, long count, float *vals)
    long jit_attr_getdouble_array(void *x, t_symbol *s, long max, double *vals)
    t_jit_err jit_attr_setdouble_array(void *x, t_symbol *s, long count, double *vals)
    long jit_attr_getsym_array(void *x, t_symbol *s, long max, t_symbol **vals)
    t_jit_err jit_attr_setsym_array(void *x, t_symbol *s, long count, t_symbol **vals)

    # attr filters util
    t_jit_err jit_attr_addfilterset_clip(void *x, double min, double max, long usemin, long usemax)
    t_jit_err jit_attr_addfilterset_clip_scale(void *x, double scale, double min, double max, long usemin, long usemax)
    t_jit_err jit_attr_addfilterget_clip(void *x, double min, double max, long usemin, long usemax)
    t_jit_err jit_attr_addfilterget_clip_scale(void *x, double scale, double min, double max, long usemin, long usemax)
    t_jit_err jit_attr_addfilter_clip(void *x, double min, double max, long usemin, long usemax)
    t_jit_err jit_attr_addfilter_clip_scale(void *x, double scale, double min, double max, long usemin, long usemax)
    t_jit_err jit_attr_addfilterset_proc(void *x, method proc)
    t_jit_err jit_attr_addfilterget_proc(void *x, method proc)

    # attr functions for assisting in user level notifications when setting attributes from C code
    t_jit_err jit_attr_user_setvalueof(void *x, t_symbol *s, long ac, t_atom *av)
    t_jit_err jit_attr_user_touch(void *x, t_symbol *s)
    t_jit_err jit_attr_user_touch_parse(void *x, char *attrnames)
    t_jit_err jit_object_addattr(void *x, t_object *attr)
    t_jit_err jit_object_deleteattr(void *x, t_symbol *attrname)

    void jit_attr_cleanall(t_object *x)
    void jit_attr_clean(t_object *x, t_symbol *name)
            
    # more util functions
    void jit_rand_setseed(long n)
    long jit_rand()
    t_symbol *jit_symbol_unique()
    void jit_error_code(void *x, t_jit_err v) # interrupt safe
    void jit_error_sym(void *x, t_symbol *s) # interrupt safe
    void jit_post_sym(void *x, t_symbol *s)  # interrupt safe

    t_jit_err jit_err_from_max_err(t_max_err err)

    # jit_matrix - type = 0, jit_gl_texture type = 1
    t_jit_err jit_video_recreate_outlet(t_object *maxob, long type, void **curoutlet)


cdef extern from "jit.cpost.h":
    void jit_cpost(const char *format, ...)


cdef extern from "jit.critical.h":
    void jit_global_critical_enter()
    void jit_global_critical_exit()


cdef extern from "jit.math.h":
    double jit_math_cos     (double x)
    double jit_math_sin     (double x)
    double jit_math_tan     (double x)
    double jit_math_acos    (double x)
    double jit_math_asin    (double x)
    double jit_math_atan    (double x)
    double jit_math_atan2   (double y, double x)
    double jit_math_cosh    (double x)
    double jit_math_sinh    (double x)
    double jit_math_tanh    (double x)
    double jit_math_acosh   (double x)
    double jit_math_asinh   (double x)
    double jit_math_atanh   (double x)
    double jit_math_exp     (double x)
    double jit_math_expm1   (double x)
    double jit_math_exp2    (double x)
    double jit_math_log     (double x)
    double jit_math_log2    (double x)
    double jit_math_log10   (double x)
    double jit_math_hypot   (double x, double y)
    double jit_math_pow     (double x, double y)
    double jit_math_sqrt    (double x)
    double jit_math_ceil    (double x)
    double jit_math_floor   (double x)
    double jit_math_round   (double x)
    double jit_math_trunc   (double x)
    double jit_math_fmod    (double x, double y)
    double jit_math_wrap    (double x, double lo, double hi)
    double jit_math_fold    (double x, double lo, double hi)
    double jit_math_j1      (double x)

    long jit_math_is_valid(float x)
    unsigned long jit_math_roundup_poweroftwo(unsigned long x)
    long jit_math_is_poweroftwo(long x)
    long jit_math_is_finite(float x)
    long jit_math_is_nan(float x)

    float jit_math_project_to_sphere(float r, float x, float y)

    float jit_math_fast_sqrt(float x)
    float jit_math_fast_invsqrt(float x)

    float jit_math_fast_sin(float x)  # absolute error of 1.7e-04 for [0, PI/2]
    float jit_math_fast_cos(float x)  # absolute error of 1.2e-03 for [0, PI/2]
    float jit_math_fast_tan(float x)  # absolute error of 1.9e-08 for [0, PI/4]
    float jit_math_fast_asin(float x) # absolute error of 6.8e-05 for [0, 1]
    float jit_math_fast_acos(float x) # absolute error of 6.8e-05 for [0, 1]
    float jit_math_fast_atan(float x) # absolute error of 1.43-08 for [-1, 1]

    # constants (compiler will collapse these)
    double JIT_MATH_F32_PI
    double JIT_MATH_F32_TWO_PI
    double JIT_MATH_F32_HALF_PI
    double JIT_MATH_F32_INV_PI
    double JIT_MATH_F32_DEGTORAD
    double JIT_MATH_F32_RADTODEG
    double JIT_MATH_F32_EPS
    double JIT_MATH_F32_MAXVAL

    double JIT_MATH_F64_PI
    double JIT_MATH_F64_TWO_PI
    double JIT_MATH_F64_HALF_PI
    double JIT_MATH_F64_INV_PI
    double JIT_MATH_F64_DEGTORAD
    double JIT_MATH_F64_RADTODEG
    double JIT_MATH_F64_EPS
    double JIT_MATH_F64_MAXVAL

    double JIT_MATH_FIXED_PI
    double JIT_MATH_FIXED_TWO_PI
    double JIT_MATH_FIXED_HALF_PI
    double JIT_MATH_FIXED_INV_PI
    double JIT_MATH_FIXED_DEGTORAD
    double JIT_MATH_FIXED_RADTODEG
    double JIT_MATH_FIXED_EPS
    double JIT_MATH_FIXED_MAXVAL


cdef extern from "max.jit.mop.h":

    # flags for greater control
    ctypedef enum t_max_jit_mop_flag:
        MAX_JIT_MOP_FLAGS_NONE                  = 0x00000000    # mop flag @ingroup jitter
        MAX_JIT_MOP_FLAGS_OWN_ALL               = 0x0FFFFFFF    # mop flag @ingroup jitter

        MAX_JIT_MOP_FLAGS_OWN_JIT_MATRIX        = 0x00000001    # mop flag @ingroup jitter
        MAX_JIT_MOP_FLAGS_OWN_BANG              = 0x00000002    # mop flag @ingroup jitter
        MAX_JIT_MOP_FLAGS_OWN_OUTPUTMATRIX      = 0x00000004    # mop flag @ingroup jitter
        MAX_JIT_MOP_FLAGS_OWN_NAME              = 0x00000008    # mop flag @ingroup jitter
        MAX_JIT_MOP_FLAGS_OWN_TYPE              = 0x00000010    # mop flag @ingroup jitter
        MAX_JIT_MOP_FLAGS_OWN_DIM               = 0x00000020    # mop flag @ingroup jitter
        MAX_JIT_MOP_FLAGS_OWN_PLANECOUNT        = 0x00000040    # mop flag @ingroup jitter
        MAX_JIT_MOP_FLAGS_OWN_CLEAR             = 0x00000080    # mop flag @ingroup jitter
        MAX_JIT_MOP_FLAGS_OWN_NOTIFY            = 0x00000100    # mop flag @ingroup jitter
        MAX_JIT_MOP_FLAGS_OWN_ADAPT             = 0x00000200    # mop flag @ingroup jitter
        MAX_JIT_MOP_FLAGS_OWN_OUTPUTMODE        = 0x00000400    # mop flag @ingroup jitter

        MAX_JIT_MOP_FLAGS_ONLY_MATRIX_PROBE     = 0x10000000    # mop flag @ingroup jitter


    ctypedef enum:
        JIT_MOP_INPUT       = 1                                 # mop flag @ingroup jitter
        JIT_MOP_OUTPUT      = 2                                 # mop flag @ingroup jitter

    t_jit_err max_jit_classex_mop_wrap(void *mclass, void *jclass, long flags)     # legacy api
    t_jit_err max_jit_class_mop_wrap(t_class *mclass, t_class *jclass, long flags) # new api
    t_jit_err max_jit_classex_mop_mproc(void *mclass, void *jclass, void *mproc)   # mproc should be method(void *x, void *mop)
    t_jit_err max_jit_object_mop_wrap(t_object *mob, t_object *job, long flags)
    t_jit_err max_jit_object_mop_mproc(void *mob, void *job, void *mproc)

    t_jit_err max_jit_mop_setup(void *x)
    t_jit_err max_jit_mop_inputs(void *x)
    t_jit_err max_jit_mop_inputs_resize(void *x, long count)
    t_jit_err max_jit_mop_outputs(void *x)
    t_jit_err max_jit_mop_outputs_resize(void *x, long count)
    t_jit_err max_jit_mop_matrix_args(void *x, long argc, t_atom *argv)
    t_jit_err max_jit_mop_jit_matrix(void *x, t_symbol *s, long argc, t_atom *argv)
    t_jit_err max_jit_mop_assist(void *x, void *b, long m, long a, char *s)
    t_jit_err max_jit_mop_bang(void *x)
    t_jit_err max_jit_mop_outputmatrix(void *x)
    t_jit_err max_jit_mop_matrixout_new(void *x, long c)
    void max_jit_mop_clear(void *x)
    t_jit_err max_jit_mop_notify(void *x, t_symbol *s, t_symbol *msg)
    void max_jit_mop_free(void *x)
    t_jit_err max_jit_mop_name(void *x, void *attr, long argc, t_atom *argv)
    t_jit_err max_jit_mop_getname(void *x, void *attr, long *argc, t_atom **argv)
    t_jit_err max_jit_mop_type(void *x, void *attr, long argc, t_atom *argv)
    t_jit_err max_jit_mop_gettype(void *x, void *attr, long *argc, t_atom **argv)
    t_jit_err max_jit_mop_dim(void *x, void *attr, long argc, t_atom *argv)
    t_jit_err max_jit_mop_getdim(void *x, void *attr, long *argc, t_atom **argv)
    t_jit_err max_jit_mop_planecount(void *x, void *attr, long argc, t_atom *argv)
    t_jit_err max_jit_mop_getplanecount(void *x, void *attr, long *argc, t_atom **argv)
    t_jit_err max_jit_mop_parse_name(t_symbol *name, long *type, long *idx)
    t_jit_err max_jit_mop_restrict_info(void *x, void *p, t_jit_matrix_info *info)
    void *max_jit_mop_get_io_by_name(void *x, t_symbol *s)
    t_jit_err max_jit_mop_outputmode(void *x, void *attr, long argc, t_atom *argv)
    t_jit_err max_jit_mop_getoutputmode_attr(void *x, void *attr, long *argc, t_atom **argv)
    t_jit_err max_jit_mop_adapt(void *x, void *attr, long argc, t_atom *argv)
    t_jit_err max_jit_mop_getadapt(void *x, void *attr, long *argc, t_atom **argv)
    void *max_jit_mop_getinput(void *x, long c)
    void *max_jit_mop_getoutput(void *x, long c)
    long max_jit_mop_getoutputmode(void *x)
    void *max_jit_mop_io_getoutlet(void *mop_io)
    t_jit_err max_jit_mop_io_setoutlet(void *mop_io, void *o)
    void *max_jit_mop_getmproc(void *mop)
    t_jit_err max_jit_mop_adapt_matrix_all(void *x, void *y)
    t_jit_err max_jit_mop_variable_parse_sym(t_symbol *name, t_symbol **msg, long *set)
    t_jit_err max_jit_mop_variable_anything(void *x, t_symbol *s, long argc, t_atom *argv)
    t_jit_err max_jit_mop_variable_addinputs(void *x, long c)
    t_jit_err max_jit_mop_variable_addoutputs(void *x, long c)

    t_jit_err max_jit_mop_setup_simple(void *x, void *o, long argc, t_atom *argv)
        # max_jit_mop_setup_simple is equivalent to :

        # max_jit_obex_jitob_set(x,o);
        # max_jit_obex_dumpout_set(x,outlet_new(x,NULL));
        # max_jit_mop_setup(x);
        # max_jit_mop_inputs(x);
        # max_jit_mop_outputs(x);
        # max_jit_mop_matrix_args(x,argc,argv);

        # NOTICE: REMOVED max_jit_attr_args from max_jit_mop_setup_simple

        # max_jit_mop_classex_mproc 


    t_jit_err max_jit_mop_setup_probing(t_class *mclass)

