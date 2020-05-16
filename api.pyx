# api.pyx
"""
- [ ] commonsyms.h
- [ ] ext.h
- [ ] ext_atomarray.h
- [x] ext_atombuf.h
- [ ] ext_atomic.h
- [x] ext_backgroundtask.h
- [p] ext_boxstyle.h
- [ ] ext_byteorder.h
- [ ] ext_charset.h
- [ ] ext_common.h
- [ ] ext_critical.h
- [x] ext_database.h
- [x] ext_default.h
- [x] ext_dictionary.h
- [x] ext_dictobj.h
- [ ] ext_drag.h
- [x] ext_expr.h
- [x] ext_globalsymbol.h
- [x] ext_hashtab.h
- [ ] ext_itm.h
- [x] ext_linklist.h
- [x] ext_maxtypes.h
- [x] ext_mess.h
- [x] ext_obex.h
- [x] ext_obex_util.h
- [ ] ext_obstring.h
- [ ] ext_packages.h
- [x] ext_parameter.h
- [ ] ext_path.h
- [ ] ext_preferences.h
- [ ] ext_prefix.h
- [ ] ext_preprocessor.h
- [x] ext_proto.h
- [ ] ext_proto_win.h
- [ ] ext_qtimage.h
- [ ] ext_qtstubs.h
- [ ] ext_quickmap.h
- [ ] ext_sndfile.h
- [ ] ext_strings.h
- [ ] ext_symobject.h
- [ ] ext_sysfile.h
- [ ] ext_sysmem.h
- [x] ext_sysmidi.h
- [ ] ext_sysparallel.h
- [ ] ext_sysprocess.h
- [ ] ext_syssem.h
- [ ] ext_sysshmem.h
- [ ] ext_systhread.h
- [ ] ext_systime.h
- [ ] ext_time.h
- [ ] ext_wind.h
- [ ] ext_xmltree.h
- [ ] indexmap.h
- [ ] jdataview.h
- [ ] jgraphics.h
- [ ] jpatcher_api.h
- [ ] jpatcher_syms.h
- [ ] jpatcher_utils.h
- [x] max_types.h
"""

cdef extern from "ext.h":
    DEF PY_DUMMY = 1 # not used just include ext.h with a single line

cdef extern from "ext_atombuf.h":
    ctypedef struct t_atombuf
    void *atombuf_new(long argc, t_atom *argv)
    void atombuf_free(t_atombuf *x)
    void atombuf_text(t_atombuf **x, char **text, long size)
    short atombuf_totext(t_atombuf *x, char **text, long *size)
    short atombuf_count(t_atombuf *x)
    void atombuf_set(t_atombuf *x, long start, long num)
    long atombuf_replacepoundargs(t_atombuf *x, long argc, t_atom *argv)


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
        A_USURP_LOW =   0x44   # A special signature for declaring methods. This is like A_GIMME, but the call is deferred to the back of the queue and multiple calls within one servicing of the queue are filtered down to one call.
    cdef int ATOM_MAX_STRLEN
    ctypedef void *(*zero_meth)(void *x)
    ctypedef void *(*one_meth)(void *x, void *z)
    ctypedef void *(*two_meth)(void *x, void *z, void *a)
    ctypedef long *(*gimmeback_meth)(void *x, t_symbol *s, long ac, t_atom *av, t_atom *rv)


cdef extern from "ext_backgroundtask.h":
    ctypedef struct t_backgroundtask 
        
    cdef long backgroundtask_execute(t_object *owner, void *args, method cbtask, method cbcomplete, t_backgroundtask **task, long flags)
    cdef long backgroundtask_execute_method(
        t_object *obtask, t_symbol *mtask, long actask, t_atom *avtask, 
        t_object *obcomp, t_symbol *mcomp, long accomp, t_atom *avcomp,
        t_backgroundtask **task, long flags)
    cdef void backgroundtask_purge_object(t_object *owner)
    cdef void backgroundtask_join_object(t_object *owner)
    cdef long backgroundtask_cancel(t_backgroundtask *task)
    cdef long backgroundtask_join(t_backgroundtask *task)


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


cdef extern from "ext_hashtab.h":
    cdef int HASH_DEFSLOTS
    ctypedef struct t_hashtab_entry
    ctypedef struct t_hashtab

    cdef t_hashtab *hashtab_new(long slotcount)
    cdef t_max_err hashtab_store(t_hashtab *x, t_symbol *key, t_object *val)
    cdef t_max_err hashtab_storelong(t_hashtab *x, t_symbol *key, t_atom_long val)
    cdef t_max_err hashtab_storesym(t_hashtab *x, t_symbol *key, t_symbol *val)
    cdef t_max_err hashtab_store_safe(t_hashtab *x, t_symbol *key, t_object *val)
    cdef t_max_err hashtab_storeflags(t_hashtab *x, t_symbol *key, t_object *val, long flags)
    cdef t_max_err hashtab_lookup(t_hashtab *x, t_symbol *key, t_object **val)
    cdef t_max_err hashtab_lookuplong(t_hashtab *x, t_symbol *key, t_atom_long *val) 
    cdef t_max_err hashtab_lookupsym(t_hashtab *x, t_symbol *key, t_symbol **val) 
    cdef t_max_err hashtab_lookupentry(t_hashtab *x, t_symbol *key, t_hashtab_entry **entry)
    cdef t_max_err hashtab_lookupflags(t_hashtab *x, t_symbol *key, t_object **val, long *flags)
    cdef t_max_err hashtab_delete(t_hashtab *x, t_symbol *key)
    cdef t_max_err hashtab_clear(t_hashtab *x)
    cdef t_max_err hashtab_chuckkey(t_hashtab *x, t_symbol *key)
    cdef t_max_err hashtab_chuck(t_hashtab *x)
    cdef t_max_err hashtab_findfirst(t_hashtab *x, void **o, long cmpfn(void *, void *), void *cmpdata)
    cdef t_max_err hashtab_methodall(t_hashtab *x, t_symbol *s, ...)
    #cdef t_max_err hashtab_methodall(...)
    cdef t_max_err hashtab_methodall_imp(void *x, void *sym, void *p1, void *p2, void *p3, void *p4, void *p5, void *p6, void *p7, void *p8)
    cdef t_max_err hashtab_funall(t_hashtab *x, method fun, void *arg)
    cdef t_max_err hashtab_objfunall(t_hashtab *x, method fun, void *arg)
    cdef t_atom_long hashtab_getsize(t_hashtab *x)
    cdef void hashtab_print(t_hashtab *x)
    cdef void hashtab_readonly(t_hashtab *x, long readonly)
    cdef void hashtab_flags(t_hashtab *x, long flags)
    cdef t_atom_long hashtab_getflags(t_hashtab *x)
    cdef t_max_err hashtab_keyflags(t_hashtab *x, t_symbol *key, long flags)
    cdef t_atom_long hashtab_getkeyflags(t_hashtab *x, t_symbol *key)
    cdef t_max_err hashtab_getkeys(t_hashtab *x, long *kc, t_symbol ***kv)
    cdef t_hashtab_entry *hashtab_entry_new(t_symbol *key, t_object *val)
    cdef void hashtab_entry_free(t_hashtab_entry *x)


