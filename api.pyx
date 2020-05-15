# maxapi.pyx

cdef extern from "ext.h":
    ctypedef t_object t_patcher


cdef extern from "ext_mess.h":
    ctypedef void *(*method)(void *, ...)
    ctypedef long (*t_intmethod)(void *, ...)
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
    cdef int ATOM_MAX_STRLEN
    ctypedef void *(*zero_meth)(void *x)
    ctypedef void *(*one_meth)(void *x, void *z)
    ctypedef void *(*two_meth)(void *x, void *z, void *a)
    ctypedef long *(*gimmeback_meth)(void *x, t_symbol *s, long ac, t_atom *av, t_atom *rv)


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
    cdef double gettime_forobject(t_object *x)
    cdef void schedule(void *ob, method fun, long when, t_symbol *sym, short argc, t_atom *argv)
    cdef void schedulef(void *ob, method fun, double when, t_symbol *sym, short argc, t_atom *argv)
    cdef void *scheduler_new()
    cdef void *scheduler_set(void *x)
    cdef void *scheduler_get()
    cdef void *scheduler_fromobject(t_object *o)
    cdef void scheduler_run(void *x, double until)
    cdef void scheduler_settime(void *x, double time)
    cdef void scheduler_gettime(void *x, double *time)
    cdef void scheduler_shift(void *x, double amount)
    cdef void schedule_delay(void *ob, method fun, long delay, t_symbol *sym, short argc, t_atom *argv)
    cdef void schedule_fdelay(void *ob, method fun, double delay, t_symbol *sym, short argc, t_atom *argv)
    cdef void schedule_defer(void *ob, method fun, long delay, t_symbol *sym, short argc, t_atom *arv)
    cdef void schedule_fdefer(void *ob, method fun, double delay, t_symbol *sym, short argc, t_atom *arv)
    cdef short lockout_set(short)
    cdef long isr()
    cdef short isr_set(short way)
    cdef void *qelem_new(void *obj, method fn)
    cdef void qelem_set(void *q)
    cdef void qelem_unset(void *q)
    cdef void qelem_free(void *x)
    cdef void qelem_front(void *x)
    cdef void *defer(void *ob,method fn,t_symbol *sym,short argc,t_atom *argv)
    cdef void *defer_low(void *ob,method fn,t_symbol *sym,short argc,t_atom *argv)
    cdef void *defer_medium(void *ob, method fn, t_symbol *sym, short argc, t_atom *argv)
    cdef void *defer_front(void *ob, method fn, t_symbol *sym, short argc, t_atom *argv)

    cdef void *binbuf_new()
    cdef void binbuf_vinsert(void *x, char *fmt, ...)
    cdef void binbuf_insert(void *x, t_symbol *s, short argc, t_atom *argv)
    cdef void *binbuf_eval(void *x, short ac, t_atom *av, void *to)
    cdef short binbuf_getatom(void *x, long *p1, long *p2, t_atom *ap)
    cdef short binbuf_text(void *x, char **srcText, long n)
    # cdef short binbuf_totext(void *x, char **dstText, t_ptr_size *sizep)
    cdef void binbuf_set(void *x, t_symbol *s, short argc, t_atom *argv);
    cdef void binbuf_append(void *x, t_symbol *s, short argc, t_atom *argv)
    cdef void binbuf_delete(void *x, long fromType, long toType, long fromData, long toData)
    cdef void binbuf_addtext(void *x, char **text, long size)
    cdef short readatom(char *outstr, char **text, long *n, long e, t_atom *ap)
    cdef char *atom_string(t_atom *a)
    cdef void *typedmess(t_object *op, t_symbol *msg, short argc, t_atom *argp)
    cdef method getfn(t_object *op, t_symbol *msg)
    cdef method egetfn(t_object *op, t_symbol *msg)
    cdef method zgetfn(t_object *op, t_symbol *msg)
    cdef short table_get(t_symbol *s, long ***hp, long *sp)
    cdef short table_dirty(t_symbol *s)
    cdef short readtohandle(const char *name, short volume, char ***h, long *sizep)
    cdef void *fileload(const char *name, short vol)
    cdef void *intload(const char *name, short volume, t_symbol *s, short ac, t_atom *av, short couldedit)
    cdef void preset_store(char *fmt, ...)
    cdef void preset_int(t_object *x, t_atom_long n)
    cdef void *proxy_new(void *x,long id,long *stuffloc)
    cdef long proxy_getinlet(t_object *master)
    cdef void *connection_client(t_object *cli, t_symbol *name, t_symbol *classname, method traverse)
    cdef void connection_server(t_object *obj, t_symbol *name)
    cdef void connection_send(t_object *server, t_symbol *name, t_symbol *mess, void *arg)
    cdef void connection_delete(t_object *ob, t_symbol *name)
    cdef void quittask_install(method m, void *a)
    cdef void quittask_remove(method m)
    cdef void quittask_remove2(method m, void *a)
    cdef short maxversion()
    cdef short ispatcher(t_object *x)
    cdef void open_promptset(const char *s)
    cdef void saveas_promptset(const char *s)
    cdef void *filewatcher_new(t_object *owner, const short path, const char *filename)
    cdef void filewatcher_start(void *x)
    cdef void filewatcher_stop(void *x)
    cdef void fileusage_addfile(void *w, long flags, const char *name, const short path)
    cdef void fileusage_addfilename(void *w, long flags, const char *name)
    cdef void fileusage_addpackage(void *w, const char *name, t_object *subfoldernames)
    cdef void fileusage_addpathname(void *w, long flags, const char *name)
    cdef void fileusage_copyfolder(void *w, const char *name, long recursive)
    cdef void fileusage_makefolder(void *w, const char *name)
    # short open_dialog(char *name, short *volptr, t_fourcc *typeptr, t_fourcc *types, short ntypes)
    # short saveas_dialog(char *filename, short *path, short *binptr)

