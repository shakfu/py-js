# maxapi.pyx

cdef extern from "ext.h":
    ctypedef struct t_object
    ctypedef struct t_symbol
    ctypedef struct t_atom
    ctypedef struct t_class
    ctypedef struct t_messlist
    ctypedef enum e_max_atomtypes:
        A_NOTHING = 0  # no type, thus no atom
        A_LONG         # long integer
        A_FLOAT        # 32-bit float
        A_SYM          # t_symbol pointer
        A_OBJ          # t_object pointer (for argtype lists; passes the value of sym)
        A_DEFLONG      # long but defaults to zero
        A_DEFFLOAT     # float, but defaults to zero
        A_DEFSYM       # symbol, defaults to ""
        A_GIMME        # request that args be passed as an array, the routine will check the types itself.
        A_CANT         # cannot typecheck args
        A_SEMI         # semicolon
        A_COMMA        # comma
        A_DOLLAR       # dollar
        A_DOLLSYM      # dollar
        A_GIMMEBACK    # request that args be passed as an array, the routine will check the types itself. can return atom value in final atom ptr arg. function returns long error code 0 = no err. see gimmeback_meth typedef
        A_DEFER =       0x41   # A special signature for declaring methods. This is like A_GIMME, but the call is deferred.
        A_USURP =       0x42   # A special signature for declaring methods. This is like A_GIMME, but the call is deferred and multiple calls within one servicing of the queue are filtered down to one call.
        A_DEFER_LOW =   0x43   # A special signature for declaring methods. This is like A_GIMME, but the call is deferref to the back of the queue.
        A_USURP_LOW =   0x44    # A special signature for declaring methods. This is like A_GIMME, but the call is deferred to the back of the queue and multiple calls within one servicing of the queue are filtered down to one call.
    ctypedef t_object t_patcher
    ctypedef void *(*method)(void *, ...)
    # cdef void object_post(t_object *x, const char *s, ...)
    ctypedef long long t_ptr_int
    ctypedef t_ptr_int t_int
    ctypedef t_ptr_int t_atom_long


cdef extern from "ext_proto.h":
    cdef void freeobject(t_object *op)
    cdef void *newinstance(const t_symbol *s, short argc, const t_atom *argv)
    cdef void alias(char *name)
    cdef void class_setname(char *obname, char *filename)
    cdef t_symbol *gensym(const char *s)
    cdef void post(const char *fmt, ...)
    cdef void cpost(const char *fmt, ...)
    cdef void error(const char *fmt, ...)
    cdef void postatom(t_atom *ap)
    cdef void error_subscribe(t_object *x)
    cdef void error_unsubscribe(t_object *x)
    cdef void object_post(t_object *x, const char *s, ...)
    cdef void object_error(t_object *x, const char *s, ...)
    cdef void object_error_obtrusive(t_object *x, const char *s, ...)
    cdef void *inlet_new(void *x, const char *s)
    cdef void *intin(void *x, short n)
    cdef void *floatin(void *x, short n)
    cdef void *outlet_new(void *x, const char *s)
    cdef void *bangout(void *x)
    cdef void *intout(void *x)
    cdef void *floatout(void *x)
    cdef void *outlet_bang(void *o)
    cdef void *outlet_int(void *o, t_atom_long n)
    cdef void *outlet_float(void *o, double f)
    cdef void *outlet_list(void *o, t_symbol *s, short ac, t_atom *av)
    cdef void *outlet_anything(void *o, const t_symbol *s, short ac, const t_atom *av)
    cdef void *clock_new(void *obj, method fn)
    cdef void clock_delay(void *x, long n)
    cdef void clock_unset(void *x)
    cdef void clock_fdelay(void *c, double time)
    cdef void clock_getftime(double *time)
    cdef void setclock_delay(t_object *x, void *c, long time)
    cdef void setclock_unset(t_object *x, void *c)
    cdef long setclock_gettime(t_object *x)
    cdef void setclock_fdelay(t_object *s, void *c, double time)
    cdef void setclock_getftime(t_object *s, double *time)
    cdef double systimer_gettime()
    cdef long gettime()
    cdef long getschedtime()
    cdef long getexttime()
    cdef void sched_suspend()
    cdef void sched_resume()
    cdef short sched_isinpoll()
    cdef short sched_isinqueue()


cdef extern from "ext_obex.h":
    t_class *class_new(const char *name, const method mnew, const method mfree, long size, const method mmenu, short type, ...)


txt = 'Hello from Max!'

cpdef public str hello():
    return txt

cpdef public void pypost(str s):
    post(s)