cdef extern from "ext_linklist.h":

    ctypedef struct t_llelem
    ctypedef struct t_linklist
    #cdef long LINKLIST_PRUNE_CHUCK 0x00000001L     
    ctypedef long (*t_cmpfn)(void *, void *)

    cdef t_linklist *linklist_new()
    cdef void linklist_chuck(t_linklist *x) 
    cdef t_atom_long linklist_getsize(t_linklist *x)
    cdef void *linklist_getindex(t_linklist *x, long index)
    cdef t_llelem *linklist_index2ptr(t_linklist *x, long index)
    cdef long linklist_ptr2index(t_linklist *x, t_llelem *p)
    cdef t_atom_long linklist_objptr2index(t_linklist *x, void *p)
    cdef t_atom_long linklist_append(t_linklist *x, void *o)
    cdef t_atom_long linklist_insertindex(t_linklist *x,  void *o, long index)
    cdef long linklist_insert_sorted(t_linklist *x, void *o, long cmpfn(void *, void *))
    cdef t_llelem *linklist_insertafterobjptr(t_linklist *x, void *o, void *objptr)  
    cdef t_llelem *linklist_insertbeforeobjptr(t_linklist *x, void *o, void *objptr) 
    cdef t_llelem *linklist_moveafterobjptr(t_linklist *x, void *o, void *objptr)    
    cdef t_llelem *linklist_movebeforeobjptr(t_linklist *x, void *o, void *objptr)   
    cdef t_llelem *linklist_insertptr(t_linklist *x,  void *o, t_llelem *p) 
    cdef t_atom_long linklist_deleteindex(t_linklist *x, long index) 
    cdef long linklist_chuckindex(t_linklist *x, long index)
    cdef long linklist_chuckobject(t_linklist *x, void *o)
    cdef long linklist_deleteobject(t_linklist *x, void *o)
    cdef long linklist_deleteptr(t_linklist *x, t_llelem *p)
    cdef long linklist_chuckptr(t_linklist *x, t_llelem *p) 
    cdef void linklist_clear(t_linklist *x)
    cdef long linklist_insertnodeindex(t_linklist *x, t_llelem *p, long index)
    cdef t_llelem *linklist_insertnodeptr(t_linklist *x, t_llelem *p1, t_llelem *p2)
    cdef long linklist_appendnode(t_linklist *x, t_llelem *p)
    cdef t_llelem *linklistelem_new()
    cdef void linklistelem_free(t_linklist *x, t_llelem *elem)
    cdef t_atom_long linklist_makearray(t_linklist *x, void **a, long max)
    cdef void linklist_reverse(t_linklist *x)
    cdef void linklist_rotate(t_linklist *x, long i)
    cdef void linklist_shuffle(t_linklist *x)
    cdef void linklist_swap(t_linklist *x, long a, long b)
    cdef t_atom_long linklist_findfirst(t_linklist *x, void **o, long cmpfn(void *, void *), void *cmpdata)
    cdef void linklist_findall(t_linklist *x, t_linklist **out, long cmpfn(void *, void *), void *cmpdata)
    cdef void linklist_methodall(t_linklist *x, t_symbol *s, ...)
    cdef void linklist_methodall_imp(void *x, void *sym, void *p1, void *p2, void *p3, void *p4, void *p5, void *p6, void *p7, void *p8)
    cdef void *linklist_methodindex(t_linklist *x, t_atom_long i, t_symbol *s, ...)
    cdef void *linklist_methodindex_imp(void *x, void *i, void *s, void *p1, void *p2, void *p3, void *p4, void *p5, void *p6, void *p7) 
    cdef void linklist_sort(t_linklist *x, long cmpfn(void *, void *))
    cdef void linklist_funall(t_linklist *x, method fun, void *arg)
    cdef t_atom_long linklist_funall_break(t_linklist *x, method fun, void *arg)
    cdef void *linklist_funindex(t_linklist *x, long i, method fun, void *arg)
    cdef void *linklist_substitute(t_linklist *x, void *p, void *newp)
    cdef void *linklist_next(t_linklist *x, void *p, void **next)
    cdef void *linklist_prev(t_linklist *x, void *p, void **prev)
    cdef void *linklist_last(t_linklist *x, void **item)
    cdef void linklist_readonly(t_linklist *x, long readonly)
    cdef void linklist_flags(t_linklist *x, long flags)
    cdef t_atom_long linklist_getflags(t_linklist *x)
    cdef long linklist_match(void *a, void *b)


cdef extern from "ext_maxtypes.h":
    ctypedef t_object t_patcher
    ctypedef t_object t_box
    ctypedef t_object t_clock
    ctypedef void* t_qelem

    ctypedef enum:
        PI_DEEP = 1         
        PI_REQUIREFIRSTIN = 2
        PI_WANTBOX = 4          
        PI_SKIPGEN = 8
        PI_WANTPATCHER = 16

    ctypedef struct Zll
    ctypedef struct t_zll

    ctypedef struct Funbuff
    ctypedef struct t_funbuff 


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
    # [private] cdef t_dictionary* dictionary_prototypefromclass(t_class *c)
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


cdef extern from "ext_dictobj.h":

    cdef t_dictionary *dictobj_register(t_dictionary *d, t_symbol **name)
    cdef t_max_err dictobj_unregister(t_dictionary *d)
    cdef t_dictionary *dictobj_findregistered_clone(t_symbol *name)
    cdef t_dictionary *dictobj_findregistered_retain(t_symbol *name)
    cdef t_max_err dictobj_release(t_dictionary *d)
    cdef t_symbol *dictobj_namefromptr(t_dictionary *d)
    cdef void dictobj_outlet_atoms(void *out, long argc, t_atom *argv)
    cdef long dictobj_atom_safety(t_atom *a)
    ctypedef enum:
        DICTOBJ_ATOM_FLAGS_DEFAULT = 0
        DICTOBJ_ATOM_FLAGS_REGISTER
    cdef long dictobj_atom_safety_flags(t_atom *a, long flags)
    cdef void dictobj_atom_release(t_atom *a)
    cdef long dictobj_validate(const t_dictionary *schema, const t_dictionary *candidate)
    cdef t_max_err dictobj_jsonfromstring(long *jsonsize, char **json, const char *str)
    cdef t_max_err dictobj_dictionaryfromstring(t_dictionary **d, const char *str, int str_is_already_json, char *errorstring)
    cdef t_max_err dictobj_dictionaryfromatoms(t_dictionary **d, const long argc, const t_atom *argv)
    cdef t_max_err dictobj_dictionaryfromatoms_extended(t_dictionary **d, const t_symbol *msg, long argc, const t_atom *argv)
    cdef t_max_err dictobj_dictionarytoatoms(const t_dictionary *d, long *argc, t_atom **argv)
    cdef t_max_err dictobj_key_parse(t_object *x, t_dictionary *d, t_atom *akey, t_bool create, t_dictionary **targetdict, t_symbol **targetkey, t_int32 *index)


cdef extern from "ext_expr.h":
    
    cdef int EXPR_MAX_VARS
        
    ctypedef enum e_max_expr_types:
        ET_INT =    0x1
        ET_FLT =    0x2
        ET_OP  =    0x3
        ET_STR =    0x4
        ET_TBL =    0x5
        ET_FUNC =   0x6
        ET_SYM =    0x7
        ET_VSYM =   0x8
        ET_LP =     0x9 
        ET_LB =     0x10
        ET_II =     0x11
        ET_FI =     0x12
        ET_SI =     0x13

    ctypedef struct t_ex_ex
    ctypedef struct t_expr

    cdef void *expr_new(long argc, t_atom *argv, t_atom *types)
    cdef short expr_eval(t_expr *x, long argc, t_atom *argv, t_atom *result)
    cdef void expr_install(method fun, const char *buf, long argc)


cdef extern from "ext_globalsymbol.h":
    cdef void *globalsymbol_reference(t_object *x, const char *name, const char *classname)
    cdef void globalsymbol_dereference(t_object *x, const char *name, const char *classname)
    cdef t_max_err globalsymbol_bind(t_object *x, const char *name, long flags)
    cdef void globalsymbol_unbind(t_object *x, const char *name, long flags)
    cdef void globalsymbol_notify(t_object *x, const char *name, t_symbol *msg, void *data)


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