cdef extern from "ext_dictionary.h":
    ctypedef struct t_dictionary_entry
    ctypedef struct t_dictionary

    cdef t_dictionary* dictionary_new()
    cdef t_dictionary* dictionary_prototypefromclass(t_class *c)
    cdef t_max_err dictionary_appendlong(t_dictionary *d, t_symbol *key, t_atom_long value)
    cdef t_max_err dictionary_appendfloat(t_dictionary *d, t_symbol *key, double value) 
    cdef t_max_err dictionary_appendsym(t_dictionary *d, t_symbol *key, t_symbol *value) 
    cdef t_max_err dictionary_appendatom(t_dictionary *d, t_symbol *key, t_atom *value)
    cdef t_max_err dictionary_appendattribute(t_dictionary *d, t_symbol *key, t_symbol *attrname, t_object *obj)  
    cdef t_max_err dictionary_appendstring(t_dictionary *d, t_symbol *key, const char *value) 
    cdef t_max_err dictionary_appendatoms(t_dictionary *d, t_symbol *key, long argc, t_atom *argv) 
    cdef t_max_err dictionary_appendatoms_flags(t_dictionary *d, t_symbol *key, long argc, t_atom *argv, long flags) 
    cdef t_max_err dictionary_appendatomarray(t_dictionary *d, t_symbol *key, t_object *value) 
    cdef t_max_err dictionary_appenddictionary(t_dictionary *d, t_symbol *key, t_object *value) 
    cdef t_max_err dictionary_appendobject(t_dictionary *d, t_symbol *key, t_object *value) 
    cdef t_max_err dictionary_appendobject_flags(t_dictionary *d, t_symbol *key, t_object *value, long flags) 
    cdef t_max_err dictionary_appendbinbuf(t_dictionary *d, t_symbol *key, void *value) 
    cdef t_max_err dictionary_getlong(const t_dictionary *d, t_symbol *key, t_atom_long *value)
    cdef t_max_err dictionary_getfloat(const t_dictionary *d, t_symbol *key, double *value) 
    cdef t_max_err dictionary_getsym(const t_dictionary *d, t_symbol *key, t_symbol **value) 
    cdef t_max_err dictionary_getatom(const t_dictionary *d, t_symbol *key, t_atom *value) 
    cdef t_max_err dictionary_getattribute(const t_dictionary *d, t_symbol *key, t_symbol *attrname, t_object *obj)  
    cdef t_max_err dictionary_getstring(const t_dictionary *d, t_symbol *key, const char **value)
    cdef t_max_err dictionary_getatoms(const t_dictionary *d, t_symbol *key, long *argc, t_atom **argv)
    cdef t_max_err dictionary_getatoms_ext(const t_dictionary *d, t_symbol *key, long stringstosymbols, long *argc, t_atom **argv)
    cdef t_max_err dictionary_copyatoms(const t_dictionary *d, t_symbol *key, long *argc, t_atom **argv)
    cdef t_max_err dictionary_getatomarray(const t_dictionary *d, t_symbol *key, t_object **value) 
    cdef t_max_err dictionary_getdictionary(const t_dictionary *d, t_symbol *key, t_object **value) 
    cdef t_max_err dictionary_get_ex(t_dictionary *d, t_symbol *key, long *ac, t_atom **av, char *errstr)
    cdef t_max_err dictionary_getobject(const t_dictionary *d, t_symbol *key, t_object **value) 
    cdef long dictionary_entryisstring(const t_dictionary *d, t_symbol *key) 
    cdef long dictionary_entryisatomarray(const t_dictionary *d, t_symbol *key)
    cdef long dictionary_entryisdictionary(const t_dictionary *d, t_symbol *key) 
    cdef long dictionary_hasentry(const t_dictionary *d, t_symbol *key) 
    cdef t_atom_long dictionary_getentrycount(const t_dictionary *d)
    cdef t_max_err dictionary_getkeys(const t_dictionary *d, long *numkeys, t_symbol ***keys)
    cdef t_max_err dictionary_getkeys_ordered(const t_dictionary *d, long *numkeys, t_symbol ***keys)
    cdef void dictionary_freekeys(t_dictionary *d, long numkeys, t_symbol **keys) 
    cdef t_max_err dictionary_deleteentry(t_dictionary *d, t_symbol *key)
    cdef t_max_err dictionary_chuckentry(t_dictionary *d, t_symbol *key)     
    cdef t_max_err dictionary_clear(t_dictionary *d)
    cdef t_dictionary *dictionary_clone(t_dictionary *d)
    cdef t_max_err dictionary_clone_to_existing(const t_dictionary *d, t_dictionary *dc)
    cdef t_max_err dictionary_copy_to_existing(const t_dictionary *d, t_dictionary *dc)
    cdef t_max_err dictionary_merge_to_existing(const t_dictionary *d, t_dictionary *dc)
    cdef void dictionary_funall(t_dictionary *d, method fun, void *arg)
    cdef t_symbol* dictionary_entry_getkey(t_dictionary_entry *x)
    cdef void dictionary_entry_getvalue(t_dictionary_entry *x, t_atom *value)
    cdef void dictionary_entry_getvalues(t_dictionary_entry *x, long *argc, t_atom **argv) 
    cdef t_max_err dictionary_copyunique(t_dictionary *d, t_dictionary *copyfrom)
    cdef t_max_err dictionary_getdeflong(const t_dictionary *d, t_symbol *key, t_atom_long *value, t_atom_long xdef)
    cdef t_max_err dictionary_getdeffloat(const t_dictionary *d, t_symbol *key, double *value, double xdef)
    cdef t_max_err dictionary_getdefsym(const t_dictionary *d, t_symbol *key, t_symbol **value, t_symbol *xdef)
    cdef t_max_err dictionary_getdefatom(const t_dictionary *d, t_symbol *key, t_atom *value, t_atom *xdef)
    cdef t_max_err dictionary_getdefstring(const t_dictionary *d, t_symbol *key, const char **value, char *xdef)
    cdef t_max_err dictionary_getdefatoms(t_dictionary *d, t_symbol *key, long *argc, t_atom **argv, t_atom *xdef)
    cdef t_max_err dictionary_copydefatoms(t_dictionary *d, t_symbol *key, long *argc, t_atom **argv, t_atom *xdef)
    cdef t_max_err dictionary_dump(t_dictionary *d, long recurse, long console)
    cdef t_max_err dictionary_copyentries(t_dictionary *src, t_dictionary *dst, t_symbol **keys)
    cdef t_dictionary *dictionary_sprintf(const char *fmt, ...)
    cdef t_object *newobject_sprintf(t_object *patcher, const char *fmt, ...)
    cdef t_object *newobject_fromboxtext(t_object *patcher, const char *text)
    cdef t_object *newobject_fromdictionary(t_object *patcher, t_dictionary *d)