cdef extern from "ext_obex_util.h":
    cdef USESYM(x)
    cdef CLASS_ATTR_CHAR(c,attrname,flags,structname,structmember)
    cdef CLASS_ATTR_LONG(c,attrname,flags,structname,structmember)
    cdef CLASS_ATTR_ATOM_LONG(c,attrname,flags,structname,structmember)
    cdef CLASS_ATTR_INT32(c,attrname,flags,structname,structmember)
    cdef CLASS_ATTR_FILEPATH(c,attrname,flags,structname,structmember)
    cdef CLASS_ATTR_FLOAT(c,attrname,flags,structname,structmember)
    cdef CLASS_ATTR_DOUBLE(c,attrname,flags,structname,structmember)
    cdef CLASS_ATTR_SYM(c,attrname,flags,structname,structmember)

    cdef CLASS_ATTR_ATOM(c,attrname,flags,structname,structmember)
    cdef CLASS_ATTR_OBJ(c,attrname,flags,structname,structmember)
    cdef CLASS_ATTR_CHAR_ARRAY(c,attrname,flags,structname,structmember,size)
    cdef CLASS_ATTR_LONG_ARRAY(c,attrname,flags,structname,structmember,size)
    cdef CLASS_ATTR_ATOM_LONG_ARRAY(c,attrname,flags,structname,structmember,size)
    cdef CLASS_ATTR_FLOAT_ARRAY(c,attrname,flags,structname,structmember,size)
    cdef CLASS_ATTR_DOUBLE_ARRAY(c,attrname,flags,structname,structmember,size)
    cdef CLASS_ATTR_SYM_ARRAY(c,attrname,flags,structname,structmember,size)
    cdef CLASS_ATTR_ATOM_ARRAY(c,attrname,flags,structname,structmember,size)
    cdef CLASS_ATTR_OBJ_ARRAY(c,attrname,flags,structname,structmember,size)
    cdef CLASS_ATTR_CHAR_VARSIZE(c,attrname,flags,structname,structmember,sizemember,maxsize) 
    cdef CLASS_ATTR_LONG_VARSIZE(c,attrname,flags,structname,structmember,sizemember,maxsize)
    cdef CLASS_ATTR_FLOAT_VARSIZE(c,attrname,flags,structname,structmember,sizemember,maxsize)
    cdef CLASS_ATTR_DOUBLE_VARSIZE(c,attrname,flags,structname,structmember,sizemember,maxsize)
    cdef CLASS_ATTR_SYM_VARSIZE(c,attrname,flags,structname,structmember,sizemember,maxsize)
    cdef CLASS_ATTR_ATOM_VARSIZE(c,attrname,flags,structname,structmember,sizemember,maxsize)
    cdef CLASS_ATTR_OBJ_VARSIZE(c,attrname,flags,structname,structmember,sizemember,maxsize)
    cdef STRUCT_ATTR_CHAR(c,flags,structname,structmember)
    cdef STRUCT_ATTR_LONG(c,flags,structname,structmember)
    cdef STRUCT_ATTR_ATOM_LONG(c,flags,structname,structmember)
    cdef STRUCT_ATTR_FLOAT(c,flags,structname,structmember)
    cdef STRUCT_ATTR_DOUBLE(c,flags,structname,structmember)
    cdef STRUCT_ATTR_SYM(c,flags,structname,structmember)
    cdef STRUCT_ATTR_ATOM(c,flags,structname,structmember)
    cdef STRUCT_ATTR_OBJ(c,flags,structname,structmember)
    cdef STRUCT_ATTR_CHAR_ARRAY(c,flags,structname,structmember,size)
    cdef STRUCT_ATTR_LONG_ARRAY(c,flags,structname,structmember,size)
    cdef STRUCT_ATTR_FLOAT_ARRAY(c,flags,structname,structmember,size)
    cdef STRUCT_ATTR_DOUBLE_ARRAY(c,flags,structname,structmember,size)
    cdef STRUCT_ATTR_SYM_ARRAY(c,flags,structname,structmember,size) 
    cdef STRUCT_ATTR_ATOM_ARRAY(c,flags,structname,structmember,size)
    cdef STRUCT_ATTR_OBJ_ARRAY(c,flags,structname,structmember,size) 
    cdef STRUCT_ATTR_CHAR_VARSIZE(c,flags,structname,structmember,sizemember,maxsize)
    cdef STRUCT_ATTR_LONG_VARSIZE(c,flags,structname,structmember,sizemember,maxsize)
    cdef STRUCT_ATTR_FLOAT_VARSIZE(c,flags,structname,structmember,sizemember,maxsize)
    cdef STRUCT_ATTR_DOUBLE_VARSIZE(c,flags,structname,structmember,sizemember,maxsize)
    cdef STRUCT_ATTR_SYM_VARSIZE(c,flags,structname,structmember,sizemember,maxsize) 
    cdef STRUCT_ATTR_ATOM_VARSIZE(c,flags,structname,structmember,sizemember,maxsize)
    cdef STRUCT_ATTR_OBJ_VARSIZE(c,flags,structname,structmember,sizemember,maxsize) 
    # cdef STATIC_ATTR_ATOMS
    # cdef STATIC_ATTR_PARSE
    # cdef STATIC_ATTR_FORMAT
    cdef STATIC_ATTR_CHAR(c,attrname,flags,val)
    cdef STATIC_ATTR_LONG(c,attrname,flags,val)
    cdef STATIC_ATTR_FLOAT(c,attrname,flags,val)
    cdef STATIC_ATTR_DOUBLE(c,attrname,flags,val)
    cdef STATIC_ATTR_SYM(c,attrname,flags,val)
    cdef STATIC_ATTR_ATOM(c,attrname,flags,val)
    cdef STATIC_ATTR_OBJ(c,attrname,flags,val)
    cdef STATIC_ATTR_CHAR_ARRAY(c,attrname,flags,count,vals) 
    cdef STATIC_ATTR_LONG_ARRAY(c,attrname,flags,count,vals) 
    cdef STATIC_ATTR_FLOAT_ARRAY(c,attrname,flags,count,vals)
    cdef STATIC_ATTR_DOUBLE_ARRAY(c,attrname,flags,count,vals)
    cdef STATIC_ATTR_SYM_ARRAY(c,attrname,flags,count,vals)
    # cdef STATIC_ATTR_ATOM_ARRAY  
    cdef STATIC_ATTR_OBJ_ARRAY(c,attrname,flags,count,vals)  
    # cdef OBJ_ATTR_ATOMS
    # cdef OBJ_ATTR_PARSE
    # cdef OBJ_ATTR_FORMAT
    cdef OBJ_ATTR_CHAR(x,attrname,flags,val)
    cdef OBJ_ATTR_LONG(x,attrname,flags,val)
    cdef OBJ_ATTR_FLOAT(x,attrname,flags,val)
    cdef OBJ_ATTR_DOUBLE(x,attrname,flags,val)
    cdef OBJ_ATTR_SYM(x,attrname,flags,val)
    cdef OBJ_ATTR_ATOM(x,attrname,flags,val)
    cdef OBJ_ATTR_OBJ(x,attrname,flags,val)  
    cdef OBJ_ATTR_CHAR_ARRAY(x,attrname,flags,count,vals)
    cdef OBJ_ATTR_LONG_ARRAY(x,attrname,flags,count,vals)
    cdef OBJ_ATTR_FLOAT_ARRAY(x,attrname,flags,count,vals)
    cdef OBJ_ATTR_DOUBLE_ARRAY(x,attrname,flags,count,vals)
    cdef OBJ_ATTR_SYM_ARRAY(x,attrname,flags,count,vals) 
    # cdef OBJ_ATTR_ATOM_ARRAY
    cdef OBJ_ATTR_OBJ_ARRAY(x,attrname,flags,count,vals) 
    cdef CLASS_ATTR_ACCESSORS(c,attrname,getter,setter)
    cdef CLASS_ATTR_ADD_FLAGS(c,attrname,flags)
    cdef CLASS_ATTR_REMOVE_FLAGS(c,attrname,flags)
    cdef CLASS_ATTR_FILTER_MIN(c,attrname,minval)
    cdef CLASS_ATTR_FILTER_MAX(c,attrname,maxval)
    cdef CLASS_ATTR_FILTER_CLIP(c,attrname,minval,maxval)
    cdef CLASS_ATTR_ALIAS(c,attrname,aliasname)
    # cdef CLASS_ATTR_ATTR_ATOMS
    # cdef CLASS_ATTR_ATTR_PARSE
    # cdef CLASS_ATTR_ATTR_FORMAT
    cdef CLASS_ATTR_DEFAULT(c,attrname,flags,parsestr)
    cdef CLASS_ATTR_SAVE(c,attrname,flags)
    cdef CLASS_ATTR_SELFSAVE(c,attrname,flags)
    cdef CLASS_ATTR_DEFAULT_SAVE(c,attrname,flags,parsestr) 
    cdef CLASS_ATTR_DEFAULTNAME(c,attrname,flags,parsestr)

    cdef CLASS_ATTR_DEFAULTNAME_SAVE(c,attrname,flags,parsestr)
    cdef CLASS_ATTR_MIN(c,attrname,flags,parsestr)
    cdef CLASS_ATTR_MAX(c,attrname,flags,parsestr)
    cdef CLASS_ATTR_PAINT(c,attrname,flags)
    cdef CLASS_ATTR_DEFAULT_PAINT(c,attrname,flags,parsestr)
    cdef CLASS_ATTR_DEFAULT_SAVE_PAINT(c,attrname,flags,parsestr)
    cdef CLASS_ATTR_DEFAULTNAME_PAINT(c,attrname,flags,parsestr)
    cdef CLASS_ATTR_DEFAULTNAME_SAVE_PAINT(c,attrname,flags,parsestr)
    cdef CLASS_ATTR_STYLE(c,attrname,flags,parsestr)
    cdef CLASS_ATTR_LABEL(c,attrname,flags,labelstr)
    cdef CLASS_ATTR_ENUM(c,attrname,flags,parsestr)
    cdef CLASS_ATTR_ENUMINDEX(c,attrname,flags,parsestr)
    cdef CLASS_ATTR_ENUMINDEX2(c,attrname,flags,enum1,enum2)
    cdef CLASS_ATTR_ENUMINDEX3(c,attrname,flags,enum1,enum2,enum3)
    cdef CLASS_ATTR_ENUMINDEX4(c,attrname,flags,enum1,enum2,enum3,enum4)
    cdef CLASS_ATTR_ENUMINDEX5(c,attrname,flags,enum1,enum2,enum3,enum4,enum5)
    cdef CLASS_ATTR_ENUMINDEX6(c,attrname,flags,enum1,enum2,enum3,enum4,enum5,enum6)
    cdef CLASS_ATTR_ENUMINDEX7(c,attrname,flags,enum1,enum2,enum3,enum4,enum5,enum6,enum7)
    cdef CLASS_ATTR_CATEGORY(c,attrname,flags,parsestr)
    cdef CLASS_ATTR_STYLE_LABEL(c,attrname,flags,stylestr,labelstr)
    cdef CLASS_ATTR_INVISIBLE(c,attrname,flags)
    cdef CLASS_ATTR_ORDER(c,attrname,flags,parsestr)
    cdef CLASS_ATTR_BASIC(c,attrname,flags)
    cdef CLASS_ATTR_ATOMARRAY(c,attrname,flags)
    cdef CLASS_METHOD_ATTR_PARSE(c,methodname,attrname,type,flags,parsestring)
    cdef CLASS_ATTR_LEGACYDEFAULT(c,legacyattrname,newattrname,flags,parsestr)
    cdef CLASS_ATTR_OBSOLETE(c,attrname,flags)
    cdef CLASS_ATTR_RENAMED(c,oldname,newname,flags)
    cdef CLASS_ATTR_INTRODUCED(c,attrname,flags,versionstr)
    cdef CLASS_METHOD_OBSOLETE(c,methodname,flags)
    cdef CLASS_METHOD_RENAMED(c,oldname,newname,flags)
    cdef CLASS_METHOD_INTRODUCED(c,methodname,flags,versionstr)
    # cdef OBJ_ATTR_ATTR_ATOMS
    # cdef OBJ_ATTR_ATTR_PARSE
    # cdef OBJ_ATTR_ATTR_FORMAT
    cdef OBJ_ATTR_DEFAULT(x,attrname,flags,parsestr)
    cdef OBJ_ATTR_SAVE(x,attrname,flags)
    cdef OBJ_ATTR_DEFAULT_SAVE(x,attrname,flags,parsestr) 
    cdef CLASS_STICKY_ATTR(c,name,flags,parsestr)
    cdef CLASS_STICKY_ATTR_CLEAR(c,name)
    cdef CLASS_STICKY_CATEGORY(c,flags,name)
    cdef CLASS_STICKY_CATEGORY_CLEAR(c)
    cdef CLASS_STICKY_METHOD(c,name,flags,parsestr)
    cdef CLASS_STICKY_METHOD_CLEAR(c,name)
    cdef int OBEX_UTIL_MAX_ATOM_GETBYTES 
    cdef int OBEX_UTIL_MAX_ATOM_STATIC
    # cdef OBEX_UTIL_ATOM_SETUP_VAR_STATIC
    # cdef OBEX_UTIL_ATOM_CLEANUP_VAR_STATIC 
    cdef OBEX_UTIL_ATOM_SETUP_ARRAY_STATIC(ac)
    cdef OBEX_UTIL_ATOM_CLEANUP_ARRAY_STATIC(ac)
    # cdef OBEX_UTIL_ATOM_SETUP_VAR_DYN
    # cdef OBEX_UTIL_ATOM_CLEANUP_VAR_DYN
    cdef OBEX_UTIL_ATOM_SETUP_ARRAY_DYN(ac)
    cdef OBEX_UTIL_ATOM_CLEANUP_ARRAY_DYN(ac)
    # cdef OBEX_UTIL_ATOM_SETUP_VAR_COMBO
    # cdef OBEX_UTIL_ATOM_CLEANUP_VAR_COMBO
    cdef OBEX_UTIL_ATOM_SETUP_ARRAY_COMBO(ac)
    cdef OBEX_UTIL_ATOM_CLEANUP_ARRAY_COMBO(ac)
    # cdef OBEX_UTIL_ATOM_SETUP_VAR
    # cdef OBEX_UTIL_ATOM_CLEANUP_VAR
    # cdef OBEX_UTIL_ATOM_SETUP_ARRAY
    # cdef OBEX_UTIL_ATOM_CLEANUP_ARRAY
    ctypedef enum e_max_atom_gettext_flags:
        OBEX_UTIL_ATOM_GETTEXT_DEFAULT =            0x00000000 
        OBEX_UTIL_ATOM_GETTEXT_TRUNCATE_ZEROS =     0x00000001 
        OBEX_UTIL_ATOM_GETTEXT_SYM_NO_QUOTE =       0x00000002 
        OBEX_UTIL_ATOM_GETTEXT_SYM_FORCE_QUOTE =    0x00000004 
        OBEX_UTIL_ATOM_GETTEXT_COMMA_DELIM =        0x00000008 
        OBEX_UTIL_ATOM_GETTEXT_FORCE_ZEROS =        0x00000010 
        OBEX_UTIL_ATOM_GETTEXT_NUM_HI_RES =         0x00000020  
        OBEX_UTIL_ATOM_GETTEXT_NUM_LO_RES =         0x00000040  

    cdef t_max_err atom_setchar_array(long ac, t_atom *av, long count, unsigned char *vals)
    cdef t_max_err atom_setlong_array(long ac, t_atom *av, long count, t_atom_long *vals)
    cdef t_max_err atom_setfloat_array(long ac, t_atom *av, long count, float *vals)
    cdef t_max_err atom_setdouble_array(long ac, t_atom *av, long count, double *vals)
    cdef t_max_err atom_setsym_array(long ac, t_atom *av, long count, t_symbol **vals)
    cdef t_max_err atom_setatom_array(long ac, t_atom *av, long count, t_atom *vals)
    cdef t_max_err atom_setobj_array(long ac, t_atom *av, long count, t_object **vals)
    cdef t_max_err atom_setparse(long *ac, t_atom **av, const char *parsestr)
    cdef t_max_err atom_setbinbuf(long *ac, t_atom **av, void *buf)
    cdef t_max_err atom_setattrval(long *ac, t_atom **av, t_symbol *attrname, t_object *obj)
    cdef t_max_err atom_setobjval(long *ac, t_atom **av, t_object *obj)
    cdef t_max_err atom_setformat(long *ac, t_atom **av, const char *fmt, ...) 
    #cdef t_max_err atom_setformat_va(long *ac, t_atom **av, const char *fmt, va_list args)
    cdef t_max_err atom_getformat(long ac, t_atom *av, const char *fmt, ...)
    #cdef t_max_err atom_getformat_va(long ac, t_atom *av, const char *fmt, va_list args)
    cdef t_max_err atom_gettext(long ac, t_atom *av, long *textsize, char **text, long flags)
    cdef t_max_err atom_getchar_array(long ac, t_atom *av, long count, unsigned char *vals)
    cdef t_max_err atom_getlong_array(long ac, t_atom *av, long count, t_atom_long *vals)
    cdef t_max_err atom_getfloat_array(long ac, t_atom *av, long count, float *vals)
    cdef t_max_err atom_getdouble_array(long ac, t_atom *av, long count, double *vals)
    cdef t_max_err atom_getsym_array(long ac, t_atom *av, long count, t_symbol **vals)
    cdef t_max_err atom_getatom_array(long ac, t_atom *av, long count, t_atom *vals)
    cdef t_max_err atom_getobj_array(long ac, t_atom *av, long count, t_object **vals)
    cdef long atomisstring(const t_atom *a)
    cdef long atomisatomarray(t_atom *a)
    cdef long atomisdictionary(t_atom *a)
    cdef OB_MSG(x,p)
    cdef t_max_err object_method_parse(t_object *x, t_symbol *s, const char *parsestr, t_atom *rv)
    cdef t_max_err object_method_binbuf(t_object *x, t_symbol *s, void *buf, t_atom *rv)
    cdef t_max_err object_method_attrval(t_object *x, t_symbol *s, t_symbol *attrname, t_object *obj, t_atom *rv)
    cdef t_max_err object_method_objval(t_object *x, t_symbol *s, t_object *obj, t_atom *rv)
    cdef t_max_err object_method_format(t_object *x, t_symbol *s, t_atom *rv, const char *fmt, ...)
    cdef t_max_err object_method_char(t_object *x, t_symbol *s, unsigned char v, t_atom *rv)
    cdef t_max_err object_method_long(t_object *x, t_symbol *s, long v, t_atom *rv)
    cdef t_max_err object_method_float(t_object *x, t_symbol *s, float v, t_atom *rv)
    cdef t_max_err object_method_double(t_object *x, t_symbol *s, double v, t_atom *rv)
    cdef t_max_err object_method_sym(t_object *x, t_symbol *s, t_symbol *v, t_atom *rv)
    cdef t_max_err object_method_obj(t_object *x, t_symbol *s, t_object *v, t_atom *rv)
    cdef t_max_err object_method_char_array(t_object *x, t_symbol *s, long ac, unsigned char *av, t_atom *rv)
    cdef t_max_err object_method_long_array(t_object *x, t_symbol *s, long ac, t_atom_long *av, t_atom *rv)
    cdef t_max_err object_method_float_array(t_object *x, t_symbol *s, long ac, float *av, t_atom *rv)
    cdef t_max_err object_method_double_array(t_object *x, t_symbol *s, long ac, double *av, t_atom *rv)
    cdef t_max_err object_method_sym_array(t_object *x, t_symbol *s, long ac, t_symbol **av, t_atom *rv)
    cdef t_max_err object_method_obj_array(t_object *x, t_symbol *s, long ac, t_object **av, t_atom *rv)
    cdef t_max_err call_method_typed(method m, t_object *x, t_symbol *s, long ac, t_atom *av, t_atom *rv)
    cdef t_max_err call_method_parse(method m, t_object *x, t_symbol *s, char *parsestr, t_atom *rv)
    cdef t_max_err call_method_binbuf(method m, t_object *x, t_symbol *s, void *buf, t_atom *rv)
    cdef t_max_err call_method_attrval(method m, t_object *x, t_symbol *s, t_symbol *attrname, t_object *obj, t_atom *rv)
    cdef t_max_err call_method_objval(method m, t_object *x, t_symbol *s, t_object *obj, t_atom *rv)
    cdef t_max_err call_method_format(method m, t_object *x, t_symbol *s, t_atom *rv, char *fmt, ...)
    cdef t_max_err call_method_char(method m, t_object *x, t_symbol *s, unsigned char v, t_atom *rv)
    cdef t_max_err call_method_long(method m, t_object *x, t_symbol *s, long v, t_atom *rv)
    cdef t_max_err call_method_float(method m, t_object *x, t_symbol *s,float v, t_atom *rv)
    cdef t_max_err call_method_double(method m, t_object *x, t_symbol *s, double v, t_atom *rv)
    cdef t_max_err call_method_sym(method m, t_object *x, t_symbol *s, t_symbol *v, t_atom *rv)
    cdef t_max_err call_method_obj(method m, t_object *x, t_symbol *s, t_object *v, t_atom *rv)
    cdef t_max_err call_method_char_array(method m, t_object *x, t_symbol *s, long ac, unsigned char *av, t_atom *rv)
    cdef t_max_err call_method_long_array(method m, t_object *x, t_symbol *s, long ac, t_atom_long *av, t_atom *rv)
    cdef t_max_err call_method_float_array(method m, t_object *x, t_symbol *s, long ac, float *av, t_atom *rv)
    cdef t_max_err call_method_double_array(method m, t_object *x, t_symbol *s, long ac, double *av, t_atom *rv)
    cdef t_max_err call_method_sym_array(method m, t_object *x, t_symbol *s, long ac, t_symbol **av, t_atom *rv)
    cdef t_max_err call_method_obj_array(method m, t_object *x, t_symbol *s, long ac, t_object **av, t_atom *rv)
    cdef t_max_err object_attr_setparse(t_object *x, t_symbol *s, const char *parsestr)
    cdef t_max_err object_attr_setbinbuf(t_object *x, t_symbol *s, void *buf)
    cdef t_max_err object_attr_setattrval(t_object *x, t_symbol *s, t_symbol *attrname, t_object *obj)
    cdef t_max_err object_attr_setobjval(t_object *x, t_symbol *s, t_object *obj)
    cdef t_max_err object_attr_setformat(t_object *x, t_symbol *s, const char *fmt, ...)
    cdef t_object *attribute_new_atoms(const char *attrname, t_symbol *type, long flags, long ac, t_atom *av)
    cdef t_object *attribute_new_parse(const char *attrname, t_symbol *type, long flags, const char *parsestr)
    cdef t_object *attribute_new_binbuf(const char *attrname, t_symbol *type, long flags, void *buf)
    cdef t_object *attribute_new_attrval(const char *attrname, t_symbol *type, long flags, t_symbol *objattrname, t_object *obj)
    cdef t_object *attribute_new_objval(const char *attrname, t_symbol *type, long flags, t_object *obj)
    cdef t_object *attribute_new_format(const char *attrname, t_symbol *type, long flags, const char *fmt, ...)
    cdef void *object_new_parse(t_symbol *name_space, t_symbol *classname, const char *parsestr)
    cdef void *object_new_binbuf(t_symbol *name_space, t_symbol *classname, void *buf)
    cdef void *object_new_attrval(t_symbol *name_space, t_symbol *classname, t_symbol *objattrname, t_object *obj)
    cdef void *object_new_objval(t_symbol *name_space, t_symbol *classname, t_object *obj)
    cdef void *object_new_format(t_symbol *name_space, t_symbol *classname, const char *fmt, ...)    
    cdef t_max_err object_attr_addattr(t_object *x, t_symbol *attrname, t_object *attr)
    cdef t_object *object_attr_attr_get(t_object *x, t_symbol *attrname, t_symbol *attrname2)
    cdef t_max_err object_attr_attr_setvalueof(t_object *x, t_symbol *attrname, t_symbol *attrname2, long argc, t_atom *argv)
    cdef t_max_err object_attr_attr_getvalueof(t_object *x, t_symbol *attrname, t_symbol *attrname2, long *argc, t_atom **argv)
    cdef t_max_err class_attr_addattr(t_class *c, t_symbol *attrname, t_object *attr)
    cdef t_object *class_attr_attr_get(t_class *c, t_symbol *attrname, t_symbol *attrname2)
    cdef t_max_err class_attr_attr_setvalueof(t_class *c, t_symbol *attrname, t_symbol *attrname2, long argc, t_atom *argv)
    cdef t_max_err class_attr_attr_getvalueof(t_class *c, t_symbol *attrname, t_symbol *attrname2, long *argc, t_atom **argv)
    cdef t_max_err object_attr_enforcelocal(t_object *x, t_symbol *attrname)
    cdef t_max_err class_addattr_atoms(t_class *c, const char *attrname, t_symbol *type, long flags, long ac, t_atom *av)
    cdef t_max_err class_addattr_parse(t_class *c, const char *attrname, t_symbol *type, long flags, const char *parsestr)
    cdef t_max_err class_addattr_format(t_class *c, const char *attrname, t_symbol *type, long flags, const char *fmt, ...)
    cdef t_max_err class_attr_addattr_atoms(t_class *c, const char *attrname, const char *attrname2, t_symbol *type, long flags, long ac, t_atom *av)
    cdef t_max_err class_attr_addattr_parse(t_class *c, const char *attrname, const char *attrname2, t_symbol *type, long flags, const char *parsestr)
    cdef t_max_err class_attr_addattr_format(t_class *c, const char *attrname, const char *attrname2, const t_symbol *type, long flags, const char *fmt, ...)
    cdef t_max_err object_addattr_atoms(t_object *x, const char *attrname, t_symbol *type, long flags, long ac, t_atom *av)
    cdef t_max_err object_addattr_parse(t_object *x, const char *attrname, t_symbol *type, long flags, const char *parsestr)
    cdef t_max_err object_addattr_format(t_object *x, const char *attrname, t_symbol *type, long flags, const char *fmt, ...)
    cdef t_max_err object_attr_addattr_atoms(t_object *x, const char *attrname, const char *attrname2, t_symbol *type, long flags, long ac, t_atom *av)
    cdef t_max_err object_attr_addattr_parse(t_object *x, const char *attrname, const char *attrname2, t_symbol *type, long flags, const char *parsestr)
    cdef t_max_err object_attr_addattr_format(t_object *x, const char *attrname, const char *attrname2, t_symbol *type, long flags, const char *fmt, ...)
    cdef t_object *object_clone(t_object *x)
    cdef t_object *object_clone_generic(t_object *x)
    cdef void object_zero(t_object *x)
    cdef t_max_err class_addcommand(t_class *c, method cmd, method enabler, method handler, const char *message)
    cdef void *object_commandenabled(t_object *o, t_symbol *cmd)
    cdef t_max_err object_getenabler(t_object *c, t_symbol *cmd, method *m)
    cdef t_max_err object_getcommand(t_object *o, t_symbol *cmd, method *m)
    cdef void *object_handlecommand(t_object *o, t_symbol *s, long argc, t_atom *argv, t_atom *rv)
    cdef t_ptr_int object_attr_getdisabled(t_object *o, t_symbol *attrname)
    cdef t_max_err object_attr_setdisabled(t_object *o, t_symbol *attrname, long way)
    cdef t_max_err object_replaceargs(t_object *x, long argc, t_atom *argv, char match, char poundfill)
    cdef t_max_err object_attr_obsolete_getter(t_object *x, t_object *attr, long *ac, t_atom **av)
    cdef t_max_err object_attr_obsolete_setter(t_object *x, t_object *attr, long ac, t_atom *av)
    cdef void object_method_obsolete(t_object *x, t_symbol *s, long ac, t_atom *av)


cdef extern from "ext_database.h":
    ctypedef t_object t_database
    ctypedef t_object t_db_result
    ctypedef t_object t_db_view

    cdef t_max_err db_open(t_symbol *dbname, const char *fullpath, t_database **db)
    ctypedef enum t_db_open_flags:
        DB_OPEN_FLAGS_NONE = 0
        DB_OPEN_FLAGS_READONLY = 0x01

    cdef t_max_err db_open_ext(t_symbol *dbname, const char *fullpath, t_database **db, long flags)
    cdef t_max_err db_close(t_database **db)
    cdef t_max_err db_query(t_database *db, t_db_result **dbresult, const char *sql, ...)
    cdef t_max_err db_query_direct(t_database *db, t_db_result **dbresult, const char *sql)
    cdef t_max_err db_query_silent(t_database *db, t_db_result **dbresult, const char *sql, ...)
    cdef t_max_err db_query_getlastinsertid(t_database *db, long *id)
    cdef t_max_err db_query_table_new(t_database *db, const char *tablename)
    cdef t_max_err db_query_table_addcolumn(t_database *db, const char *tablename, const char *columnname, const char *columntype, const char *flags)
    cdef t_max_err db_transaction_start(t_database *db)
    cdef t_max_err db_transaction_end(t_database *db)
    cdef t_max_err db_transaction_flush(t_database *db)
    cdef t_max_err db_view_create(t_database *db, const char *sql, t_db_view **dbview)
    cdef t_max_err db_view_remove(t_database *db, t_db_view **dbview)
    cdef t_max_err db_view_getresult(t_db_view *dbview, t_db_result **result)
    cdef t_max_err db_view_setquery(t_db_view *dbview, char *newquery)
    cdef char** db_result_nextrecord(t_db_result *result)
    cdef void db_result_reset(t_db_result *result)
    cdef void db_result_clear(t_db_result *result)
    cdef long db_result_numrecords(t_db_result *result)
    cdef long db_result_numfields(t_db_result *result)
    cdef char* db_result_fieldname(t_db_result *result, long fieldindex)
    cdef char* db_result_string(t_db_result *result, long recordindex, long fieldindex)
    cdef long db_result_long(t_db_result *result, long recordindex, long fieldindex)
    cdef float db_result_float(t_db_result *result, long recordindex, long fieldindex)
    cdef t_ptr_uint db_result_datetimeinseconds(t_db_result *result, long recordindex, long fieldindex)
    cdef void db_util_stringtodate(const char *string, t_ptr_uint *date)
    cdef void db_util_datetostring(const t_ptr_uint date, char *string)