cdef extern from "ext_obex.h":

    cdef int TRUE
    cdef int FALSE
    cdef t_symbol *CLASS_BOX
    cdef t_symbol *CLASS_NOBOX

    ctypedef enum e_max_attrflags:
        ATTR_FLAGS_NONE =       0x0000000  # No flags
        ATTR_GET_OPAQUE =       0x00000001 # The attribute cannot be queried by either max message when used inside of a CLASS_BOX object, nor from C code.
        ATTR_SET_OPAQUE =       0x00000002 # The attribute cannot be set by either max message when used inside of a CLASS_BOX object, nor from C code.
        ATTR_GET_OPAQUE_USER =  0x00000100 # The attribute cannot be queried by max message when used inside of a CLASS_BOX object, but <em>can</em> be queried from C code.
        ATTR_SET_OPAQUE_USER =  0x00000200 # The attribute cannot be set by max message when used inside of a CLASS_BOX object, but <em>can</em> be set from C code.
        ATTR_GET_DEFER =        0x00010000 # Placeholder for potential future functionality: Any attribute queries will be called through a defer().
        ATTR_GET_USURP =        0x00020000 # Placeholder for potential future functionality: Any calls to query the attribute will be called through the equivalent of a defer(), repeated calls will be ignored until the getter is actually run.
        ATTR_GET_DEFER_LOW =    0x00040000 # Placeholder for potential future functionality: Any attribute queries will be called through a defer_low().
        ATTR_GET_USURP_LOW =    0x00080000 # Placeholder for potential future functionality: Any calls to query the attribute will be called through the equivalent of a defer_low(), repeated calls will be ignored until the getter is actually run.
        ATTR_SET_DEFER =        0x01000000 # Placeholder for potential future functionality: The attribute setter will be called through a defer().
        ATTR_SET_USURP =        0x02000000 # Placeholder for potential future functionality: Any calls to set the attribute will be called through the equivalent of a defer_low(), repeated calls will be ignored until the setter is actually run.
        ATTR_SET_DEFER_LOW =    0x04000000 # Placeholder for potential future functionality: The attribute setter will be called through a defer_low()
        ATTR_SET_USURP_LOW =    0x08000000 # Placeholder for potential future functionality: Any calls to set the attribute will be called through the equivalent of a defer_low(), repeated calls will be ignored until the setter is actually run.
        ATTR_IS_JBOXATTR =      0x10000000 # a common jbox attr
        ATTR_DIRTY =            0x20000000 # attr has been changed from its default value
    
    # Standard values returned by function calls with a return type of #t_max_err
    ctypedef enum e_max_errorcodes:
        MAX_ERR_NONE =           0 # No error
        MAX_ERR_GENERIC =       -1 # Generic error
        MAX_ERR_INVALID_PTR =   -2 # Invalid Pointer
        MAX_ERR_DUPLICATE =     -3 # Duplicate
        MAX_ERR_OUT_OF_MEM =    -4 # Out of memory
    
    # Flags used in linklist and hashtab objects 
    ctypedef enum e_max_datastore_flags:
        OBJ_FLAG_OBJ =          0x00000000 # free using object_free()
        OBJ_FLAG_REF =          0x00000001 # don't free
        OBJ_FLAG_DATA =         0x00000002 # don't free data or call method
        OBJ_FLAG_MEMORY =       0x00000004 # don't call method, and when freeing use sysmem_freeptr() instead of freeobject 
        OBJ_FLAG_SILENT =       0x00000100 # don't notify when modified
        OBJ_FLAG_INHERITABLE =  0x00000200 # obexprototype entry will be inherited by subpatchers and abstractions
        OBJ_FLAG_ITERATING =    0x00001000 # used by linklist to signal when is inside iteration 
        OBJ_FLAG_DEBUG =        0x40000000 # context-dependent flag, used internally for linklist debug code


    cdef t_class *class_new(const char *name, const method mnew, const method mfree, long size, const method mmenu, short type, ...)
    cdef t_max_err class_free(t_class *c)
    cdef t_max_err class_register(t_symbol *name_space, t_class *c)
    cdef t_max_err class_alias(t_class *c, t_symbol *aliasname)

    cdef t_max_err class_copy(t_symbol *src_name_space, t_symbol *src_classname, t_symbol *dst_name_space, t_symbol *dst_classname)
    cdef t_max_err class_addmethod(t_class *c, const method m, const char *name, ...)
    cdef t_max_err class_addattr(t_class *c,t_object *attr)
    cdef t_max_err class_addadornment(t_class *c,t_object *o)
    cdef void *class_adornment_get(t_class *c,t_symbol *classname)
    cdef t_symbol *class_nameget(t_class *c)
    cdef t_class *class_findbyname(t_symbol *name_space, t_symbol *classname)
    cdef t_class *class_findbyname_casefree(t_symbol *name_space, t_symbol *classname)
    cdef t_max_err class_dumpout_wrap(t_class *c)
    cdef t_class *class_getifloaded(t_symbol *name_space, t_symbol *classname)
    cdef t_class *class_getifloaded_casefree(t_symbol *name_space, t_symbol *classname)
    cdef long object_classname_compare(void *x, t_symbol *name)
    # cdef t_hashtab *reg_object_namespace_lookup(t_symbol *name_space)
    cdef method class_method(t_class *x, t_symbol *methodname)
    cdef t_messlist *class_mess(t_class *x, t_symbol *methodname)
    cdef t_messlist *object_mess(t_object *x, t_symbol *methodname)
    cdef method class_attr_method(t_class *x, t_symbol *methodname, void **attr, long *get)
    cdef void *class_attr_get(t_class *x, t_symbol *attrname)
    cdef t_max_err class_extra_store(t_class *x,t_symbol *s,t_object *o)
    cdef t_max_err class_extra_storeflags(t_class *x,t_symbol *s,t_object *o,long flags)
    cdef void *class_extra_lookup(t_class *x,t_symbol *s)
    cdef t_max_err class_addtypedwrapper(t_class *x, method m, char *name, ...)
    cdef t_messlist *class_typedwrapper_get(t_class *x, t_symbol *s)
    cdef t_max_err object_addtypedwrapper(t_object *x, method m, char *name, ...)
    cdef t_messlist *object_typedwrapper_get(t_object *x, t_symbol *s)
    # cdef t_hashtab *class_namespace_fromsym(t_symbol *name_space)
    cdef t_max_err class_namespace_getclassnames(t_symbol *name_space, long *kc, t_symbol ***kv)
    cdef t_max_err class_setpath(t_class *x, short vol)
    cdef short class_getpath(t_class *x)
    cdef void *object_alloc(t_class *c)
    cdef void *object_new(t_symbol *name_space, t_symbol *classname, ...)
    cdef void *object_new_imp(void *p1, void *p2, void *p3, void *p4, void *p5, void *p6, void *p7, void *p8, void *p9, void *p10)
    cdef void *object_new_typed(t_symbol *name_space, t_symbol *classname, long ac, t_atom *av)
    cdef void *object_new_menufun(t_symbol *name_space, t_symbol *classname, void *p, long h, long v, long f)
    cdef t_max_err object_free(void *x)
    cdef void *object_method(void *x, t_symbol *s, ...)
    cdef void *object_method_imp(void *x, void *sym, void *p1, void *p2, void *p3, void *p4, void *p5, void *p6, void *p7, void *p8)
    cdef method object_method_direct_getmethod(t_object *x, t_symbol *sym)
    cdef void *object_method_direct_getobject(t_object *x, t_symbol *sym)
    cdef t_max_err object_method_typed(void *x, t_symbol *s, long ac, t_atom *av, t_atom *rv)
    cdef t_max_err object_method_typedfun(void *x, t_messlist *mp, t_symbol *s, long ac, t_atom *av, t_atom *rv)
    cdef method object_getmethod(void *x, t_symbol *s)
    cdef t_symbol *object_classname(void *x)
    cdef t_symbol *object_namespace(t_object *x)
    cdef t_symbol *class_namespace(t_class *c)
    cdef void *object_register(t_symbol *name_space, t_symbol *s, void *x)
    cdef t_symbol *object_register_unique(t_symbol *name_space, t_symbol *s, void *x)
    cdef void *object_findregistered(t_symbol *name_space, t_symbol *s)
    cdef t_max_err object_findregisteredbyptr(t_symbol **name_space, t_symbol **s, void *x)
    cdef t_max_err object_register_getnames(t_symbol *name_space, long *namecount, t_symbol ***names)
    cdef void *object_attach(t_symbol *name_space, t_symbol *s, void *x)
    cdef t_max_err object_detach(t_symbol *name_space, t_symbol *s, void *x)
    cdef t_max_err object_attach_byptr(void *x, void *registeredobject)
    cdef t_max_err object_attach_byptr_register(void *x, void *object_to_attach, t_symbol *reg_name_space)
    cdef t_max_err object_detach_byptr(void *x, void *registeredobject)
    cdef void *object_subscribe(t_symbol *name_space, t_symbol *s, t_symbol *classname, void *x)
    cdef t_max_err object_unsubscribe(t_symbol *name_space, t_symbol *s, t_symbol *classname, void *x)
    cdef t_max_err object_unregister(void *x)
    cdef t_max_err object_register_getnames(t_symbol *name_space, long *namecount, t_symbol ***names)
    cdef t_max_err object_notify(void *x, t_symbol *s, void *data)
    cdef t_symbol *reg_object_singlesym(t_symbol *name_space, t_symbol *s)
    cdef t_symbol *reg_object_singlesymbyptr(t_object *x)
    cdef t_max_err reg_object_singlesym_split(t_symbol *singlesym, t_symbol **name_space, t_symbol **objname)
    cdef t_class *object_class(void *x)
    cdef t_max_err object_getvalueof(void *x, long *ac, t_atom **av)
    cdef t_max_err object_setvalueof(void *x, long ac, t_atom *av)
    cdef void *object_attr_get(void *x, t_symbol *attrname)
    cdef method object_attr_method(void *x, t_symbol *methodname, void **attr, long *get)
    cdef long object_attr_usercanset(void *x,t_symbol *s)
    cdef long object_attr_usercanget(void *x,t_symbol *s)
    cdef void object_attr_getdump(void *x, t_symbol *s, long argc, t_atom *argv)
    cdef t_max_err object_attr_getvalueof(void *x, t_symbol *s, long *argc, t_atom **argv)
    cdef t_max_err object_attr_setvalueof(void *x, t_symbol *s, long argc, t_atom *argv)
    cdef t_max_err object_addattr(void *x, t_object *attr)
    cdef t_max_err object_deleteattr(void *x, t_symbol *attrsym)
    cdef t_max_err object_chuckattr(void *x, t_symbol *attrsym)
    cdef void class_obexoffset_set(t_class *c, long offset)
    cdef long class_obexoffset_get(t_class *c)
    cdef t_max_err object_obex_lookup(void *x, t_symbol *key, t_object **val)
    cdef t_max_err object_obex_lookuplong(void *x, t_symbol *key, t_atom_long *val)
    cdef t_max_err object_obex_lookupsym(void *x, t_symbol *key, t_symbol **val)
    cdef t_max_err object_obex_store(void *x,t_symbol *key, t_object *val)
    cdef t_max_err object_obex_storeflags(void *x,t_symbol *key, t_object *val, long flags)
    cdef t_max_err object_obex_storelong(void *x, t_symbol *key, t_atom_long val)
    cdef t_max_err object_obex_storesym(void *x, t_symbol *key, t_symbol *val)
    # cdef t_max_err object_obex_set(void *x, t_hashtab *obex)
    # cdef t_hashtab *object_obex_get(void *x)
    # cdef t_hashtab *object_obex_enforce(void *x)
    cdef void object_obex_dumpout(void *x, const t_symbol *s, long argc, const t_atom *argv)
    cdef t_max_err atom_setlong(t_atom *a, t_atom_long b)
    cdef t_max_err atom_setfloat(t_atom *a, double b)
    cdef t_max_err atom_setsym(t_atom *a, const t_symbol *b)
    cdef t_max_err atom_setobj(t_atom *a, void *b)
    cdef t_atom_long atom_getlong(const t_atom *a)
    cdef t_atom_float atom_getfloat(const t_atom *a)
    cdef t_symbol *atom_getsym(const t_atom *a)
    cdef void *atom_getobj(const t_atom *a)
    cdef long atom_getcharfix(const t_atom *a)
    cdef long atom_gettype(const t_atom *a)
    cdef t_max_err atom_arg_getlong(t_atom_long *c, long idx, long ac, const t_atom *av)
    cdef long atom_arg_getfloat(float *c, long idx, long ac, const t_atom *av)
    cdef long atom_arg_getdouble(double *c, long idx, long ac, const t_atom *av)
    cdef long atom_arg_getsym(t_symbol **c, long idx, long ac, const t_atom *av)
    cdef long attr_args_offset(short ac, t_atom *av)
    cdef void attr_args_process(void *x, short ac, t_atom *av)
    cdef t_object *attribute_new(const char *name, t_symbol *type, long flags, method mget, method mset)
    cdef t_object *attr_offset_new(const char *name, const t_symbol *type, long flags, const method mget, const method mset, long offset)
    cdef t_object *attr_offset_array_new(const char *name, t_symbol *type, long size, long flags, method mget, method mset, long offsetcount, long offset)
    cdef t_object *attr_filter_clip_new()
    cdef t_object *attr_filter_proc_new(method proc)
    cdef t_atom_long object_attr_getlong(void *x, t_symbol *s)
    cdef t_max_err object_attr_setlong(void *x, t_symbol *s, t_atom_long c)
    cdef t_atom_float object_attr_getfloat(void *x, t_symbol *s)
    cdef t_max_err object_attr_setfloat(void *x, t_symbol *s, t_atom_float c)
    cdef t_symbol *object_attr_getsym(void *x, t_symbol *s)
    cdef t_max_err object_attr_setsym(void *x, t_symbol *s, t_symbol *c)
    cdef char object_attr_getchar(void *x, t_symbol *s)
    cdef t_max_err object_attr_setchar(void *x, t_symbol *s, char c)
    cdef t_object* object_attr_getobj(void *x, t_symbol *s)
    cdef t_max_err object_attr_setobj(void *x, t_symbol *s, t_object *o)
    cdef long object_attr_getlong_array(void *x, t_symbol *s, long max, t_atom_long *vals)
    cdef t_max_err object_attr_setlong_array(void *x, t_symbol *s, long count, t_atom_long *vals)
    cdef long object_attr_getchar_array(void *x, t_symbol *s, long max, t_uint8 *vals)
    cdef t_max_err object_attr_setchar_array(void *x, t_symbol *s, long count, const t_uint8 *vals)
    cdef long object_attr_getfloat_array(void *x, t_symbol *s, long max, float *vals)
    cdef t_max_err object_attr_setfloat_array(void *x, t_symbol *s, long count, float *vals)
    cdef long object_attr_getdouble_array(void *x, t_symbol *s, long max, double *vals)
    cdef t_max_err object_attr_setdouble_array(void *x, t_symbol *s, long count, double *vals)
    cdef long object_attr_getsym_array(void *x, t_symbol *s, long max, t_symbol **vals)
    cdef t_max_err object_attr_setsym_array(void *x, t_symbol *s, long count, t_symbol **vals)
    cdef t_max_err attr_addfilterset_clip(void *x, double min, double max, long usemin, long usemax)
    cdef t_max_err attr_addfilterset_clip_scale(void *x, double scale, double min, double max, long usemin, long usemax)
    cdef t_max_err attr_addfilterget_clip(void *x, double min, double max, long usemin, long usemax)
    cdef t_max_err attr_addfilterget_clip_scale(void *x, double scale, double min, double max, long usemin, long usemax)
    cdef t_max_err attr_addfilter_clip(void *x, double min, double max, long usemin, long usemax)
    cdef t_max_err attr_addfilter_clip_scale(void *x, double scale, double min, double max, long usemin, long usemax)
    cdef t_max_err attr_addfilterset_proc(void *x, method proc)
    cdef t_max_err attr_addfilterget_proc(void *x, method proc)
    cdef t_symbol *symbol_unique()
    cdef t_symbol *symbol_stripquotes(t_symbol *s)
    cdef void error_code(void *x,t_max_err v)
    cdef void error_sym(void *x,t_symbol *s)
    cdef void post_sym(void *x,t_symbol *s)
    cdef t_max_err symbolarray_sort(long ac, t_symbol **av)
    cdef void object_obex_quickref(void *x, long *numitems, t_symbol **items)
    cdef method class_menufun_get(t_class *c)
    cdef long class_clonable(t_class *x)
    cdef long object_clonable(t_object *x)
    cdef t_max_err class_buildprototype(t_class *x)
    cdef t_dictionary *class_cloneprototype(t_class *x)
    cdef void attr_args_dictionary(t_dictionary *x, short ac, t_atom *av)
    cdef void attr_dictionary_process(void *x, t_dictionary *d)
    cdef void attr_dictionary_check(void *x, t_dictionary *d)
    cdef t_dictionary *object_dictionaryarg(long ac, t_atom *av)
    cdef t_max_err class_sticky(t_class *x, t_symbol *stickyname, t_symbol *s, t_object *o)
    cdef t_max_err class_sticky_clear(t_class *x, t_symbol *stickyname, t_symbol *s)
    cdef t_max_err object_retain(t_object *x)
    cdef t_max_err object_release(t_object *x)
    ctypedef struct t_method_object
    cdef t_method_object *method_object_new(method m, const char *name, ...)
    cdef t_method_object *method_object_new_messlist(t_messlist *m)
    cdef void method_object_free(t_method_object *x)
    cdef t_symbol *method_object_getname(t_method_object *x)
    cdef void method_object_setname(t_method_object *x, t_symbol *s)
    cdef method method_object_getmethod(t_method_object *x)
    cdef void method_object_setmethod(t_method_object *x, method m)
    cdef t_messlist *method_object_getmesslist(t_method_object *x)
    cdef void method_object_setmesslist(t_method_object *x, t_messlist *m)
    cdef t_method_object *class_getmethod_object(t_class *x, t_symbol *methodname)
    cdef t_method_object *object_getmethod_object(t_object *x, t_symbol *methodname)
    # cdef t_max_err object_attrhash_apply(t_object *x, t_hashtab *attrhash)
    cdef t_max_err object_sticky(t_object *x, t_symbol *stickyname, t_symbol *s, t_object *o)
    cdef t_max_err object_sticky_clear(t_object *x, t_symbol *stickyname, t_symbol *s)
    cdef t_max_err object_addmethod(t_object *x, method m, const char *name, ...)
    cdef t_max_err object_addmethod_object(t_object *x, t_object *mo)
    cdef t_max_err object_deletemethod(t_object *x, t_symbol *methodsym)
    cdef t_max_err object_chuckmethod(t_object *x, t_symbol *methodsym)
    cdef t_max_err attr_typedfun_set(void *parent, t_object *x, long ac, t_atom *av)
    cdef t_max_err object_attr_getnames(void *x, long *argc, t_symbol ***argv)
    cdef t_max_err atom_alloc(long *ac, t_atom **av, char *alloc)
    cdef t_max_err atom_alloc_array(long minsize, long *ac, t_atom **av, char *alloc)
    cdef long class_is_box(t_class *c)
    cdef t_dictionary *object_dictionary_fromnewargs(t_object *patcher, t_class *c, long argc, t_atom *argv, long flags, char *freedict)
    cdef long class_is_ui(t_class *c)
    cdef t_max_err class_subclass(t_class *superclass, t_class *subclass)
    cdef t_object *class_super_construct(t_class *c, ...)
    cdef t_object *class_super_construct_imp(void *c, void *p1, void *p2, void *p3, void *p4, void *p5, void *p6, void *p7, void *p8, void *p9)
    cdef void *object_super_method(t_object *x, t_symbol *s, ...)
    cdef void *object_super_method_imp(void *x, void *s, void *p1, void *p2, void *p3, void *p4, void *p5, void *p6, void *p7, void *p8)
    cdef void *object_this_method(t_object *x, t_symbol *s, ...)
    cdef void *object_this_method_imp(void *x, void *s, void *p1, void *p2, void *p3, void *p4, void *p5, void *p6, void *p7, void *p8)
    cdef t_max_err object_attr_touch(t_object *x, t_symbol *attrname)
    cdef t_max_err object_attr_touch_parse(t_object *x, char *attrnames)


txt = 'Hello from Max!'

cpdef public str hello():
    return txt

cpdef public void pypost(str s):
    post(s)