cdef extern from "ext_default.h":
    cdef t_max_err patcher_setdefault(t_object *patcher, t_symbol *key, long argc, t_atom *argv);
    cdef t_max_err patcher_getdefault(t_object *patcher, t_symbol *key, long *argc, t_atom *argv);
    cdef t_max_err patcher_removedefault(t_object *patcher, t_symbol *key);


cdef extern from "ext_parameter.h":

    cdef long PARAMETER_METHOD_FLAG_OVERRIDE
    cdef long PARAMETER_METHOD_FLAG_PRE
    cdef long PARAMETER_METHOD_FLAG_POST
    cdef long PARAMETER_METHOD_FLAG_FULL
    cdef long PARAMETER_METHOD_FLAG_DONOTHING
    cdef long PARAMETER_GESTURE_INDEX

    ctypedef enum PARAM_TYPE:
        PARAM_TYPE_INVALID = -1
        PARAM_TYPE_FLOAT = 0
        PARAM_TYPE_INT
        PARAM_TYPE_ENUM
        PARAM_TYPE_BLOB     
        PARAM_TYPE_FILE

    ctypedef enum PARAM_TYPE_ENABLE:
        PARAM_TYPE_ENABLE_OFF   
        PARAM_TYPE_ENABLE_ON    
        PARAM_TYPE_ENABLE_BLOB  

    ctypedef enum PARAM_UNITSTYLE:
        PARAM_UNITSTYLE_INVALID = -1
        PARAM_UNITSTYLE_INT
        PARAM_UNITSTYLE_FLOAT
        PARAM_UNITSTYLE_MS
        PARAM_UNITSTYLE_HZ
        PARAM_UNITSTYLE_DB
        PARAM_UNITSTYLE_PERCENT
        PARAM_UNITSTYLE_PAN
        PARAM_UNITSTYLE_SEMITONE
        PARAM_UNITSTYLE_MIDINOTE
        PARAM_UNITSTYLE_CUSTOM 
        PARAM_UNITSTYLE_NATIVE 

    ctypedef enum PARAM_MOD:
        PARAM_MOD_INVALID = -1
        PARAM_MOD_NONE
        PARAM_MOD_UNI
        PARAM_MOD_BI
        PARAM_MOD_ADD
        PARAM_MOD_ABS

    ctypedef enum PARAM_DATA_TYPE:
        PARAM_DATA_TYPE_INVALID = -1
        PARAM_DATA_TYPE_VALUE
        PARAM_DATA_TYPE_TYPE
        PARAM_DATA_TYPE_TYPE_ENABLE
        PARAM_DATA_TYPE_ORDER
        PARAM_DATA_TYPE_LONGNAME
        PARAM_DATA_TYPE_SHORTNAME
        PARAM_DATA_TYPE_MIN
        PARAM_DATA_TYPE_MAX
        PARAM_DATA_TYPE_ENUM
        PARAM_DATA_TYPE_MODMODE
        PARAM_DATA_TYPE_MODMIN
        PARAM_DATA_TYPE_MODMAX
        PARAM_DATA_TYPE_INITIAL_ENABLE
        PARAM_DATA_TYPE_INITIAL
        PARAM_DATA_TYPE_INITIAL_EDITABLE
        PARAM_DATA_TYPE_UNITSTYLE
        PARAM_DATA_TYPE_EXPONENT
        PARAM_DATA_TYPE_STEPS
        PARAM_DATA_TYPE_SPEEDLIM
        PARAM_DATA_TYPE_SMOOTHING 
        PARAM_DATA_TYPE_UNITS
        PARAM_DATA_TYPE_INFO
        PARAM_DATA_TYPE_FOCUS 
        PARAM_DATA_TYPE_INVISIBLE
        PARAM_DATA_TYPE_AUTOMATION_VALUE
        PARAM_DATA_TYPE_AUTOMATION_STATE
        PARAM_DATA_TYPE_MODULATION_VALUE
        PARAM_DATA_TYPE_DIRTY
        PARAM_DATA_TYPE_ASSIGNMENT_TEXT_MIDI
        PARAM_DATA_TYPE_ASSIGNMENT_TEXT_KEY
        PARAM_DATA_TYPE_ASSIGNMENT_TEXT_MACRO
        PARAM_DATA_TYPE_LEARNING_MODE
        PARAM_DATA_TYPE_FILEREF_PATH
        PARAM_DATA_TYPE_FILEREF_DISPLAYPATH
        PARAM_DATA_TYPE_FILEREF_DISPLAYNAME
        PARAM_DATA_TYPE_DEVICESTATE 
        PARAM_DATA_TYPE_DEFER
        PARAM_DATA_TYPE_MAPPING_INDEX
        PARAM_DATA_TYPE_NOBLOBCACHE

    ctypedef enum PARAM_VALUE_SET_TYPE:
        PARAM_VALUE_SET_DISTANCE = 0
        PARAM_VALUE_SET_REAL
        PARAM_VALUE_SET_GETONLY
        PARAM_VALUE_SET_DISTANCE_NONOTIFY
        PARAM_VALUE_SET_REAL_NONOTIFY

    ctypedef enum PARAM_VALUE_GET_TYPE:
        PARAM_VALUE_GET_OUTPUT = 0
        PARAM_VALUE_GET_AUTOMATION
        PARAM_VALUE_GET_MODULATION

    ctypedef enum PARAM_VALUE_FORMAT:
        PARAM_VALUE_DISTANCE = 0
        PARAM_VALUE_LINEAR
        PARAM_VALUE_REAL

    ctypedef enum PARAM_AUTOMATION_STATE:
        PARAM_AUTOMATION_STATE_NONE         = 0x00
        PARAM_AUTOMATION_STATE_AUTOMATED    = 0x01
        PARAM_AUTOMATION_STATE_AUTOENABLED  = 0x02
        PARAM_AUTOMATION_STATE_IRRELEVANT   = 0x04
        PARAM_AUTOMATION_STATE_DISABLED     = 0x08
        PARAM_AUTOMATION_STATE_MACRO        = 0x10

    ctypedef enum PARAM_DEVICESTATE:
        PARAM_DEVICESTATE_ACTIVE            = 0
        PARAM_DEVICESTATE_INACTIVE          = 1

    ctypedef enum PARAM_LEARNING_TYPE:
        PARAM_LEARNING_TYPE_NONE = 0
        PARAM_LEARNING_TYPE_MIDI
        PARAM_LEARNING_TYPE_KEY
        PARAM_LEARNING_TYPE_MACRO

    ctypedef struct t_parameter_notify_data

    ctypedef struct t_param_class_defcolor_data
    
    cdef t_max_err class_parameter_init(t_class *c)
    cdef t_max_err object_parameter_init(t_object *x, PARAM_TYPE type)
    cdef t_max_err object_parameter_dictionary_process(t_object *x, t_dictionary *d)
    cdef t_max_err object_parameter_free(t_object *x)
    cdef t_bool object_parameter_notify(t_object *x, t_symbol *s, t_symbol *msg, void *sender, void *data, t_parameter_notify_data *pnd)
    cdef t_max_err object_parameter_getinfo(t_object *x, PARAM_DATA_TYPE type, long *ac, t_atom **av)
    cdef t_max_err object_parameter_setinfo(t_object *x, PARAM_DATA_TYPE type, long ac, t_atom *av)
    cdef t_max_err object_parameter_string_get(t_object *x, double val, char **outstr)
    cdef t_max_err object_parameter_stringtovalue(t_object *x, double *value, char *str)
    cdef t_max_err object_parameter_value_set(t_object *x, PARAM_VALUE_SET_TYPE how, double *linear, double *real, char blobnotify)
    cdef t_max_err object_parameter_value_get(t_object *x, PARAM_VALUE_GET_TYPE how, PARAM_VALUE_FORMAT what, double *value)
    cdef t_max_err object_parameter_current_to_initial(t_object *x)
    # cdef t_max_err object_parameter_color_get(t_object *x, t_symbol *s, t_jrgba *jrgba)
    cdef t_max_err object_parameter_value_setvalueof(t_object *x, long ac, t_atom *av, char blobnotify)
    cdef t_max_err object_parameter_value_setvalueof_nonotify(t_object *x, long ac, t_atom *av, char blobnotify)
    cdef t_max_err object_parameter_value_getvalueof(t_object *x, long *ac, t_atom **av)
    cdef t_max_err object_parameter_value_changed(t_object *x, char blobnotify)
    cdef t_max_err object_parameter_value_changed_nonotify(t_object *x, char blobnotify)
    cdef t_max_err class_parameter_addmethod(t_class *c, method m, char *name, long flags, ...)
    cdef t_max_err parameter_default_int(t_object *x, long n)
    cdef t_max_err parameter_default_float(t_object *x, double d)
    cdef t_max_err parameter_default_anything(t_object *x, t_symbol *s, long ac, t_atom *av)
    cdef t_max_err class_parameter_register_default_color(t_class *c, t_symbol *attrname, t_symbol *colorname)
    cdef t_bool object_parameter_is_initialized(t_object *x)
    cdef t_bool object_parameter_is_in_Live(t_object *x)
    cdef t_bool object_parameter_is_in_maxtilde(t_object *x)
    cdef t_bool object_parameter_is_automated(t_object *x)
    cdef t_max_err object_parameter_wants_focus(t_object *x)
    cdef t_bool object_parameter_is_parameter(t_object *x)
    cdef t_atom_long object_parameter_get_order(t_object *x)

    cdef t_symbol *ps_surface_bg
    cdef t_symbol *ps_control_bg         
    cdef t_symbol *ps_control_text_bg    
    cdef t_symbol *ps_control_fg         
    cdef t_symbol *ps_control_fg_on      
    cdef t_symbol *ps_control_fg_off     
    cdef t_symbol *ps_control_selection  
    cdef t_symbol *ps_control_zombie     
    cdef t_symbol *ps_value_arc
    cdef t_symbol *ps_value_bar
    cdef t_symbol *ps_active_automation  
    cdef t_symbol *ps_inactive_automation
    cdef t_symbol *ps_macro_assigned     
    cdef t_symbol *ps_contrast_frame     
    cdef t_symbol *ps_key_assignment     
    cdef t_symbol *ps_midi_assignment    
    cdef t_symbol *ps_macro_assignment   
    cdef t_symbol *ps_assignment_text_bg 
    cdef t_symbol *ps_control_fg_zombie  
    cdef t_symbol *ps_value_arc_zombie   
    cdef t_symbol *ps_numbox_triangle    
    cdef t_symbol *ps_macro_title        
    cdef t_symbol *ps_selection
    cdef t_symbol *ps_led_bg
    cdef char *PARAM_COLOR_SURFACE_BG
    cdef char *PARAM_COLOR_CONTROL_BG
    cdef char *PARAM_COLOR_CONTROL_TEXT_BG
    cdef char *PARAM_COLOR_CONTROL_FG
    cdef char *PARAM_COLOR_CONTROL_FG_ON
    cdef char *PARAM_COLOR_CONTROL_FG_OFF
    cdef char *PARAM_COLOR_CONTROL_SELECTION
    cdef char *PARAM_COLOR_CONTROL_ZOMBIE
    cdef char *PARAM_COLOR_VALUE_ARC
    cdef char *PARAM_COLOR_VALUE_BAR
    cdef char *PARAM_COLOR_ACTIVE_AUTOMATION
    cdef char *PARAM_COLOR_INACTIVE_AUTOMATION
    cdef char *PARAM_COLOR_MACRO_ASSIGNED
    cdef char *PARAM_COLOR_CONTRAST_FRAME
    cdef char *PARAM_COLOR_KEY_ASSIGNMENT
    cdef char *PARAM_COLOR_MIDI_ASSIGNMENT
    cdef char *PARAM_COLOR_MACRO_ASSIGNMENT
    cdef char *PARAM_COLOR_ASSIGNMENT_TEXT_BG
    cdef char *PARAM_COLOR_CONTROL_FG_ZOMBIE
    cdef char *PARAM_COLOR_VALUE_ARC_ZOMBIE
    cdef char *PARAM_COLOR_NUMBOX_TRIANGLE
    cdef char *PARAM_COLOR_MACRO_TITLE
    cdef char *PARAM_COLOR_SELECTION
    cdef char *PARAM_COLOR_LED_BG
    cdef int PARAM_COLOR_COUNT


cdef extern from "ext_sysmidi.h":

    ctypedef enum:
        SYSMIDI_ENABLED = 1
        SYSMIDI_DYNAMIC = 2
        SYSMIDI_PERMANENT = 4

    ctypedef struct t_midistate
    ctypedef struct t_midiportinfo
    ctypedef struct t_sysmididriver

    cdef void sysmidi_enqbigpacket(t_midiportinfo *port, t_uint8 *data, double ts, long len, long contFlags)
    cdef long sysmidi_numinports()
    cdef long sysmidi_numoutports()
    cdef t_symbol *sysmidi_indextoname(long index, long io)
    cdef void sysmidi_iterate(method meth, void *arg, long io)
    cdef t_midiportinfo *sysmidi_createport(long id, long refnum, t_symbol *name, t_sysmididriver *dx, long io, long flags)
    cdef void sysmidi_deletemarked(long io)
    cdef t_midiportinfo *sysmidi_idtoport(long id, long io)
    cdef long sysmidi_uniqueid()
    cdef t_midiportinfo *sysmidi_data1toport(void *data, long io)
    cdef t_midiportinfo *sysmidi_nametoport(t_symbol *name, long io)


cdef extern from "ext_itm.h":


    ctypedef t_object t_itm
    ctypedef struct t_clocksource
    ctypedef struct t_tschange

    ctypedef enum:
        TIME_FLAGS_LOCATION = 1     
        TIME_FLAGS_TICKSONLY = 2        
        TIME_FLAGS_FIXEDONLY = 4        
        TIME_FLAGS_LOOKAHEAD = 8        
        TIME_FLAGS_USECLOCK = 16        
        TIME_FLAGS_USEQELEM = 32        
        TIME_FLAGS_FIXED = 64           
        TIME_FLAGS_PERMANENT = 128      
        TIME_FLAGS_TRANSPORT = 256      
        TIME_FLAGS_EVENTLIST = 512      
        TIME_FLAGS_CHECKSCHEDULE = 1024
        TIME_FLAGS_LISTENTICKS = 2048   
        TIME_FLAGS_NOUNITS = 4096       
        TIME_FLAGS_BBUSOURCE = 8192 
        TIME_FLAGS_POSITIVE = 16384         

    cdef void *itm_getglobal()
    cdef void *itm_getnamed(t_symbol *s, void *scheduler, t_symbol *defaultclocksourcename, long create)
    cdef void *itm_getfromarg(t_object *o, t_symbol *s)
    cdef void itm_reference(t_itm *x)
    cdef void itm_dereference(t_itm *x)
    cdef void itm_deleteeventlist(t_itm *x, t_symbol *eventlist)
    cdef void itm_eventlistseek(t_itm *x)
    cdef void itm_geteventlistnames(t_itm *x, long *count, t_symbol ***names)
    cdef void itm_switcheventlist(t_itm *x, t_symbol *eventlist, double offset)
    cdef double itm_gettime(t_itm *x)
    cdef double itm_getticks(t_itm *x)
    cdef void itm_dump(t_itm *x)
    cdef void itm_sync(t_itm *x)
    cdef void itm_settimesignature(t_itm *x, long num, long denom, long flags)
    cdef void itm_gettimesignature(t_itm *x, long *num, long *denom)
    cdef void itm_seek(t_itm *x, double oldticks, double newticks, long chase)
    cdef void itm_pause(t_itm *x)
    cdef void itm_resume(t_itm *x)
    cdef long itm_getstate(t_itm *x)
    cdef void itm_setresolution(double res)
    cdef double itm_getresolution()
    cdef t_symbol *itm_getname(t_itm *x)
    cdef t_max_err itm_parse(t_itm *x, long argc, t_atom *argv, long flags, double *ticks, double *fixed, t_symbol **unit, long *bbu, char *bbusource)
    cdef double itm_tickstoms(t_itm *x, double ticks)
    cdef double itm_mstoticks(t_itm *x, double ms)
    cdef double itm_mstosamps(t_itm *x, double ms)
    cdef double itm_sampstoms(t_itm *x, double samps)
    cdef void itm_barbeatunitstoticks(t_itm *x, long bars, long beats, double units, double *ticks, char position)
    cdef void itm_tickstobarbeatunits(t_itm *x, double ticks, long *bars, long *beats, double *units, char position)
    cdef void itm_format(t_itm *x, double ms, double ticks, long flags, t_symbol *unit, long *argc, t_atom **argv)
    cdef long itm_isunitfixed(t_symbol *u)
    cdef void itmclock_delay(t_object *x, t_itm *m, t_symbol *eventlist, double delay, long quantization)
    cdef void *itmclock_new(t_object *owner, t_object *timeobj, method task, method killer, long permanent)
    cdef void itmclock_set(t_object *x, t_itm *m, t_symbol *eventlist, double time)
    cdef void itmclock_unset(t_object *x)
    cdef void *itm_clocksource_getnamed(t_symbol *name, long create)
    cdef void itm_getclocksources(long *count, t_symbol ***sources)
    cdef double itm_getsr(t_itm *x)
    cdef double itm_gettempo(t_itm *x)




# cdef extern from "ext_boxstyle.h":

#     cdef void class_attr_setstyle(t_class *c, const char *s)
#     cdef void class_attr_style_alias(t_class *c, const char *name, const char *aliasname, long legacy)
#     cdef int FILL_ATTR_SAVE
#     cdef void class_attr_setfill(t_class *c, const char *name, long flags)
#     cdef void jgraphics_attr_fillrect(t_object *b, t_jgraphics *g, t_symbol *attrname, t_rect *area)
#     cdef t_jpattern *jgraphics_attr_setfill(t_object *b, t_jgraphics *g, t_symbol *attrname, t_rect *area)
#     cdef void object_attr_getfillcolor_atposition(t_object *b, const char *attrname, double pos, t_jrgba *c)
#     cdef long object_attr_getfill(t_object *obj, t_symbol *attrname)
#     cdef void object_style_setfillattribute(t_object *x, t_symbol *fillattr, t_symbol *entry, long argc, t_atom *argv)
#     cdef void class_attr_stylemap(t_class *c, char *attrname, char *mapname)
#     cdef t_symbol *object_attr_attrname_forstylemap(t_object *x, t_symbol *mapname)     
#     cdef t_symbol *object_attr_stylemapname(t_object *x, t_symbol *attrname)            
#     cdef t_jpopupmenu *style_getmenu(t_object *context, t_symbol *current, long mask, long *selecteditem, long *headercount)        
#     cdef void style_handlemenu(t_object *context, long itemindex, t_symbol **current)

#     cdef void CLASS_ATTR_STYLE_RGBA_NOSAVE(c,attrname,flags,structname,structmember,label)
#     cdef void CLASS_ATTR_STYLE_RGBA(c,attrname,flags,structname,structmember,label)
#     cdef void CLASS_ATTR_STYLE_RGBA_PREVIEW(c,attrname,flags,structname,structmember,label,previewtype)
#     cdef void CLASS_ATTR_STYLE_ALIAS_NOSAVE(c,attrname,aliasname)
#     cdef void CLASS_ATTR_STYLE_ALIAS_COMPATIBILITY(c,attrname,aliasname)
#     cdef void CLASS_ATTR_STYLE_ALIAS_RGBA_LEGACY(c,attrname,aliasname)

txt = 'Hello from Max!'

cpdef public str hello():
    return txt

cpdef public void py_post(str s):
    post(s)


