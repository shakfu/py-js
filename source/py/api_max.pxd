# api_max.pyx
"""
'x' -> the header has been fully exposed to cython

'-' -> the header is not explicitly exposed to cython and presently
       not required for the external. It is exposed to non-cython c code
       via the primary includes in "ext.h"

'p' -> partial analyzed but not yet included in api.pxd

' ' -> an empty box means it is planned

- [x] commonsyms.h
- [ ] ext.h
- [x] ext_atomarray.h
- [x] ext_atombuf.h
- [-] ext_atomic.h
- [x] ext_backgroundtask.h
- [p] ext_boxstyle.h
- [-] ext_byteorder.h
- [-] ext_charset.h
- [-] ext_common.h
- [-] ext_critical.h
- [x] ext_database.h
- [x] ext_default.h
- [x] ext_dictionary.h
- [x] ext_dictobj.h
- [-] ext_drag.h
- [x] ext_expr.h
- [x] ext_globalsymbol.h
- [x] ext_hashtab.h
- [x] ext_itm.h
- [x] ext_linklist.h
- [x] ext_maxtypes.h
- [x] ext_mess.h
- [x] ext_obex.h
- [x] ext_obex_util.h
- [-] ext_obstring.h
- [x] ext_packages.h
- [x] ext_parameter.h
- [x] ext_path.h
- [x] ext_preferences.h
- [-] ext_prefix.h
- [-] ext_preprocessor.h
- [x] ext_proto.h
- [-] ext_proto_win.h
- [ ] ext_qtimage.h
- [ ] ext_qtstubs.h
- [-] ext_quickmap.h
- [ ] ext_sndfile.h
- [x] ext_strings.h
- [x] ext_symobject.h
- [x] ext_sysfile.h
- [x] ext_sysmem.h
- [x] ext_sysmidi.h
- [x] ext_sysparallel.h
- [x] ext_sysprocess.h
- [x] ext_syssem.h
- [x] ext_sysshmem.h
- [x] ext_systhread.h
- [x] ext_systime.h
- [x] ext_time.h
- [x] ext_wind.h
- [-] ext_xmltree.h
- [-] indexmap.h
- [x] jdataview.h
- [p] jgraphics.h
- [x] jpatcher_api.h
- [-] jpatcher_syms.h
- [x] jpatcher_utils.h
- [x] max_types.h
"""

cdef extern from "ext.h":
    pass

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
    cdef long MAGIC
    cdef long OB_MAGIC
    cdef int NOGOOD(x)
    cdef int OB_INVALID(x)
    cdef int MSG_MAXARG
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
    # cdef <void*>int NIL


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


cdef extern from "ext_time.h":

    ctypedef t_object t_timeobject
    cdef void time_stop(t_timeobject *x)
    cdef void time_tick(t_timeobject *x)
    cdef double time_getms(t_timeobject *x)
    cdef double time_getticks(t_timeobject *x)
    cdef void time_getphase(t_timeobject *tx, double *phase, double *slope, double *ticks)
    cdef void time_listen(t_timeobject *x, t_symbol *attr, long flags)
    cdef void time_setvalue(t_timeobject *tx, t_symbol *s, long argc, t_atom *argv)
    cdef void class_time_addattr(t_class *c, const char *attrname, const char *attrlabel, long flags)
    cdef void *time_new(t_object *owner, t_symbol *attrname, method tick, long flags)
    cdef t_object *time_getnamed(t_object *owner, t_symbol *attrname)
    cdef void time_enable_attributes(t_object *x)
    cdef long time_isfixedunit(t_timeobject *x)
    cdef void time_schedule(t_timeobject *x, t_timeobject *quantize)
    cdef void time_schedule_limit(t_timeobject *x, t_timeobject *quantize)
    cdef void time_now(t_timeobject *x, t_timeobject *quantize)
    cdef void *time_getitm(t_timeobject *ox)
    cdef double time_calcquantize(t_timeobject *ox, t_itm *vitm, t_timeobject *oq)
    cdef void time_setclock(t_timeobject *tx, t_symbol *sc)


cdef extern from "ext_packages.h":

    ctypedef struct t_package_file
    cdef short packages_getpackagepath(const char *packagename)
    cdef t_linklist *packages_createsubpathlist(const char *subfoldername, short includesysfolder)
    cdef t_max_err packages_getsubpathcontents(const char *subfoldername, const char *suffix_selector, short includesysfolder, t_linklist **subpathlist, t_dictionary **names_to_packagefiles)


cdef extern from "ext_path.h":

    cdef int MAX_PATH_CHARS
    cdef int MAX_FILENAME_CHARS
    ctypedef short FILE_REF
    cdef char PATH_SEPARATOR_CHAR
    cdef char PATH_SEPARATOR_STRING
    cdef char SEPARATOR_CHAR

    ctypedef enum e_max_path_styles:
        PATH_STYLE_MAX = 0
        PATH_STYLE_NATIVE
        PATH_STYLE_COLON
        PATH_STYLE_SLASH
        PATH_STYLE_NATIVE_WIN       


    cdef int PATH_STYLE_MAX_PLAT
    cdef int PATH_STYLE_NATIVE_PLAT

    ctypedef enum e_max_path_types:
        PATH_TYPE_IGNORE = 0
        PATH_TYPE_ABSOLUTE  
        PATH_TYPE_RELATIVE  
        PATH_TYPE_BOOT      
        PATH_TYPE_C74       
        PATH_TYPE_PATH      
        PATH_TYPE_DESKTOP   
        PATH_TYPE_TILDE 
        PATH_TYPE_TEMPFOLDER
        PATH_TYPE_MAXDB

    cdef int PATH_CHAR_IS_SEPARATOR(c)

    ctypedef enum e_max_fileinfo_flags: 
        PATH_FILEINFO_ALIAS = 1
        PATH_FILEINFO_FOLDER = 2
        PATH_FILEINFO_PACKAGE = 4   

    cdef int FILEINFO_ALIAS 
    cdef int FILEINFO_FOLDER

    ctypedef enum e_max_path_folder_flags:
        PATH_REPORTPACKAGEASFOLDER = 1
        PATH_FOLDER_SNIFF = 2   
        PATH_NOALIASRESOLUTION = 4      


    ctypedef enum e_max_openfile_permissions:
        PATH_READ_PERM = 1
        PATH_WRITE_PERM = 2
        PATH_RW_PERM = 3    


    cdef int READ_PERM
    cdef int WRITE_PERM
    cdef int RW_PERM
    cdef int PATH_DEFAULT_PATHNAME_COUNT

    ctypedef enum e_max_path_indices:   
        PATH_STARTUP_PATH = 0
        PATH_SEARCH_PATH
        PATH_ACTION_PATH
        PATH_HELP_PATH


    cdef int STARTUP_PATH 
    cdef int SEARCH_PATH      
    cdef int ACTION_PATH
    cdef int HELP_PATH
    cdef int COLLECTIVE_FILECOPY
    cdef int COLLECTIVE_COPYTOMADEFOLDER
    cdef int TYPELIST_SIZE 

    ctypedef enum e_max_typelists:
        TYPELIST_MAXFILES       = 1
        TYPELIST_EXTERNS        = 2
        TYPELIST_COLLECTIVES    = 4
        TYPELIST_MAXFORLIVE     = 8
        TYPELIST_SNAPSHOTS      = 16
        TYPELIST_GENPATCHERS    = 32
        TYPELIST_SNIPPETS       = 64


    ctypedef struct t_fileinfo

    ctypedef struct t_path

    ctypedef struct t_pathlink

    ctypedef enum e_max_searchpath_flags:
        PATH_FLAGS_RECURSIVE    = 0x001
        PATH_FLAGS_READONLY     = 0x010

    short path_getapppath()
    short path_getsupportpath()

        
    # cdef short path_tofsref(const short path, const char *filename, FSRef *ref)
    # cdef short path_fromfsref(FSRef *ref)
    cdef void path_namefrompathname(char *pathname, char *name)
    cdef short locatefile(const char *name, short *outvol, short *binflag)
    cdef short locatefiletype(const char *name, short *outvol, t_fourcc filetype, t_fourcc creator)
    cdef short locatefilelist(char *name, short *outvol, t_fourcc *outtype, t_fourcc *filetypelist, short numtypes)
    cdef short locatefile_extended(char *name, short *outvol, t_fourcc *outtype, const t_fourcc *filetypelist, short numtypes)
    cdef short locatefile_pathlist(t_pathlink *list, char *name, short *outvol, t_fourcc *outtype, t_fourcc *filetypelist, short numtypes)
    cdef short path_resolvefile(char *name, const short path, short *outpath)
    cdef short path_fileinfo(const char *name, const short path, t_fileinfo *info)
    cdef short path_tempfolder()
    cdef short path_desktopfolder()
    cdef short path_userdocfolder()
    cdef short path_usermaxfolder()
    cdef short path_createfolder(const short path, const char *name, short *newpath)
    cdef short path_createnewfolder(short path, char *name, short *newpath)
    cdef short path_copyfile(short srcpath, char *srcname, short dstpath, char *dstname)
    cdef short path_copytotempfile(short srcpath, char *srcname, short *outpath, char *outname)
    cdef short path_copyfolder(short srcpath, short dstpath, char *dstname, long recurse, short *newpath)
    #cdef short C74_MUST_CHECK path_getpath(short path, const char *name, short *outpath)
    cdef short path_getname(short path, char *name, short *outpath)
    cdef short path_topathname(const short path, const char *file, char *name)
    cdef short path_frompathname(const char *name, short *path, char *filename)
    cdef short path_frompotentialpathname(const char *name, short *path, char *filename)
    cdef void path_splitnames(const char *pathname, char *foldername, char *filename)
    cdef short path_getnext(t_pathlink *list, short *val)
    cdef void path_setdefault(short path, short recursive)
    cdef short path_getdefault()
    # cdef void path_setdefaultlist(struct _pathlink *list)
    cdef short path_getmoddate(short path, t_ptr_uint *date)
    cdef short path_getfilemoddate(const char *filename, const short path, t_ptr_uint *date)
    cdef short path_getfiledatesandsize(const char *filename, short path, t_uint64 *create, t_uint64 *mod, t_uint64 *access, t_uint64 *size)
    cdef short path_getfilecreationdate(const char *filename, const short path, t_ptr_uint *date)
    cdef short path_getfilesize(char *filename, short path, t_ptr_size *size)
    cdef long path_listcount(t_pathlink *list)
    cdef short nameinpath(char *name, short *ref)                    
    cdef short path_nameinpath(const char *name, const short path, short *ref)
    cdef short path_sysnameinpath(char *name, short *ref)
    cdef void *path_openfolder(short path)
    cdef short path_foldernextfile(void *xx, t_fourcc *filetype, char *name, short descend)
    cdef void path_closefolder(void *x)
    cdef short path_renamefile(const char *name, const short path, const char *newname)
    cdef long path_getprefstring(long preftype, long index, t_symbol **s)
    cdef void path_setprefstring(long preftype, long index, t_symbol *s, long flags, t_symbol *label, short update)
    cdef void path_makefromsymbol(long pathtype, t_symbol *sp, short recursive)
    # cdef short path_opensysfile(const char *name, const short path, t_filehandle *ref, short perm)
    # cdef short path_createsysfile(const char *name, short path, t_fourcc type, t_filehandle *ref)
    # cdef short path_createressysfile(const char *name, const short path, t_fourcc type, t_filehandle *ref)
    cdef short path_nameconform(const char *src, char *dst, long style, long type)
    cdef short path_deletefile(const char *name, const short path)
    cdef short path_extendedfileinfo(const char *name, short path, t_fileinfo *info, const t_fourcc *typelist, short numtypes, short sniff)
    cdef short path_getstyle(char *name)
    cdef char path_getseparator(char *name)
    cdef short path_fileisresource(char *name, short path)
    cdef short path_topotentialname(const short path, const char *file, char *name, short check)
    cdef short path_topotentialunicodename(short path, char *file, unsigned short **name, long *outlen, short check)
    cdef short path_fromunicodepathname(unsigned short *name, short *path, char *filename, short check)   
    cdef t_max_err path_toabsolutesystempath(const short in_path, const char *in_filename, char *out_filepath)
    cdef t_max_err path_absolutepath(t_symbol **returned_path, const t_symbol *s, const t_fourcc *filetypelist, short numtypes)
    cdef void path_addsearchpath(short path, short parent)
    cdef void path_addnamed(long pathtype, char *name, short recursive, short permanent)
    # cdef void path_removefromlist(t_pathlink **list, short parent)
    cdef short path_collpathnamefrompath(short vol, short *collvol, char *filename)
    cdef short defvolume()           


cdef extern from "ext_preferences.h":

    t_max_err preferences_getatomforkey(t_symbol *key, t_atom *value)
    t_symbol *preferences_getsym(const char *name)
    void preferences_setsym(const char *name, t_symbol *value)
    long preferences_getlong(const char *name)
    void preferences_setlong(const char *name, long value)
    long preferences_getchar(const char *name)
    void preferences_setchar(const char *name, long value)
    t_max_err preferences_getatoms(const char *name, long *argc, t_atom **argv)
    t_max_err preferences_setatoms(const char *name, long argc, t_atom *argv)
    void *preferences_define(const char *name, const char *type, const char *label, const char *style, const char *category, long attrflags, method get, method set, long flags)
    void *preferences_class_define(t_class *c, const char *name, const char *type, const char *label, const char *style, const char *category, long attrflags, method get, method set, long flags)
    void *preferences_defineoption(const char *name, const char *label, const char *category, char *ptr, method notify, long flags)
    void *preferences_class_defineoption(t_class *c, const char *name, const char *label, const char *category, char *ptr, method notify, long flags)
    t_max_err preferences_writedictionary(const t_dictionary *d, const char *filename)
    t_max_err preferences_readdictionary(const char *filename, t_dictionary **d)
    t_dictionary *simpleprefs_dictionary()

    ctypedef enum:
        PREFERENCES_FLAGS_INVISIBLE = 1
        PREFERENCES_FLAGS_DONTSAVE = 2


cdef extern from "ext_strings.h":

    cdef char *strncpy_zero(char *dst, const char* src, long size)
    cdef char *strncat_zero(char *dst, const char* src, long size)
    cdef int snprintf_zero(char *buffer, size_t count, const char *format, ...)

    cdef int SPRINTF_MAXLEN
    cdef void ctopcpy(unsigned char *p1, char *p2)
    cdef void ptoccpy(char *p1, unsigned char *p2)
    cdef void pstrcpy(unsigned char *p2, unsigned char *p1)


cdef extern from "ext_symobject.h":

    ctypedef struct t_symobject

    cdef void symobject_initclass()
    cdef void *symobject_new(t_symbol *sym)
    cdef long symobject_linklist_match(void *a, void *b)


cdef extern from "ext_sysfile.h":

    ctypedef struct t_filestruct 
    ctypedef t_filestruct *t_filehandle

    ctypedef enum t_sysfile_pos_mode:
        SYSFILE_ATMARK = 0          
        SYSFILE_FROMSTART = 1       
        SYSFILE_FROMLEOF = 2        
        SYSFILE_FROMMARK = 3        


    ctypedef enum t_sysfile_flags:
        SYSFILE_SUBFILE = 1
        SYSFILE_HANDLE = 2      
        SYSFILE_RESOURCE = 4    
        SYSFILE_MEMORY = 6      
        SYSFILE_RESFILE = 8 
        SYSFILE_OPENRESFILE = 16    
        SYSFILE_EXTERNALDATA = 32   
        SYSFILE_JUSTAPOINTER = 64   
        SYSFILE_EXTERNALDATA_CANWRITE = 128  
        SYSFILE_EXTERNALDATA_CANGROW = 256    
        SYSFILE_EXTERNALDATA_FREE = 512       
        SYSFILE_EXTERNALDATA_LATEFREE = 1024


    ctypedef enum t_sysfile_text_flags:
        TEXT_LB_NATIVE =            0x00000001L 
        TEXT_LB_MAC =               0x00000002L 
        TEXT_LB_PC =                0x00000004L 
        TEXT_LB_UNIX =              0x00000008L 
        TEXT_LB_MASK = 0x0000000FL              
        TEXT_ENCODING_USE_FILE =    0x00000100L 
        TEXT_NULL_TERMINATE =       0x00000200L     

    cdef t_max_err sysfile_close(t_filehandle f)
    cdef t_max_err sysfile_read( t_filehandle f, t_ptr_size *count, void *bufptr)  
    cdef t_max_err sysfile_readtohandle(t_filehandle f, char ***h)
    cdef t_max_err sysfile_readtoptr(t_filehandle f, char **p)
    cdef t_max_err sysfile_write(t_filehandle f, t_ptr_size *count, const void *bufptr)  
    cdef t_max_err sysfile_seteof(t_filehandle f, t_ptr_size logeof)
    cdef t_max_err sysfile_geteof(t_filehandle f, t_ptr_size *logeof)
    cdef t_max_err sysfile_setpos(t_filehandle f, t_sysfile_pos_mode mode, t_ptr_int offset)
    cdef t_max_err sysfile_getpos(t_filehandle f, t_ptr_size *filepos)
    cdef t_max_err sysfile_spoolcopy(t_filehandle src, t_filehandle dst, t_ptr_size size)
    cdef void sysfile_setobject(t_filehandle f, t_object *o)
    cdef t_max_err sysfile_readtextfile(t_filehandle f, t_handle htext, t_ptr_size maxlen, t_sysfile_text_flags flags)
    cdef t_max_err sysfile_writetextfile(t_filehandle f, t_handle htext, t_sysfile_text_flags flags)
    cdef t_max_err sysfile_openhandle(char **h, t_sysfile_flags flags, t_filehandle *fh)
    cdef t_max_err sysfile_openptrsize(char *p, t_ptr_size length, t_sysfile_flags flags, t_filehandle *fh)


cdef extern from "ext_sysmem.h":

    extern t_ptr sysmem_newptr(long size)
    extern t_ptr sysmem_newptrclear(long size)
    extern t_ptr sysmem_resizeptr(void *ptr, long newsize)
    extern t_ptr sysmem_resizeptrclear(void *ptr, long newsize)
    extern long sysmem_ptrsize(void *ptr)
    extern void sysmem_freeptr(void *ptr)
    extern void sysmem_copyptr(const void *src, void *dst, long bytes)
    extern t_handle sysmem_newhandle(long size)
    extern t_handle sysmem_newhandleclear(unsigned long size) 
    extern long sysmem_resizehandle(t_handle handle, long newsize)
    extern long sysmem_handlesize(t_handle handle)
    extern void sysmem_freehandle(t_handle handle)
    extern long sysmem_lockhandle(t_handle handle, long lock)
    extern long sysmem_ptrandhand(void *p, t_handle h, long size)
    extern long sysmem_ptrbeforehand(void *p, t_handle h, unsigned long size)
    extern long sysmem_nullterminatehandle(t_handle h)


cdef extern from "ext_atomarray.h":

    cdef int ATOMARRAY_FLAG_FREECHILDREN

    ctypedef struct t_atomarray

    cdef t_atomarray *atomarray_new(long ac, t_atom *av)
    cdef void atomarray_flags(t_atomarray *x, long flags) 
    cdef long atomarray_getflags(t_atomarray *x) 
    cdef t_max_err atomarray_setatoms(t_atomarray *x, long ac, t_atom *av)
    cdef t_max_err atomarray_getatoms(t_atomarray *x, long *ac, t_atom **av)
    cdef t_max_err atomarray_copyatoms(t_atomarray *x, long *ac, t_atom **av)
    cdef t_atom_long atomarray_getsize(t_atomarray *x)
    cdef t_max_err atomarray_getindex(t_atomarray *x, long index, t_atom *av)
    cdef t_max_err atomarray_setindex(t_atomarray *x, long index, t_atom *av)
    cdef void *atomarray_duplicate(t_atomarray *x)
    cdef void *atomarray_clone(t_atomarray *x)
    cdef void atomarray_appendatom(t_atomarray *x, t_atom *a)
    cdef void atomarray_appendatoms(t_atomarray *x, long ac, t_atom *av)
    cdef void atomarray_chuckindex(t_atomarray *x, long index)
    cdef void atomarray_clear(t_atomarray *x)
    cdef void atomarray_funall(t_atomarray *x, method fun, void *arg)


cdef extern from "ext_systhread.h":

    ctypedef void *t_systhread
    ctypedef void *t_systhread_mutex
    ctypedef void *t_systhread_cond
    ctypedef void *t_systhread_rwlock   
    ctypedef void *t_systhread_key

    ctypedef enum e_max_systhread_mutex_flags:
        SYSTHREAD_MUTEX_NORMAL =        0x00000000
        SYSTHREAD_MUTEX_ERRORCHECK =    0x00000001  
        SYSTHREAD_MUTEX_RECURSIVE =     0x00000002  


    ctypedef enum e_max_systhread_priority:
        SYSTHREAD_PRIORITY_MIN = -30
        SYSTHREAD_PRIORITY_DEFAULT = 0
        SYSTHREAD_PRIORITY_MAX = 30
     
    ctypedef enum e_max_systhread_rwlock_flags:
        SYSTHREAD_RWLOCK_NORMAL =       0x00000000
        SYSTHREAD_RWLOCK_LITE =         0x00000001

        
    cdef long systhread_create(method entryproc, void *arg, long stacksize, long priority, long flags, t_systhread *thread)
    cdef long systhread_terminate(t_systhread thread)
    cdef void systhread_sleep(long milliseconds)
    cdef void systhread_exit(long status)
    cdef long systhread_join(t_systhread thread, unsigned int* retval)
    cdef long systhread_detach(t_systhread thread)
    cdef t_systhread systhread_self()
    cdef void systhread_setpriority(t_systhread thread, int priority) 
    cdef int systhread_getpriority(t_systhread thread) 
    cdef char *systhread_getstackbase()
    cdef void systhread_init()
    cdef void systhread_mainstacksetup()
    cdef void systhread_timerstacksetup()
    cdef short systhread_stackcheck()
    cdef short systhread_ismainthread()
    cdef short systhread_istimerthread()
    cdef short systhread_isaudiothread()
    cdef long systhread_mutex_new(t_systhread_mutex *pmutex,long flags)
    cdef long systhread_mutex_free(t_systhread_mutex pmutex)
    cdef long systhread_mutex_lock(t_systhread_mutex pmutex)
    cdef long systhread_mutex_unlock(t_systhread_mutex pmutex)
    cdef long systhread_mutex_trylock(t_systhread_mutex pmutex)
    cdef long systhread_mutex_newlock(t_systhread_mutex *pmutex,long flags)
    cdef t_max_err systhread_rwlock_new(t_systhread_rwlock *rwlock, long flags) 
    cdef t_max_err systhread_rwlock_free(t_systhread_rwlock rwlock) 
    cdef t_max_err systhread_rwlock_rdlock(t_systhread_rwlock rwlock) 
    cdef t_max_err systhread_rwlock_tryrdlock(t_systhread_rwlock rwlock) 
    cdef t_max_err systhread_rwlock_rdunlock(t_systhread_rwlock rwlock) 
    cdef t_max_err systhread_rwlock_wrlock(t_systhread_rwlock rwlock) 
    cdef t_max_err systhread_rwlock_trywrlock(t_systhread_rwlock rwlock) 
    cdef t_max_err systhread_rwlock_wrunlock(t_systhread_rwlock rwlock) 
    cdef t_max_err systhread_rwlock_setspintime(t_systhread_rwlock rwlock, double spintime_ms)
    cdef t_max_err systhread_rwlock_getspintime(t_systhread_rwlock rwlock, double *spintime_ms) 
    cdef long systhread_cond_new(t_systhread_cond *pcond, long flags)
    cdef long systhread_cond_free(t_systhread_cond pcond)
    cdef long systhread_cond_wait(t_systhread_cond pcond, t_systhread_mutex pmutex)
    cdef long systhread_cond_signal(t_systhread_cond pcond)
    cdef long systhread_cond_broadcast(t_systhread_cond pcond)
    cdef long systhread_key_create(t_systhread_key *key, void (*destructor)(void*))
    cdef long systhread_key_delete(t_systhread_key key) 
    cdef void* systhread_getspecific(t_systhread_key key)
    cdef long systhread_setspecific(t_systhread_key key, const void *value) 


cdef extern from "ext_sysparallel.h":

    cdef int SYSPARALLEL_PRIORITY_DEFAULT
    cdef int SYSPARALLEL_PRIORITY_LOW
    cdef int SYSPARALLEL_PRIORITY_MEDIUM
    cdef int SYSPARALLEL_PRIORITY_HIGH
    cdef int SYSPARALLEL_PRIORITY_TASK_LOCAL
    cdef int SYSPARALLEL_MAX_WORKERS
    cdef int SYSPARALLEL_STATE_IDLE
    cdef int SYSPARALLEL_STATE_RUN
    cdef int SYSPARALLEL_STATE_DONE
    cdef int SYSPARALLEL_STATE_QUIT
    cdef int SYSPARALLEL_TASK_FLAG_WORKERTRIGGERS
            
    ctypedef struct t_sysparallel_task
    ctypedef struct t_sysparallel_worker
    cdef void sysparallel_init()
    cdef long sysparallel_processorcount()
    cdef t_sysparallel_task *sysparallel_task_new(void *data, method workerproc, long maxworkercount)
    cdef t_max_err sysparallel_task_workercount(t_sysparallel_task *x, long workercount)
    cdef t_max_err sysparallel_task_execute(t_sysparallel_task *x)
    cdef void sysparallel_task_signalworkers(t_sysparallel_task *x, long count) 
    cdef t_max_err sysparallel_task_cancel(t_sysparallel_task *x)
    cdef void sysparallel_task_free(t_sysparallel_task *x)
    cdef void sysparallel_task_benchprint(t_sysparallel_task *x)
    cdef void sysparallel_task_data(t_sysparallel_task *x, void * data)
    cdef void sysparallel_task_workerproc(t_sysparallel_task *x, method workerproc)
    cdef t_sysparallel_worker *sysparallel_worker_new(void *data, method workerproc, t_sysparallel_task *task)
    cdef t_max_err sysparallel_worker_execute(t_sysparallel_worker *x)
    cdef void sysparallel_worker_free(t_sysparallel_worker *x)


cdef extern from "ext_sysprocess.h":

    cdef long sysprocess_isrunning(long id)   
    cdef long sysprocess_launch(const char *utf8path, const char *utf8commandline)  
    cdef long sysprocess_activate(long id)
    cdef long sysprocess_getid(const char *utf8path)  
    cdef long sysprocess_getcurrentid() 
    cdef long sysprocess_getpath(long id, char **utf8path)
    cdef t_object* sysprocesswatcher_new(long id, method m, void *arg)
    cdef long sysprocess_fitsarch(long id)


cdef extern from "ext_sysmem.h":

    ctypedef void * t_syssem 
    cdef t_max_err syssem_create(t_syssem *x, const char *name, long flags, unsigned int value)
    cdef t_max_err syssem_open(t_syssem *x, const char *name, long flags) 
    cdef t_max_err syssem_close(t_syssem x)
    cdef t_max_err syssem_wait(t_syssem x)
    cdef t_max_err syssem_trywait(t_syssem x)
    cdef t_max_err syssem_post(t_syssem x)


cdef extern from "ext_sysshmem.h":

    ctypedef void *t_sysshmem 
    cdef int SYSSHMEM_FLAGS_READONLY
    cdef t_max_err sysshmem_alloc(t_sysshmem *x, const char *name, long size, long flags)   
    cdef t_max_err sysshmem_open(t_sysshmem *x, const char *name, long flags)               
    cdef t_max_err sysshmem_close(t_sysshmem x)     
    cdef unsigned long sysshmem_getsize(t_sysshmem x)       
    cdef void* sysshmem_getptr(t_sysshmem x)


cdef extern from "ext_systime.h":

    ctypedef struct t_datetime

    ctypedef enum e_max_dateflags:
        SYSDATEFORMAT_FLAGS_SHORT = 1
        SYSDATEFORMAT_FLAGS_MEDIUM = 2  
        SYSDATEFORMAT_FLAGS_LONG = 3    
        
    cdef t_uint32 systime_ticks()
    cdef t_uint32 systime_ms()
    cdef t_int64 systime_datetime_milliseconds()
    cdef void systime_datetime(t_datetime *d)
    cdef t_ptr_uint systime_seconds()
    cdef void systime_secondstodate(t_ptr_uint secs, t_datetime *d)
    cdef t_ptr_uint systime_datetoseconds(t_datetime *d)
    cdef void sysdateformat_strftimetodatetime(char *strf, t_datetime *d)
    cdef void sysdateformat_formatdatetime(t_datetime *d, long dateflags, long timeflags, char *s, long buflen)
    cdef int SYSDATEFORMAT_RELATIVE


cdef extern from "commonsyms.h":
    cdef int COMMON_SYMBOLS_VERSION_5_0_0
    cdef int COMMON_SYMBOLS_VERSION
    ctypedef struct t_common_symbols_table
    cdef extern t_common_symbols_table *_common_symbols
    cdef void common_symbols_init()
    cdef t_common_symbols_table *common_symbols_gettable()
    


cdef extern from "ext_wind.h":
    ctypedef enum e_max_wind_advise_result:
        aaYes = 1
        aaNo        
        aaCancel        

    cdef int ADVISE_SAVE
    cdef int ADVISE_DISCARD
    cdef int ADVISE_CANCEL
    cdef int ADVISE_FIRST
    cdef int ADVISE_SECOND

    cdef short wind_advise(t_object *w, char *s, ...)
    cdef short wind_advise_explain(t_object *w, char *note, char *explanation, char *b1, char *b2, char *b3)
    cdef void wind_setcursor(short which)




cdef extern from "jpatcher_api.h":

    cdef int JPATCHER_API_CURRENT_FILE_VERSION

    ctypedef enum t_clipboard_datatype:
        JCLIPBOARD_TYPE_TEXT = 1
        JCLIPBOARD_TYPE_BINBUF = 2
        JCLIPBOARD_TYPE_JSON = 4
        JCLIPBOARD_TYPE_IMAGE = 8
        JCLIPBOARD_TYPE_JSON_ATTRIBUTES = 16
        JCLIPBOARD_TYPE_UNKNOWN = 256

    cdef t_symbol *fontmap_getmapping(t_symbol *from_, char *mapped)
    cdef double fontinfo_getsize(short oldsize)     
    cdef t_symbol *fontinfo_getname(short number)
    cdef short fontinfo_getnumber(t_symbol *s)
    ctypedef struct t_rect
    ctypedef struct t_pt
    ctypedef struct t_size
    ctypedef struct t_jrgb
    ctypedef struct t_jrgba
    ctypedef struct t_jboxdrawparams

    cdef int JBOX_SPOOL_CONTENTS
    cdef int JBOX_SPOOL_WHOLEBOX
    #// cdef int JBOX_RELINE_DEFER
    cdef int JBOX_FLAG_READ
    cdef int JBOX_FLAG_FIRST_PAINT

    ctypedef struct t_jbox
    ctypedef struct t_pvselinfo

    cdef t_max_err object_attr_get_rect(t_object *o, t_symbol *name, t_rect *rect)
    cdef t_max_err object_attr_set_rect(t_object *o, t_symbol *name, t_rect *rect)
    cdef void object_attr_set_xywh(t_object *o, t_symbol *attr, double x, double y, double w, double h)
    cdef t_max_err object_attr_getpt(t_object *o, t_symbol *name, t_pt *pt)
    cdef t_max_err object_attr_setpt(t_object *o, t_symbol *name, t_pt *pt) 
    cdef t_max_err object_attr_getsize(t_object *o, t_symbol *name, t_size *size)
    cdef t_max_err object_attr_setsize(t_object *o, t_symbol *name, t_size *size) 
    cdef t_max_err object_attr_getcolor(t_object *b, t_symbol *attrname, t_jrgba *prgba)
    cdef t_max_err object_attr_setcolor(t_object *b, t_symbol *attrname, t_jrgba *prgba)
    cdef t_max_err jrgba_attr_get(t_jrgba *jrgba, long *argc, t_atom **argv)
    cdef t_max_err jrgba_attr_set(t_jrgba *jrgba, long argc, t_atom *argv)
    cdef void set_jrgba_from_palette_index(short index, t_jrgba *jrgba) 
    cdef void set_jrgba_from_boxcolor_index(short index, t_jrgba *jrgba) 
    cdef short get_boxcolor_index_from_jrgba(t_jrgba *jrgba) 
    cdef void jgraphics_clip_rgba(t_jrgba *rgba) 
    cdef void object_openhelp(t_object *x)
    cdef void object_openrefpage(t_object *x)
    cdef void object_openquery(t_object *x)
    cdef void classname_openhelp(char *classname)
    cdef void classname_openrefpage(char *classname)
    cdef void classname_openquery(char *classname) 
    cdef t_object* patcherview_findpatcherview(int x, int y)
    cdef void patcherview_makepalette()
    cdef int jpatcher_is_patcher(t_object *p)
    cdef t_object* jpatcher_get_box(t_object *p)
    cdef long jpatcher_get_count(t_object *p)
    cdef char jpatcher_get_locked(t_object *p) 
    cdef t_max_err jpatcher_set_locked(t_object *p, char c)
    cdef char jpatcher_get_presentation(t_object *p)
    cdef t_max_err jpatcher_set_presentation(t_object *p, char c)
    cdef t_object* jpatcher_get_firstobject(t_object *p)
    cdef t_object* jpatcher_get_lastobject(t_object *p)
    cdef t_object* jpatcher_get_firstline(t_object *p)
    cdef t_object* jpatcher_get_firstview(t_object *p)
    cdef t_symbol* jpatcher_get_title(t_object *p)
    cdef t_max_err jpatcher_set_title(t_object *p, t_symbol *ps)
    cdef t_symbol* jpatcher_get_name(t_object *p)
    cdef t_symbol* jpatcher_get_filepath(t_object *p)
    cdef t_symbol* jpatcher_get_filename(t_object *p)
    cdef char jpatcher_get_dirty(t_object *p)
    cdef t_max_err jpatcher_set_dirty(t_object *p, char c)
    cdef char jpatcher_get_bglocked(t_object *p)
    cdef t_max_err jpatcher_set_bglocked(t_object *p, char c)
    cdef char jpatcher_get_bghidden(t_object *p)
    cdef t_max_err jpatcher_set_bghidden(t_object *p, char c)
    cdef char jpatcher_get_fghidden(t_object *p)
    cdef t_max_err jpatcher_set_fghidden(t_object *p, char c)
    cdef t_max_err jpatcher_get_editing_bgcolor(t_object *p, t_jrgba *prgba)
    cdef t_max_err jpatcher_set_editing_bgcolor(t_object *p, t_jrgba *prgba)
    cdef t_max_err jpatcher_get_bgcolor(t_object *p, t_jrgba *prgba)
    cdef t_max_err jpatcher_get_locked_bgcolor(t_object *p, t_jrgba *prgba)
    cdef t_max_err jpatcher_set_bgcolor(t_object *p, t_jrgba *prgba)
    cdef t_max_err jpatcher_set_locked_bgcolor(t_object *p, t_jrgba *prgba)
    cdef t_max_err jpatcher_get_gridsize(t_object *p, double *gridsizeX, double *gridsizeY)
    cdef t_max_err jpatcher_set_gridsize(t_object *p, double gridsizeX, double gridsizeY)
    cdef t_object* jpatcher_get_controller(t_object *p)
    cdef void jpatcher_deleteobj(t_object *p, t_jbox *b)
    cdef t_object* jpatcher_get_parentpatcher(t_object *p)
    cdef t_object* jpatcher_get_toppatcher(t_object *p)
    cdef t_object* jpatcher_get_hubholder(t_object *p)
    cdef t_symbol* jpatcher_get_maxclass(t_object *p)
    cdef t_symbol* jpatcher_get_parentclass(t_object *p) 
    cdef t_max_err jpatcher_get_rect(t_object *p, t_rect *pr)
    cdef t_max_err jpatcher_set_rect(t_object *p, t_rect *pr)
    cdef t_max_err jpatcher_get_defrect(t_object *p, t_rect *pr)
    cdef t_max_err jpatcher_set_defrect(t_object *p, t_rect *pr)
    cdef char jpatcher_get_noedit(t_object *p) 
    cdef t_object *jpatcher_get_collective(t_object *p) 
    cdef char jpatcher_get_cansave(t_object *p) 
    cdef t_symbol *jpatcher_uniqueboxname(t_object *p, t_symbol *classname)
    cdef short jpatcher_getboxfont(t_object *p, short fnum, double *fsize, t_symbol **fontname)
    cdef t_symbol *jpatcher_get_default_fontname(t_object *p)
    cdef float jpatcher_get_default_fontsize(t_object *p)
    cdef long jpatcher_get_default_fontface(t_object *p)
    cdef t_max_err jpatcher_set_imprint(t_object *p, char c)
    cdef char jpatcher_get_imprint(t_object *p)
    cdef void jpatcher_addboxlistener(t_object *p, t_object *listener)
    cdef void jpatcher_removeboxlistener(t_object *p, t_object *listener)
    cdef long jpatcher_get_fileversion(t_object *p)
    cdef long jpatcher_get_currentfileversion()
    cdef t_max_err jbox_get_rect_for_view(t_object *box, t_object *patcherview, t_rect *rect)
    cdef t_max_err jbox_set_rect_for_view(t_object *box, t_object *patcherview, t_rect *rect)
    cdef t_max_err jbox_get_rect_for_sym(t_object *box, t_symbol *which, t_rect *pr)
    cdef t_max_err jbox_set_rect_for_sym(t_object *box, t_symbol *which, t_rect *pr)
    cdef t_max_err jbox_set_rect(t_object *box, t_rect *pr)
    cdef t_max_err jbox_get_patching_rect(t_object *box, t_rect *pr)         
    cdef t_max_err jbox_set_patching_rect(t_object *box, t_rect *pr)         
    cdef t_max_err jbox_get_presentation_rect(t_object *box, t_rect *pr) 
    cdef t_max_err jbox_set_presentation_rect(t_object *box, t_rect *pr) 
    cdef t_max_err jbox_set_position(t_object *box, t_pt *pos)
    cdef t_max_err jbox_get_patching_position(t_object *box, t_pt *pos)  
    cdef t_max_err jbox_set_patching_position(t_object *box, t_pt *pos)      
    cdef t_max_err jbox_get_presentation_position(t_object *box, t_pt *pos) 
    cdef t_max_err jbox_set_presentation_position(t_object *box, t_pt *pos) 
    cdef t_max_err jbox_set_size(t_object *box, t_size *size)
    cdef t_max_err jbox_get_patching_size(t_object *box, t_size *size)  
    cdef t_max_err jbox_set_patching_size(t_object *box, t_size *size)       
    cdef t_max_err jbox_get_presentation_size(t_object *box, t_size *size) 
    cdef t_max_err jbox_set_presentation_size(t_object *box, t_size *size) 
    cdef t_symbol* jbox_get_maxclass(t_object *b)
    cdef t_object* jbox_get_object(t_object *b)
    cdef t_object* jbox_get_patcher(t_object *b) 
    cdef char jbox_get_hidden(t_object *b)
    cdef t_max_err jbox_set_hidden(t_object *b, char c)
    cdef t_symbol* jbox_get_fontname(t_object *b)
    cdef t_max_err jbox_set_fontname(t_object *b, t_symbol *ps) 
    cdef double jbox_get_fontsize(t_object *b)
    cdef t_max_err jbox_set_fontsize(t_object *b, double d) 
    cdef t_max_err jbox_get_color(t_object *b, t_jrgba *prgba)
    cdef t_max_err jbox_set_color(t_object *b, t_jrgba *prgba) 
    cdef t_symbol *jbox_get_hint(t_object *b) 
    cdef t_max_err jbox_set_hint(t_object *b, t_symbol *s) 
    cdef char *jbox_get_hintstring(t_object *bb)
    cdef void jbox_set_hintstring(t_object *bb, char *s)
    cdef char jbox_get_hinttrack(t_object *b) 
    cdef t_max_err jbox_set_hinttrack(t_object *b, char h) 
    cdef char *jbox_get_annotation(t_object *bb)
    cdef void jbox_set_annotation(t_object *bb, char *s)
    cdef t_object* jbox_get_nextobject(t_object *b)
    cdef t_object* jbox_get_prevobject(t_object *b)
    cdef t_symbol* jbox_get_varname(t_object *b)
    cdef t_max_err jbox_set_varname(t_object *b, t_symbol *ps)
    cdef t_symbol* jbox_get_id(t_object *b)
    cdef char jbox_get_canhilite(t_object *b)
    cdef char jbox_get_background(t_object *b)
    cdef t_max_err jbox_set_background(t_object *b, char c)
    cdef char jbox_get_ignoreclick(t_object *b)
    cdef t_max_err jbox_set_ignoreclick(t_object *b, char c)
    cdef char jbox_get_drawfirstin(t_object *b)
    cdef char jbox_get_outline(t_object *b)
    cdef t_max_err jbox_set_outline(t_object *b, char c)
    cdef char jbox_get_growy(t_object *b)
    cdef char jbox_get_growboth(t_object *b)
    cdef char jbox_get_nogrow(t_object *b)
    cdef char jbox_get_drawinlast(t_object *b)
    cdef char jbox_get_mousedragdelta(t_object *b)
    cdef t_max_err jbox_set_mousedragdelta(t_object *b, char c)
    cdef t_object* jbox_get_textfield(t_object *b)
    cdef long jbox_get_understanding(t_object *b, t_symbol *msg)
    cdef char jbox_get_presentation(t_object *b)
    cdef t_max_err jbox_set_presentation(t_object *b, char c)
    cdef t_object *jbox_get_autocompletion(t_object *b)
    cdef t_symbol *jbox_get_prototypename(t_object *b)
    cdef void jbox_set_prototypename(t_object *b, t_symbol *name)
    cdef t_atom_long jclipboard_datatypes()
    cdef void jbox_validaterects(t_jbox *b)

    ctypedef enum t_patchline_updatetype:
        JPATCHLINE_DISCONNECT = 0
        JPATCHLINE_CONNECT = 1
        JPATCHLINE_ORDER = 2

        
    cdef t_max_err jpatchline_get_startpoint(t_object *l, double *x, double *y)
    cdef t_max_err jpatchline_get_endpoint(t_object *l, double *x, double *y)
    cdef long jpatchline_get_nummidpoints(t_object *l) 
    cdef char jpatchline_get_pending(t_object *l) 
    cdef t_object* jpatchline_get_box1(t_object *l) 
    cdef long jpatchline_get_outletnum(t_object *l)
    cdef t_object* jpatchline_get_box2(t_object *l) 
    cdef long jpatchline_get_inletnum(t_object *l)
    cdef double jpatchline_get_straightthresh(t_object *l)
    cdef t_max_err jpatchline_set_straightthresh(t_object *l, double d)
    cdef char jpatchline_get_straightstart(t_object *l) 
    cdef char jpatchline_get_straightend(t_object *l) 
    cdef t_max_err jpatchline_set_straightstart(t_object *l, char c) 
    cdef t_max_err jpatchline_set_straightend(t_object *l, char c)
    cdef t_object* jpatchline_get_nextline(t_object *b)
    cdef char jpatchline_get_hidden(t_object *l) 
    cdef t_max_err jpatchline_set_hidden(t_object *l, char c) 
    cdef t_max_err jpatchline_get_color(t_object *l, t_jrgba *prgba) 
    cdef t_max_err jpatchline_set_color(t_object *l, t_jrgba *prgba)
    cdef t_object *jpatchline_get_wiretap(t_object *l)
    cdef long wiretap_get_id(t_object *w)
    cdef long wiretap_get_flags(t_object *w)
    cdef void wiretap_set_flags(t_object *w, long n)
    cdef char patcherview_get_visible(t_object *pv) 
    cdef t_max_err patcherview_set_visible(t_object *pv, char c) 
    cdef t_max_err patcherview_get_rect(t_object *pv, t_rect *pr)
    cdef t_max_err patcherview_set_rect(t_object *pv, t_rect *pr)
    cdef void patcherview_canvas_to_screen(t_object *pv, double cx, double cy, long *sx, long *sy) 
    cdef void patcherview_screen_to_canvas(t_object *pv, long sx, long sy, double *cx, double *cy)
    cdef char patcherview_get_locked(t_object *p) 
    cdef t_max_err patcherview_set_locked(t_object *p, char c) 
    cdef char patcherview_get_presentation(t_object *pv)
    cdef t_max_err patcherview_set_presentation(t_object *p, char c)
    cdef double patcherview_get_zoomfactor(t_object *pv)
    cdef t_max_err patcherview_set_zoomfactor(t_object *pv, double d) 
    cdef t_object* patcherview_get_nextview(t_object *pv)
    cdef t_object* patcherview_get_jgraphics(t_object *pv)
    cdef t_max_err patcherview_set_jgraphics(t_object *pv, t_object *po)
    cdef t_object* patcherview_get_patcher(t_object *pv) 
    cdef t_object* patcherview_get_topview(t_object *pv)
    cdef t_object* textfield_get_owner(t_object *tf)
    cdef t_max_err textfield_get_textcolor(t_object *tf, t_jrgba *prgba) 
    cdef t_max_err textfield_set_textcolor(t_object *tf, t_jrgba *prgba) 
    cdef t_max_err textfield_get_bgcolor(t_object *tf, t_jrgba *prgba) 
    cdef t_max_err textfield_set_bgcolor(t_object *tf, t_jrgba *prgba) 
    cdef t_max_err textfield_get_textmargins(t_object *tf, double *pleft, double *ptop, double *pright, double *pbottom)
    cdef t_max_err textfield_set_textmargins(t_object *tf, double left, double top, double right, double bottom) 
    cdef char textfield_get_editonclick(t_object *tf)
    cdef t_max_err textfield_set_editonclick(t_object *tf, char c) 
    cdef char textfield_get_selectallonedit(t_object *tf)
    cdef t_max_err textfield_set_selectallonedit(t_object *tf, char c) 
    cdef char textfield_get_noactivate(t_object *tf)
    cdef t_max_err textfield_set_noactivate(t_object *tf, char c) 
    cdef char textfield_get_readonly(t_object *tf)
    cdef t_max_err textfield_set_readonly(t_object *tf, char c) 
    cdef char textfield_get_wordwrap(t_object *tf)
    cdef t_max_err textfield_set_wordwrap(t_object *tf, char c) 
    cdef char textfield_get_useellipsis(t_object *tf)
    cdef t_max_err textfield_set_useellipsis(t_object *tf, char c) 
    cdef char textfield_get_autoscroll(t_object *tf)
    cdef t_max_err textfield_set_autoscroll(t_object *tf, char c) 
    cdef char textfield_get_wantsreturn(t_object *tf)
    cdef t_max_err textfield_set_wantsreturn(t_object *tf, char c) 
    cdef char textfield_get_wantstab(t_object *tf)
    cdef t_max_err textfield_set_wantstab(t_object *tf, char c) 
    cdef char textfield_get_underline(t_object *tf)
    cdef t_max_err textfield_set_underline(t_object *tf, char c)
    cdef char textfield_get_justification(t_object *tf)
    cdef t_max_err textfield_set_justification(t_object *tf, char c)
    cdef char textfield_get_autofixwidth(t_object *tf)
    cdef t_max_err textfield_set_autofixwidth(t_object *tf, char c) 
    cdef t_max_err textfield_set_emptytext(t_object *tf, t_symbol *txt)
    cdef t_symbol *textfield_get_emptytext(t_object *tf)


    cdef int TEXTFIELD_DEF_LEFTMARGIN
    cdef int TEXTFIELD_DEF_TOPMARGIN   
    cdef int TEXTFIELD_DEF_RIGHTMARGIN  
    cdef int TEXTFIELD_DEF_BOTTOMMARGIN 
    cdef int JBOX_DRAWFIRSTIN                 
    cdef int JBOX_NODRAWBOX                   
    cdef int JBOX_DRAWINLAST                  
    cdef int JBOX_TRANSPARENT                 
    cdef int JBOX_NOGROW                      
    cdef int JBOX_GROWY                       
    cdef int JBOX_GROWBOTH                    
    cdef int JBOX_IGNORELOCKCLICK             
    cdef int JBOX_HILITE                      
    cdef int JBOX_BACKGROUND                  
    cdef int JBOX_NOFLOATINSPECTOR                
    cdef int JBOX_TEXTFIELD                       
    cdef int JBOX_FIXWIDTH                        
    cdef int JBOX_FONTATTR                        
    cdef int JBOX_TEXTJUSTIFICATIONATTR        
    cdef int JBOX_BINBUF                          
    cdef int JBOX_MOUSEDRAGDELTA                  
    cdef int JBOX_COLOR                           
    cdef int JBOX_DRAWIOLOCKED                    
    cdef int JBOX_DRAWBACKGROUND                  
    cdef int JBOX_NOINSPECTFIRSTIN                
                                              
    cdef int JBOX_FOCUS                       
    cdef int JBOX_BOXVIEW                     
    cdef int JBOX_LEGACYCOLOR                 
    cdef int JBOX_COPYLEGACYDEFAULT           
    cdef int JBOX_NOLEGACYDEFAULT      

    ctypedef enum:
        JBOX_FONTFACE_REGULAR = 0
        JBOX_FONTFACE_BOLD = 1      
        JBOX_FONTFACE_ITALIC = 2        
        JBOX_FONTFACE_BOLDITALIC = 3    

    ctypedef enum HitTestResult:
        HitNothing = 0      
        HitBox = 1          
        HitInlet = 2        
        HitOutlet = 3       
        HitGrowBox = 4      
        HitLine = 5     
        HitLineLocked = 6   

    ctypedef enum DecoratorPaintFlags:
        BoxSelected = 1 << 0
        DrawFirstIn = 1 << 1
        NoGrow = 1 << 2
        Outline = 1 << 3 
        Locked = 1 << 4
        InletHighlighted = 1 << 5           
        OutletHighlighted = 1 << 6          

    cdef void jbox_initclass(t_class *c, long flags)  
    cdef t_max_err jbox_new(t_jbox *b, long flags, long argc, t_atom *argv)
    cdef void jbox_free(t_jbox *b)
    cdef void jbox_ready(t_jbox *b)
    cdef void jbox_redraw(t_jbox *b)
    cdef void jbox_redrawcontents(t_jbox *b) 
    cdef void *jbox_getoutlet(t_jbox *x, long index)
    cdef void *jbox_getinlet(t_jbox *x, long index)
    cdef void jbox_updatetextfield(t_jbox *b)
    cdef int LEGACYDEFAULTS_FLAGS_FORCE

    cdef void jbox_processlegacydefaults(t_jbox *b, t_dictionary *d, long flags)
    cdef t_max_err jbox_notify(t_jbox *b, t_symbol *s, t_symbol *msg, void *sender, void *data)
    cdef t_max_err jbox_set_to_defaultsize(t_jbox *b, t_symbol *s, short argc, t_atom *argv)
    cdef void jbox_grabfocus(t_jbox *b) 
    cdef void jbox_redrawpeers(t_jbox *b)
    cdef long jbox_getinletindex(t_jbox *b, void *inlet)
    cdef long jbox_getoutletindex(t_jbox *b, void *outlet)
    cdef void jbox_show_caption(t_jbox *b)
    cdef void jbox_hide_caption(t_jbox *b)

    cdef int DICT_JRGBA  

    cdef t_max_err dictionary_appendjrgba(t_dictionary *d, t_symbol *key, t_jrgba *jc)
    cdef t_max_err dictionary_getdefjrgba(t_dictionary *d, t_symbol *key, t_jrgba *jc, t_jrgba *def_)
    cdef t_max_err dictionary_gettrect(t_dictionary *d, t_symbol *key, t_rect *rect)
    cdef t_max_err dictionary_appendtrect(t_dictionary *d, t_symbol *key, t_rect *rect)
    cdef t_max_err dictionary_gettpt(t_dictionary *d, t_symbol *key, t_pt *pt)
    cdef t_max_err dictionary_appendtpt(t_dictionary *d, t_symbol *key, t_pt *pt)
    cdef void atomstojrgba(long argc, t_atom *argv, t_jrgba *dest)
    cdef void jrgbatoatoms(t_jrgba *src, t_atom *argv)
    cdef t_max_err dictionary_read(char *filename, short path, t_dictionary **d)
    cdef t_max_err dictionary_write(t_dictionary *d, char *filename, short path)
    cdef t_max_err dictionary_read_yaml(const char *filename, const short path, t_dictionary **d)
    cdef t_max_err dictionary_write_yaml(const t_dictionary *d, const char *filename, const short path)
    #define newobject_fromdictionary_delete(p,d) newobject_fromdictionary(p,d), freeobject((t_object *)d)

    ctypedef enum t_modifiers:
        eCommandKey = 1     
        eShiftKey = 2           
        eControlKey = 4     
        eAltKey = 8         
        eLeftButton = 16        
        eRightButton = 32       
        eMiddleButton = 64      
        ePopupMenu = 128        
        eCapsLock = 256     
        eAutoRepeat = 512       

    cdef t_modifiers jkeyboard_getcurrentmodifiers() 
    cdef t_modifiers jkeyboard_getcurrentmodifiers_realtime()

    ctypedef enum t_keycode:
        
        
        JKEY_NONE               = -1
        JKEY_SPACEBAR           = -2
        JKEY_ESC                = -3
        JKEY_RETURN             = -4
        JKEY_ENTER              = -4  
        JKEY_TAB                = -5
        JKEY_DELETE             = -6
        JKEY_BACKSPACE          = -7
        JKEY_INSERT             = -8
        JKEY_UPARROW            = -9
        JKEY_DOWNARROW          = -10
        JKEY_LEFTARROW          = -11
        JKEY_RIGHTARROW         = -12
        JKEY_PAGEUP             = -13
        JKEY_PAGEDOWN           = -14
        JKEY_HOME               = -15
        JKEY_END                = -16
        JKEY_F1                 = -17
        JKEY_F2                 = -18
        JKEY_F3                 = -19
        JKEY_F4                 = -20
        JKEY_F5                 = -21
        JKEY_F6                 = -22
        JKEY_F7                 = -23
        JKEY_F8                 = -24
        JKEY_F9                 = -25
        JKEY_F10                = -26
        JKEY_F11                = -27
        JKEY_F12                = -28
        JKEY_F13                = -29
        JKEY_F14                = -30
        JKEY_F15                = -31
        JKEY_F16                = -32
        JKEY_NUMPAD0            = -33
        JKEY_NUMPAD1            = -34
        JKEY_NUMPAD2            = -35
        JKEY_NUMPAD3            = -36
        JKEY_NUMPAD4            = -37
        JKEY_NUMPAD5            = -38
        JKEY_NUMPAD6            = -39
        JKEY_NUMPAD7            = -40
        JKEY_NUMPAD8            = -41
        JKEY_NUMPAD9            = -42
        JKEY_NUMPADADD          = -43
        JKEY_NUMPADSUBTRACT     = -44
        JKEY_NUMPADMULTIPLY     = -45
        JKEY_NUMPADDIVIDE       = -46
        JKEY_NUMPADSEPARATOR    = -47
        JKEY_NUMPADDECIMALPOINT = -48
        JKEY_NUMPADEQUALS       = -49
        JKEY_NUMPADDELETE       = -50
        JKEY_PLAYPAUSE          = -51
        JKEY_STOP               = -52
        JKEY_NEXTTRACK          = -53
        JKEY_PREVTRACK          = -54
        JKEY_HELP               = -55

    cdef void jmouse_getposition_global(int *x, int *y)
    cdef void jmouse_setposition_global(int x, int y)
    cdef void jmouse_setposition_view(t_object *patcherview, double cx, double cy)
    cdef void jmouse_setposition_box(t_object *patcherview, t_object *box, double bx, double by)
    cdef void *jmouse_getobject()

    ctypedef enum t_jmouse_cursortype:
        JMOUSE_CURSOR_NONE                      
        JMOUSE_CURSOR_ARROW                         
        JMOUSE_CURSOR_WAIT                      
        JMOUSE_CURSOR_IBEAM                         
        JMOUSE_CURSOR_CROSSHAIR                     
        JMOUSE_CURSOR_COPYING                       
        JMOUSE_CURSOR_POINTINGHAND                  
        JMOUSE_CURSOR_DRAGGINGHAND                  
        JMOUSE_CURSOR_RESIZE_LEFTRIGHT              
        JMOUSE_CURSOR_RESIZE_UPDOWN             
        JMOUSE_CURSOR_RESIZE_FOURWAY                
        JMOUSE_CURSOR_RESIZE_TOPEDGE                
        JMOUSE_CURSOR_RESIZE_BOTTOMEDGE         
        JMOUSE_CURSOR_RESIZE_LEFTEDGE               
        JMOUSE_CURSOR_RESIZE_RIGHTEDGE              
        JMOUSE_CURSOR_RESIZE_TOPLEFTCORNER          
        JMOUSE_CURSOR_RESIZE_TOPRIGHTCORNER     
        JMOUSE_CURSOR_RESIZE_BOTTOMLEFTCORNER       
        JMOUSE_CURSOR_RESIZE_BOTTOMRIGHTCORNER      

    cdef void jmouse_setcursor(t_object *patcherview, t_object *box, t_jmouse_cursortype type)
    cdef t_object* jwind_getactive()
    cdef long jwind_getcount()
    cdef t_object* jwind_getat(long index)
    cdef long jmonitor_getnumdisplays()
    cdef void jmonitor_getdisplayrect(long workarea, long displayindex, t_rect *rect)
    cdef void jmonitor_getdisplayrect_foralldisplays(long workarea, t_rect *rect)           
    cdef void jmonitor_getdisplayrect_forpoint(long workarea, t_pt pt, t_rect *rect)
    cdef void swatches_init()
    cdef void swatches_shutdown()
    cdef void *jpatcher_load(char *name, short volume, short ac, t_atom *av)
    cdef void *jpatcher_load_frombuffer(char *name, short vol, const char *json, long len, short ac, t_atom *av)
    cdef void *jpatcher_load_fromdictionary(char *name, short vol, t_object *rd, short ac, t_atom *av)
    cdef void *jpatcher_load_namespace(char *name, short volume, short ac, t_atom *av, t_symbol *classnamespace)
    cdef void *jpatcher_load_frombuffer_namespace(char *name, short vol, const char *json, long len, short ac, t_atom *av, t_symbol *classnamespace)
    cdef void *jpatcher_load_fromdictionary_namespace(char *name, short vol, t_object *rd, short ac, t_atom *av, t_symbol *classnamespace)
    cdef void classname_openrefpage_ext(t_symbol *classnamespace, char *classname)
    cdef long jpatcher_is_box_namespace(t_object *p)
    cdef t_object *jbox_get_dragtarget(t_jbox *b, char locked)
    cdef long jpatcher_inc_maxsendcontext()
    cdef long jbox_is_selected_in_view(t_object *box, t_object *view)
    cdef t_atom_long jpatcher_dictionary_modernui(t_dictionary *d)
    cdef t_atom_long jpatcher_dictionary_version()
    cdef t_dictionary *jpatcher_fallback_version()
    cdef long jbox_isdefaultattribute(t_jbox *x, t_symbol *attrname)
    cdef const char *systemfontname()
    cdef const char *systemfontname_bold()
    cdef const char *systemfontname_light()
    cdef t_symbol *systemfontsym()
    #//#define JPATCHER_DEFAULT_EXTENSION ".maxpat"

cdef extern from "jpatcher_utils.h":

    cdef void atom_copy(long argc1, t_atom *argv1, t_atom *argv2)
    cdef void postargs(long argc, t_atom *argv)
    cdef void postdictionary(t_object *d)
    cdef t_max_err atom_arg_getobjclass(t_object **x, long idx, long argc, t_atom *argv, t_symbol *cls)
    cdef void *atom_getobjclass(t_atom *av, t_symbol *cls)
    cdef method my_object_getmethod(void *obj, t_symbol *s)


cdef extern from "jgraphics.h":
    ctypedef struct t_jgraphics 
    ctypedef struct t_jpath     
    ctypedef struct t_jpattern      
    ctypedef struct t_jfont     
    ctypedef struct t_jtextlayout   
    ctypedef struct t_jtransform    
    ctypedef struct t_jsurface  
    ctypedef struct t_jdesktopui    
    ctypedef struct t_jpopupmenu    
    ctypedef struct t_jsvg      
    ctypedef struct t_jsvg_remap    

    cdef int JGRAPHICS_RECT_BOTTOM(rect)
    cdef int JGRAPHICS_RECT_RIGHT(rect)
    cdef int JGRAPHICS_PI
    cdef int JGRAPHICS_2PI
    cdef int JGRAPHICS_PIOVER2
    cdef int JGRAPHICS_3PIOVER2

    ctypedef enum t_jgraphics_line_join:
        JGRAPHICS_LINE_JOIN_MITER
        JGRAPHICS_LINE_JOIN_ROUND
        JGRAPHICS_LINE_JOIN_BEVEL


    ctypedef enum t_jgraphics_line_cap:
        JGRAPHICS_LINE_CAP_BUTT
        JGRAPHICS_LINE_CAP_ROUND
        JGRAPHICS_LINE_CAP_SQUARE


    ctypedef enum t_jgraphics_bubble_side:
        JGRAPHICS_BUBBLE_SIDE_TOP
        JGRAPHICS_BUBBLE_SIDE_LEFT
        JGRAPHICS_BUBBLE_SIDE_BOTTOM
        JGRAPHICS_BUBBLE_SIDE_RIGHT


    ctypedef enum t_jgraphics_path_type:
        JGRAPHICS_PATH_STARTNEWSUBPATH
        JGRAPHICS_PATH_LINETO
        JGRAPHICS_PATH_QUADRATICTO
        JGRAPHICS_PATH_CUBICTO
        JGRAPHICS_PATH_CLOSEPATH


    cdef int jgraphics_round(double d)

    ctypedef enum t_jgraphics_format:
        JGRAPHICS_FORMAT_ARGB32   
        JGRAPHICS_FORMAT_RGB24         
        JGRAPHICS_FORMAT_A8             

    ctypedef enum t_jgraphics_fileformat:
        JGRAPHICS_FILEFORMAT_PNG
        JGRAPHICS_FILEFORMAT_JPEG

    cdef t_jsurface* jgraphics_image_surface_create(t_jgraphics_format format, int width, int height)
    cdef t_jsurface *jgraphics_image_surface_create_referenced(const char *filename, short path)
    cdef t_jsurface* jgraphics_image_surface_create_from_file(const char *filename, short path)
    cdef t_jsurface* jgraphics_image_surface_create_for_data(unsigned char *data, t_jgraphics_format format,  int width, int height, int stride, method freefun, void *freearg)
    cdef t_jsurface* jgraphics_image_surface_create_for_data_premult(unsigned char *data, t_jgraphics_format format, int width, int height, int stride, method freefun, void *freearg)
    cdef t_jsurface* jgraphics_image_surface_create_from_filedata(const void *data, unsigned long datalen)
    cdef t_jsurface* jgraphics_image_surface_create_from_resource(const void* moduleRef, const char *resname)
    cdef t_max_err jgraphics_get_resource_data(const void *moduleRef, const char *resname, long extcount, t_atom *exts, void **data, unsigned long *datasize)
    cdef t_jsurface* jgraphics_surface_reference(t_jsurface *s)      
    cdef void        jgraphics_surface_destroy(t_jsurface *s)        
    cdef t_max_err   jgraphics_image_surface_writepng(t_jsurface *surface, const char *filename, short path, long dpi) 
    cdef t_max_err   jgraphics_image_surface_writejpeg(t_jsurface *surface, const char *filename, short path)
    cdef void        jgraphics_surface_set_device_offset(t_jsurface *s, double x_offset, double y_offset) 
    cdef void        jgraphics_surface_get_device_offset(t_jsurface *s, double *x_offset, double *y_offset) 
    cdef int         jgraphics_image_surface_get_width(t_jsurface *s)
    cdef int         jgraphics_image_surface_get_height(t_jsurface *s) 
    cdef void        jgraphics_image_surface_set_pixel(t_jsurface *s, int x, int y, t_jrgba color) 
    cdef void        jgraphics_image_surface_get_pixel(t_jsurface *s, int x, int y, t_jrgba *color)
    cdef void        jgraphics_image_surface_scroll(t_jsurface *s, int x, int y, int width, int height, int dx, int dy, t_jpath **path)      
    cdef const unsigned char* jgraphics_image_surface_lockpixels_readonly(t_jsurface *s, int x, int y, int width, int height, int *linestride, int *pixelstride)
    cdef void                 jgraphics_image_surface_unlockpixels_readonly(t_jsurface *s, const unsigned char *data) 
    cdef unsigned char*       jgraphics_image_surface_lockpixels(t_jsurface *s, int x, int y, int width, int height, int *linestride, int *pixelstride)
    cdef void                 jgraphics_image_surface_unlockpixels(t_jsurface *s, unsigned char *data) 
    cdef void        jgraphics_image_surface_draw(t_jgraphics *g, t_jsurface *s, t_rect srcRect, t_rect destRect)
    cdef void        jgraphics_image_surface_draw_fast(t_jgraphics *g, t_jsurface *s)        
    cdef void jgraphics_write_image_surface_to_filedata(t_jsurface *surf, long fmt, void **data, long *size)
    cdef t_jsurface* jgraphics_image_surface_create_from_base64(const char *base64, unsigned long datalen)
    cdef void jgraphics_write_image_surface_to_base64(t_jsurface *surf, long fmt, char **base64, long *size)
    cdef void jgraphics_image_surface_clear(t_jsurface *s, int x, int y, int width, int height)
    cdef t_jsvg*     jsvg_create_from_file(const char *filename, short path) 
    cdef t_jsvg*     jsvg_create_from_resource(const void *moduleRef, const char *resname)
    cdef t_jsvg*     jsvg_create_from_xmlstring(const char *svgXML) 
    cdef void        jsvg_get_size(t_jsvg *svg, double *width, double *height) 
    cdef void        jsvg_destroy(t_jsvg *svg) 
    cdef void        jsvg_render(t_jsvg *svg, t_jgraphics *g)
    cdef void jsvg_load_cached(t_symbol *name, t_jsvg **psvg)
    cdef t_jsvg_remap *jsvg_remap_create(t_jsvg *svg)
    cdef void jsvg_remap_addcolor(t_jsvg_remap *r, t_jrgba *src, t_jrgba *dst)
    cdef void jsvg_remap_perform(t_jsvg_remap *r, t_jsvg **remapped)
    cdef void jsvg_remap_destroy(t_jsvg_remap *r)
    cdef void jgraphics_draw_jsvg(t_jgraphics *g, t_jsvg *svg, t_rect *r, int flags, double opacity)
    cdef t_jgraphics*    jgraphics_create(t_jsurface *target)
    cdef t_jgraphics*    jgraphics_reference(t_jgraphics *g)
    cdef void            jgraphics_destroy(t_jgraphics *g)

    ctypedef struct t_jgraphics_path_elem

    cdef void        jgraphics_new_path(t_jgraphics *g)
    cdef t_jpath*    jgraphics_copy_path(t_jgraphics *g)
    cdef t_jpath*    jgraphics_path_createstroked(t_jpath *p, double thickness, t_jgraphics_line_join join, t_jgraphics_line_cap cap) 
    cdef void        jgraphics_path_destroy(t_jpath *path) 
    cdef void        jgraphics_append_path(t_jgraphics *g, t_jpath *path)
    cdef void        jgraphics_close_path(t_jgraphics *g)     
    cdef void        jgraphics_path_roundcorners(t_jgraphics *g, double cornerRadius)
    cdef long        jgraphics_path_contains(t_jpath *path, double x, double y) 
    cdef long        jgraphics_path_intersectsline(t_jpath *path, double x1, double y1, double x2, double y2) 
    cdef double      jgraphics_path_getlength(t_jpath *path) 
    cdef void        jgraphics_path_getpointalongpath(t_jpath *path, double distancefromstart, double *x, double *y) 
    cdef double      jgraphics_path_getnearestpoint(t_jpath *path, double x, double y, double *path_x, double *path_y)
    cdef long        jgraphics_path_getpathelems(t_jpath *path, t_jgraphics_path_elem **elems)
    cdef void        jgraphics_get_current_point(t_jgraphics *g, double *x, double *y) 
    cdef void        jgraphics_arc(t_jgraphics *g, double xc, double yc, double radius, double angle1, double angle2)

    cdef void jgraphics_piesegment(t_jgraphics *g, double xc, double yc, double radius, double angle1, double angle2, double innercircleproportionalsize)
    cdef void jgraphics_ovalarc(t_jgraphics *g, double xc, double yc, double radiusx, double radiusy, double angle1, double angle2)
    cdef void jgraphics_arc_negative(t_jgraphics *g, double xc, double yc, double radius, double angle1, double angle2)
    cdef void jgraphics_curve_to(t_jgraphics *g,double x1, double y1, double x2, double y2, double x3, double y3)
    cdef void jgraphics_rel_curve_to(t_jgraphics *g, double x1, double y1, double x2, double y2, double x3, double y3)
    cdef void jgraphics_line_to(t_jgraphics *g, double x, double y)     
    cdef void jgraphics_rel_line_to(t_jgraphics *g, double x, double y)
    cdef void jgraphics_move_to(t_jgraphics *g, double x, double y)     
    cdef void jgraphics_rel_move_to(t_jgraphics *g, double x, double y) 
    cdef void jgraphics_rectangle(t_jgraphics *g, double x, double y, double width, double height)
    cdef void jgraphics_oval(t_jgraphics *g,    double x, double y, double width, double height)
    cdef void jgraphics_rectangle_rounded(t_jgraphics *g, double x, double y, double width, double height, double ovalwidth, double ovalheight)
    cdef void jgraphics_ellipse(t_jgraphics *g,  double x, double y, double width, double height)
    cdef void jgraphics_bubble(t_jgraphics *g, double bodyx, double bodyy,  double bodywidth, double bodyheight, double cornersize, double arrowtipx, double arrowtipy, t_jgraphics_bubble_side whichside, double arrowedgeprop, double arrowwidth)
    cdef void jgraphics_triangle(t_jgraphics *g, double x1, double y1, double x2, double y2, double x3, double y3)
    cdef void jgraphics_diagonal_line_fill(t_jgraphics *g, double pixels, double x, double y, double width, double height)

    ctypedef enum t_jgraphics_font_slant:
        JGRAPHICS_FONT_SLANT_NORMAL,        
        JGRAPHICS_FONT_SLANT_ITALIC         

    ctypedef enum t_jgraphics_font_weight:
        JGRAPHICS_FONT_WEIGHT_NORMAL
        JGRAPHICS_FONT_WEIGHT_BOLD

    cdef void jgraphics_select_font_face(t_jgraphics *g, const char *family, t_jgraphics_font_slant slant, t_jgraphics_font_weight weight)
    cdef void jgraphics_select_jfont(t_jgraphics *g, t_jfont *jfont) 
    cdef void jgraphics_set_font_size(t_jgraphics *g, double size)
    cdef void jgraphics_set_underline(t_jgraphics *g, char underline)
    cdef void jgraphics_show_text(t_jgraphics *g, const char *utf8) 
    cdef void jgraphics_text_path(t_jgraphics *g, const char *utf8)

    ctypedef struct t_jgraphics_font_extents

    cdef void jgraphics_font_extents(t_jgraphics *g, t_jgraphics_font_extents *extents)
    cdef void jgraphics_text_measure(t_jgraphics *g, const char *utf8, double *width, double *height)
    cdef void jgraphics_text_measuretext_wrapped(t_jgraphics *g, const char *utf8, double wrapwidth, long includewhitespace, double *width, double *height, long *numlines) 
    cdef double jgraphics_getfontscale()
    cdef t_jfont* jfont_create_from_maxfont(short number, short size)
    cdef t_jfont* jfont_create(const char *family, t_jgraphics_font_slant slant, t_jgraphics_font_weight weight, double size) 
    cdef t_jfont* jfont_reference(t_jfont *font) 
    cdef void jfont_destroy(t_jfont *font) 
    cdef long jfont_isequalto(t_jfont *font, t_jfont *other)
    cdef void jfont_set_family(t_jfont *font, t_symbol *family)
    cdef t_symbol* jfont_get_family(t_jfont *font)
    cdef void jfont_set_slant(t_jfont *font, t_jgraphics_font_slant slant)
    cdef t_jgraphics_font_slant jfont_get_slant(t_jfont *font)
    cdef void jfont_set_weight(t_jfont *font, t_jgraphics_font_weight weight)
    cdef t_jgraphics_font_weight jfont_get_weight(t_jfont *font)
    cdef void jfont_set_font_size(t_jfont *font, double size)
    cdef double jfont_get_font_size(t_jfont *font)
    cdef void jfont_set_underline(t_jfont *font, char ul)
    cdef char jfont_get_underline(t_jfont *font)
    cdef double jfont_get_heighttocharheightratio(t_jfont *font) 
    cdef void jfont_extents(t_jfont *font, t_jgraphics_font_extents *extents)
    cdef void jfont_text_measure(t_jfont *font, const char *utf8, double *width, double *height)
    cdef void jfont_text_measuretext_wrapped(t_jfont *font, const char *utf8, double wrapwidth, long includewhitespace, double *width, double *height, long *numlines)
    cdef void jfont_get_em_dimensions(t_jfont *font, double *width, double *height) 
    cdef t_max_err  jfont_getfontlist(long *count, t_symbol ***list)
    cdef long jfont_isfixedwidth(const char *name) 
    cdef const char *jfont_get_default_fixedwidth_name()
    cdef void jfont_set_juce_default_fontname(char *s)
    cdef void jfont_copy_juce_default_fontname(char *s, long maxlen)
    cdef void jfont_copy_juce_platform_fontname(char *s, long maxlen)
    cdef void jfont_set_juce_fallback_fontname(char *s)
    cdef void jfont_copy_juce_fallback_fontname(char *s, long maxlen)
    cdef long jgraphics_system_canantialiastexttotransparentbg() 
    cdef long jgraphics_fontname_hasglyph(char *name, long code)
    cdef t_jtextlayout* jtextlayout_create() 
    cdef t_jtextlayout* jtextlayout_withbgcolor(t_jgraphics *g, t_jrgba *bgcolor) 
    cdef void jtextlayout_destroy(t_jtextlayout* textlayout)

    ctypedef enum t_jgraphics_text_justification:
        JGRAPHICS_TEXT_JUSTIFICATION_LEFT = 1           
        JGRAPHICS_TEXT_JUSTIFICATION_RIGHT = 2          
        JGRAPHICS_TEXT_JUSTIFICATION_HCENTERED = 4      
        JGRAPHICS_TEXT_JUSTIFICATION_TOP = 8            
        JGRAPHICS_TEXT_JUSTIFICATION_BOTTOM = 16        
        JGRAPHICS_TEXT_JUSTIFICATION_VCENTERED = 32 
        JGRAPHICS_TEXT_JUSTIFICATION_HJUSTIFIED = 64    
        JGRAPHICS_TEXT_JUSTIFICATION_CENTERED = JGRAPHICS_TEXT_JUSTIFICATION_HCENTERED + JGRAPHICS_TEXT_JUSTIFICATION_VCENTERED 

    ctypedef enum t_jgraphics_textlayout_flags:
        JGRAPHICS_TEXTLAYOUT_NOWRAP = 1     
        JGRAPHICS_TEXTLAYOUT_USEELLIPSIS = 3    

    cdef void jtextlayout_set(t_jtextlayout *textlayout, const char *utf8,  t_jfont *jfont, double x, double y, double width, double height, t_jgraphics_text_justification justification, t_jgraphics_textlayout_flags flags) 
    cdef void jtextlayout_settext(t_jtextlayout *textlayout, const char *utf8, t_jfont *jfont)
    cdef void jtextlayout_settextcolor(t_jtextlayout *textlayout, t_jrgba *textcolor) 
    cdef void jtextlayout_measuretext(t_jtextlayout *textlayout, long startindex, long numchars, long includewhitespace, double *width, double *height, long *numlines)
    cdef void jtextlayout_draw(t_jtextlayout *tl, t_jgraphics *g) 
    cdef long jtextlayout_getnumchars(t_jtextlayout *tl) 
    cdef t_max_err jtextlayout_getcharbox(t_jtextlayout *tl, long index, t_rect *rect) 
    cdef t_max_err jtextlayout_getchar(t_jtextlayout *tl, long index, long *pch)
    cdef t_jpath* jtextlayout_createpath(t_jtextlayout *tl)

    ctypedef struct t_jmatrix


    cdef void jgraphics_matrix_init(t_jmatrix *x, double xx, double yx, double xy, double yy, double x0, double y0)
    cdef void jgraphics_matrix_init_identity(t_jmatrix *x) 
    cdef void jgraphics_matrix_init_translate(t_jmatrix *x, double tx, double ty) 
    cdef void jgraphics_matrix_init_scale(t_jmatrix *x, double sx, double sy) 
    cdef void jgraphics_matrix_init_rotate(t_jmatrix *x, double radians) 
    cdef void jgraphics_matrix_translate(t_jmatrix *x, double tx, double ty) 
    cdef void jgraphics_matrix_scale(t_jmatrix *x, double sx, double sy) 
    cdef void jgraphics_matrix_rotate(t_jmatrix *x, double radians) 
    cdef void jgraphics_matrix_invert(t_jmatrix *x) 
    cdef void jgraphics_matrix_multiply(t_jmatrix *result, const t_jmatrix *a, const t_jmatrix *b) 
    cdef void jgraphics_matrix_transform_point(const t_jmatrix *matrix, double *x, double *y)  
    cdef t_jpattern*    jgraphics_pattern_create_rgba(double red, double green, double blue, double alpha)  
    cdef t_jpattern*    jgraphics_pattern_create_for_surface(t_jsurface *surface) 
    cdef t_jpattern* jgraphics_pattern_create_linear(double x0, double y0, double x1, double y1)
    cdef t_jpattern* jgraphics_pattern_create_radial(double cx0, double cy0, double radius0, double cx1, double cy1, double radius1)
    cdef void jgraphics_pattern_add_color_stop_rgba(t_jpattern* pattern, double offset, double red, double green, double blue, double alpha)
    cdef void jgraphics_pattern_add_color_for_proportion(t_jpattern* pattern, double proportion)
    cdef t_jpattern *jgraphics_pattern_reference(t_jpattern *pattern)
    cdef void jgraphics_pattern_destroy(t_jpattern *pattern)

    ctypedef enum t_jgraphics_pattern_type:
        JGRAPHICS_PATTERN_TYPE_SOLID
        JGRAPHICS_PATTERN_TYPE_SURFACE
        JGRAPHICS_PATTERN_TYPE_LINEAR
        JGRAPHICS_PATTERN_TYPE_RADIAL

    t_jgraphics_pattern_type jgraphics_pattern_get_type(t_jpattern *pattern) 

    ctypedef enum t_jgraphics_extend:
        JGRAPHICS_EXTEND_NONE
        JGRAPHICS_EXTEND_REPEAT
        JGRAPHICS_EXTEND_REFLECT       
        JGRAPHICS_EXTEND_PAD            

    ##define JGRAPHICS_EXTEND_GRADIENT_DEFAULT JGRAPHICS_EXTEND_PAD
    ##define JGRAPHICS_EXTEND_SURFACE_DEFAULT JGRAPHICS_EXTEND_NONE

    cdef void jgraphics_pattern_set_extend(t_jpattern *pattern, t_jgraphics_extend extend) 
    cdef t_jgraphics_extend jgraphics_pattern_get_extend(t_jpattern *pattern) 
    cdef void jgraphics_pattern_set_matrix(t_jpattern *pattern, const t_jmatrix *matrix) 
    cdef void jgraphics_pattern_get_matrix(t_jpattern *pattern, t_jmatrix *matrix)
    cdef void jgraphics_pattern_translate(t_jpattern *pattern, double tx, double ty)
    cdef void jgraphics_pattern_scale(t_jpattern *pattern, double sx, double sy)
    cdef void jgraphics_pattern_rotate(t_jpattern *pattern, double angle)
    cdef void jgraphics_pattern_transform(t_jpattern *pattern, const t_jmatrix *matrix)
    cdef void jgraphics_pattern_identity_matrix(t_jpattern *pattern)
    cdef t_jsurface *jgraphics_pattern_get_surface(t_jpattern *pattern)
    cdef void jgraphics_translate(t_jgraphics *g, double tx, double ty)
    cdef void jgraphics_scale(t_jgraphics *g, double sx, double sy)
    cdef void jgraphics_rotate(t_jgraphics *g, double angle)
    cdef void jgraphics_transform(t_jgraphics *g, const t_jmatrix *matrix)
    cdef void jgraphics_set_matrix(t_jgraphics *g, const t_jmatrix *matrix)
    cdef void jgraphics_get_matrix(t_jgraphics *g, t_jmatrix *matrix) 
    cdef void jgraphics_identity_matrix(t_jgraphics *g)
    cdef void jgraphics_user_to_device(t_jgraphics *g, double *x, double *y)
    cdef void jgraphics_device_to_user(t_jgraphics *g, double *x, double *y)
    cdef void jgraphics_save(t_jgraphics* g)                
    cdef void jgraphics_restore(t_jgraphics *g)
    cdef t_jsurface* jgraphics_get_target(t_jgraphics *g) 
    cdef void jgraphics_push_group(t_jgraphics *g)
    cdef t_jpattern* jgraphics_pop_group(t_jgraphics *g) 
    cdef void jgraphics_pop_group_to_source(t_jgraphics *g) 
    cdef t_jsurface* jgraphics_get_group_target(t_jgraphics *g) 
    cdef t_jsurface* jgraphics_pop_group_surface(t_jgraphics *g) 
    cdef void jgraphics_set_source_rgba(t_jgraphics *g,double red, double green, double blue, double alpha)
    cdef void jgraphics_set_source_jrgba(t_jgraphics *g, t_jrgba *rgba) 
    cdef void jgraphics_set_source_rgb(t_jgraphics *g, double red, double green, double blue) 
    cdef void jgraphics_set_source(t_jgraphics *g, t_jpattern *source)
    cdef void jgraphics_set_source_surface(t_jgraphics *g, t_jsurface *surface, double x, double y) 


    ctypedef enum t_jgraphics_pattern_shared:
        JGRAPHICS_PATTERN_GRAY = 0
        JGRAPHICS_NUM_SHARED_PATTERNS



    cdef void jgraphics_set_source_shared(t_jgraphics *g, t_jgraphics_pattern_shared patindex) 
    cdef void jgraphics_scale_source_rgba(t_jgraphics *g, double redscale, double greenscale, double bluescale, double alphascale)
    cdef void jgraphics_translate_source_rgba(t_jgraphics *g, double redoffset, double greenoffset, double blueoffset, double alphaoffset)
    cdef void jgraphics_set_dash(t_jgraphics *g, double *dashes, int numdashes, double offset)

    ctypedef enum t_jgraphics_fill_rule:
        JGRAPHICS_FILL_RULE_WINDING
        JGRAPHICS_FILL_RULE_EVEN_ODD


    cdef void jgraphics_set_fill_rule(t_jgraphics *g, t_jgraphics_fill_rule fill_rule)
    cdef t_jgraphics_fill_rule jgraphics_get_fill_rule(t_jgraphics *g)
    cdef void jgraphics_set_line_cap(t_jgraphics *g, t_jgraphics_line_cap line_cap)
    cdef t_jgraphics_line_cap jgraphics_get_line_cap(t_jgraphics *g)
    cdef void jgraphics_set_line_join(t_jgraphics *g, t_jgraphics_line_join line_join)
    cdef t_jgraphics_line_join  jgraphics_get_line_join(t_jgraphics *g)
    cdef void jgraphics_set_line_width(t_jgraphics *g, double width)
    cdef double jgraphics_get_line_width(t_jgraphics *g)
    cdef void jgraphics_fill(t_jgraphics *g)
    cdef void jgraphics_fill_preserve(t_jgraphics *g) 
    cdef void jgraphics_fill_with_alpha(t_jgraphics *g, double alpha)
    cdef void jgraphics_fill_preserve_with_alpha(t_jgraphics *g, double alpha)
    cdef int jgraphics_in_fill(t_jgraphics *g, double x, double y)      
    cdef int    jgraphics_path_intersects_line(t_jgraphics *g, double x1, double y1, double x2, double y2)      
    cdef int jgraphics_ptinrect(t_pt pt, t_rect rect)
    cdef int jgraphics_lines_intersect(double l1x1, double l1y1, double l1x2, double l1y2, double l2x1, double l2y1, double l2x2, double l2y2, double *ix, double *iy)
    cdef int jgraphics_line_intersects_rect(double linex1, double liney1, double linex2, double liney2, t_rect r, double *ix, double *iy)
    cdef int jgraphics_ptaboveline(t_pt pt, double lx1, double ly1, double lx2, double ly2)
    cdef int jgraphics_points_on_same_side_of_line(t_pt a, t_pt b, double lx1, double ly1, double lx2, double ly2)
    cdef int jgraphics_ptinroundedrect(t_pt pt, t_rect rect, double ovalwidth, double ovalheight) 


    cdef void jgraphics_fill_extents(t_jgraphics *g, double *x1, double *y1, double *x2, double *y2)
    cdef void jgraphics_paint(t_jgraphics *g)       
    cdef void jgraphics_paint_with_alpha(t_jgraphics *g, double alpha)
    cdef void jgraphics_stroke(t_jgraphics *g)
    cdef void jgraphics_stroke_preserve(t_jgraphics *g)
    cdef void jgraphics_stroke_with_alpha(t_jgraphics *g, double alpha)
    cdef void jgraphics_stroke_preserve_with_alpha(t_jgraphics *g, double alpha)
    cdef void jgraphics_rectangle_fill_fast(t_jgraphics *g, double x, double y, double width, double height)
    cdef void jgraphics_rectangle_draw_fast(t_jgraphics *g, double x, double y, double width, double height, double border)
    cdef void jgraphics_line_draw_fast(t_jgraphics *g, double x1, double y1, double x2, double y2, double linewidth)

    ctypedef enum t_jdesktopui_flags:
        JDESKTOPUI_FLAGS_FIRSTFLAG = 1              

    cdef t_jdesktopui* jdesktopui_new(t_object *owner, t_jdesktopui_flags flags, t_rect rect) 
    cdef void jdesktopui_destroy(t_jdesktopui *x) 
    cdef void jdesktopui_setvisible(t_jdesktopui *x, long way) 
    cdef void jdesktopui_setalwaysontop(t_jdesktopui *x, long way) 
    cdef void jdesktopui_setrect(t_jdesktopui *x, t_rect rect) 
    cdef void jdesktopui_getrect(t_jdesktopui *x, t_rect *rect) 
    cdef void jdesktopui_setposition(t_jdesktopui *x, t_pt pt) 
    cdef void jdesktopui_setfadetimes(t_jdesktopui *x, int fade_in_ms, int fade_out_ms) 
    cdef t_jgraphics* jdesktopui_get_jgraphics(t_jdesktopui *x) 
    cdef void jdesktopui_redraw(t_jdesktopui *x) 
    cdef void jdesktopui_redrawrect(t_jdesktopui *x, t_rect rect) 
    cdef double jdesktopui_getopacity(t_jdesktopui *x)
    cdef void *jdesktopui_createtimer(t_jdesktopui *x, t_symbol *msg, void *arg)
    cdef void jdesktopui_starttimer(void *ref, int interval)
    cdef void jdesktopui_stoptimer(void *ref, int alsodelete)
    cdef void jdesktopui_destroytimer(void *ref)
    cdef t_jrgba jgraphics_jrgba_contrasting(t_jrgba *c, double amount)
    cdef t_jrgba jgraphics_jrgba_contrastwith(t_jrgba *c1, t_jrgba *c2)
    cdef t_jrgba jgraphics_jrgba_darker(t_jrgba *c, double amount)
    cdef t_jrgba jgraphics_jrgba_brighter(t_jrgba *c, double amount)
    cdef t_jrgba jgraphics_jrgba_overlay(t_jrgba *c1, t_jrgba *c2)
    cdef t_jrgba jgraphics_jrgba_interpolate(t_jrgba *c1, t_jrgba *c2, double proportion)
    cdef void jgraphics_jrgba_gethsb(t_jrgba *c, double *h, double *s, double *b)
    cdef t_jrgba jgraphics_jrgba_fromhsb(double h, double s, double b, double a)
    cdef long jcolor_getcolor(t_symbol *name, t_jrgba *on, t_jrgba *off)
    cdef t_jpopupmenu* jpopupmenu_create() 
    cdef void jpopupmenu_destroy(t_jpopupmenu *menu)
    cdef void jpopupmenu_clear(t_jpopupmenu *menu)
    cdef void jpopupmenu_setitemcallback(method fun, void *arg)

    cdef void jpopupmenu_setcolors(t_jpopupmenu *menu, t_jrgba text, t_jrgba bg, t_jrgba highlightedtext, t_jrgba highlightedbg) 
    cdef void jpopupmenu_setheadercolor(t_jpopupmenu *menu, t_jrgba *hc)
    cdef void jpopupmenu_setfont(t_jpopupmenu *menu, t_jfont *font)
    cdef void jpopupmenu_additem(t_jpopupmenu *menu, int itemid, const char *utf8Text, t_jrgba *textColor, int checked, int disabled, t_jsurface *icon)
    cdef void jpopupmenu_addsubmenu(t_jpopupmenu *menu, const char *utf8Name, t_jpopupmenu *submenu, int disabled) 
    cdef void jpopupmenu_addseperator(t_jpopupmenu *menu) 
    cdef void jpopupmenu_addheader(t_jpopupmenu *menu, const char *utf8Text)
    cdef void jpopupmenu_addownerdrawitem(t_jpopupmenu *menu, int itemid, t_object *owner)
    cdef int jpopupmenu_popup(t_jpopupmenu *menu, t_pt screen, int defitemid)
    cdef int jpopupmenu_popup_abovebox(t_jpopupmenu *menu, t_object *box, t_object *view, int offset, int defitemid)
    cdef int jpopupmenu_popup_nearbox(t_jpopupmenu *menu, t_object *box, t_object *view, int defitemid) 
    cdef int jpopupmenu_popup_belowrect(t_jpopupmenu *menu, t_rect rect, int defitemid)
    cdef int jpopupmenu_popup_aboverect(t_jpopupmenu *menu, t_rect rect, int defitemid)
    cdef int jpopupmenu_popup_leftofpt(t_jpopupmenu *menu, t_pt pt, int defitemid, int flags)
    cdef void jpopupmenu_estimatesize(t_jpopupmenu *menu, int *width, int *height)
    cdef void jpopupmenu_setitemtooltip(void *itemref, char *tip)

    ctypedef enum:
        JPOPUPMENU_DARKSTYLE = 1

    cdef void jpopupmenu_setstandardstyle(t_jpopupmenu *menu, long styleindex, double fontsize, int margin)
    cdef void jpopupmenu_setstandardstyle_forjucemenu(void *jpm, long styleindex, double fontsize, t_jrgba *headertextcolor)
    cdef void jpopupmenu_closeall() 
    cdef void jmouse_setcursor_surface(t_object *patcherview, t_object *box, t_jsurface *surface, int xHotSpot, int yHotSpot)   
    cdef void jbox_fontface_to_weight_slant(t_object *b, long *weight, long *slant)
    cdef long jbox_get_font_weight(t_object *b)
    cdef long jbox_get_font_slant(t_object *b)
    cdef t_jfont *jbox_createfont(t_object *b)
    cdef void jgraphics_jrgba_set_brightness(t_jrgba *c, double amt)
    cdef t_max_err jgraphics_attr_setrgba(t_object *x, t_object *attr, long argc, t_atom *argv)
    cdef t_max_err jgraphics_attr_setrgb_alias(t_object *x, t_object *attr, long argc, t_atom *argv)

    # #define CLASS_ATTR_RGBA(c,attrname,flags,structname,structmember) \
    #     {   CLASS_ATTR_DOUBLE_ARRAY(c,attrname,flags,structname,structmember,4) \
    #         CLASS_ATTR_ACCESSORS(c,attrname,NULL,jgraphics_attr_setrgba) \
    #         CLASS_ATTR_PAINT(c,attrname,0) }
    # #define CLASS_ATTR_RGBA_LEGACY(c,attrname,aliasname,flags,structname,structmember) \
    #     {   CLASS_ATTR_RGBA(c,attrname,flags,structname,structmember) \
    #         CLASS_ATTR_ALIAS(c,attrname,aliasname) \
    #         CLASS_ATTR_INVISIBLE(c,aliasname,0) \
    #         CLASS_ATTR_ACCESSORS(c,aliasname,NULL,jgraphics_attr_setrgb_alias) }

    cdef t_max_err object_attr_getjrgba(void *ob, t_symbol *s, t_jrgba *c)
    cdef t_max_err object_attr_setjrgba(void *ob, t_symbol *s, t_jrgba *c)
    cdef void jrgba_to_atoms(t_jrgba *c, t_atom *argv)
    cdef t_max_err atoms_to_jrgba(long argc, t_atom *argv, t_jrgba *c)
    cdef void jrgba_set(t_jrgba *prgba, double r, double g, double b, double a)
    cdef void jrgba_copy(t_jrgba *dest, t_jrgba *src)
    cdef long jrgba_compare(t_jrgba *rgba1, t_jrgba *rgba2)
    cdef void jgraphics_getfiletypes(void *dummy, long *count, t_fourcc **filetypes, char *alloc)
    cdef t_max_err jbox_invalidate_layer(t_object *b, t_object *view, t_symbol *name)
    cdef t_max_err jbox_remove_layer(t_object *b, t_object *view, t_symbol *name)
    cdef t_jgraphics* jbox_start_layer(t_object *b, t_object *view, t_symbol *name, double width, double height)
    cdef t_max_err jbox_end_layer(t_object *b, t_object *view, t_symbol *name)
    cdef t_max_err jbox_paint_layer(t_object *b, t_object *view, t_symbol *name, double x, double y)
    cdef long jgraphics_rectintersectsrect(t_rect *r1, t_rect *r2)
    cdef long jgraphics_rectcontainsrect(t_rect *outer, t_rect *inner)
    cdef void jgraphics_position_one_rect_near_another_rect_but_keep_inside_a_third_rect( t_rect *positioned_rect, const t_rect *positioned_near_this_rect, const t_rect *keep_inside_this_rect)
    cdef void jgraphics_clip(t_jgraphics *g, double x, double y, double width, double height)



cdef extern from "ext_boxstyle.h":

    cdef void class_attr_setstyle(t_class *c, const char *s)
    cdef void class_attr_style_alias(t_class *c, const char *name, const char *aliasname, long legacy)
    cdef int FILL_ATTR_SAVE
    cdef void class_attr_setfill(t_class *c, const char *name, long flags)
    cdef void jgraphics_attr_fillrect(t_object *b, t_jgraphics *g, t_symbol *attrname, t_rect *area)
    cdef t_jpattern *jgraphics_attr_setfill(t_object *b, t_jgraphics *g, t_symbol *attrname, t_rect *area)
    cdef void object_attr_getfillcolor_atposition(t_object *b, const char *attrname, double pos, t_jrgba *c)
    cdef long object_attr_getfill(t_object *obj, t_symbol *attrname)
    cdef void object_style_setfillattribute(t_object *x, t_symbol *fillattr, t_symbol *entry, long argc, t_atom *argv)
    cdef void class_attr_stylemap(t_class *c, char *attrname, char *mapname)
    cdef t_symbol *object_attr_attrname_forstylemap(t_object *x, t_symbol *mapname)     
    cdef t_symbol *object_attr_stylemapname(t_object *x, t_symbol *attrname)            
    cdef t_jpopupmenu *style_getmenu(t_object *context, t_symbol *current, long mask, long *selecteditem, long *headercount)        
    cdef void style_handlemenu(t_object *context, long itemindex, t_symbol **current)

    cdef void CLASS_ATTR_STYLE_RGBA_NOSAVE(c,attrname,flags,structname,structmember,label)
    cdef void CLASS_ATTR_STYLE_RGBA(c,attrname,flags,structname,structmember,label)
    cdef void CLASS_ATTR_STYLE_RGBA_PREVIEW(c,attrname,flags,structname,structmember,label,previewtype)
    cdef void CLASS_ATTR_STYLE_ALIAS_NOSAVE(c,attrname,aliasname)
    cdef void CLASS_ATTR_STYLE_ALIAS_COMPATIBILITY(c,attrname,aliasname)
    cdef void CLASS_ATTR_STYLE_ALIAS_RGBA_LEGACY(c,attrname,aliasname)



cdef extern from "jdataview.h":

    cdef int JDATAVIEW_CELLMAX
        
    cdef int JCOLUMN_DEFAULT_WIDTH
    cdef int JCOLUMN_DEFAULT_MINWIDTH
    cdef int JCOLUMN_DEFAULT_MAXWIDTH
    cdef int JCOLUMN_DEFAULT_FONTSIZE 
    cdef int JCOLUMN_DEFAULT_MAXTEXTLEN

    ctypedef enum:
        JCOLUMN_ALIGN_LEFT = 1
        JCOLUMN_ALIGN_CENTER
        JCOLUMN_ALIGN_RIGHT

    ctypedef enum:
        JCOLUMN_STYLE_PLAIN = 0
        JCOLUMN_STYLE_BOLD = 1
        JCOLUMN_STYLE_ITALIC = 2

    ctypedef enum:
        JCOLUMN_MOUSE_ENTER = 0
        JCOLUMN_MOUSE_MOVE = 1
        JCOLUMN_MOUSE_EXIT = 2

    ctypedef enum:
        JCOLUMN_SORTDIRECTION_FORWARD = 1
        JCOLUMN_SORTDIRECTION_BACKWARD = 0

    ctypedef enum:
        JCOLUMN_INITIALLYSORTED_NONE = 0
        JCOLUMN_INITIALLYSORTED_FORWARDS = 1
        JCOLUMN_INITIALLYSORTED_BACKWARDS = 2

    ctypedef enum:
        JCOLUMN_COMPONENT_NONE = 0
        JCOLUMN_COMPONENT_CHECKBOX = 1
        JCOLUMN_COMPONENT_TEXTEDITOR = 2        
        JCOLUMN_COMPONENT_SLIDER = 3
        JCOLUMN_COMPONENT_COLOR = 4
        JCOLUMN_COMPONENT_MENU = 5

    ctypedef enum:
        JCOLUMN_MENU_INDEX = 1                  
        JCOLUMN_MENU_FONTMENU = 2               
        JCOLUMN_MENU_PAINT = 4                  
        JCOLUMN_MENU_SELECT = 8                 
        JCOLUMN_MENU_NOPANEL = 16               
        JCOLUMN_MENU_CLEARITEM = 32             
        JCOLUMN_MENU_STYLEMENU = 64             

    ctypedef enum:
        JCOLUMN_TEXT_ONESYMBOL = 1
        JCOLUMN_TEXT_COMMASEPARATED = 2
        JCOLUMN_TEXT_ICON = 4
        JCOLUMN_TEXT_HASBUTTON = 8
        JCOLUMN_TEXT_FILECHOOSEBUTTON = 16
        JCOLUMN_TEXT_VIEWBUTTON = 32
        JCOLUMN_TEXT_EDITBUTTON = 64
        JCOLUMN_TEXT_TIME = 128
        JCOLUMN_TEXT_FLOAT = 256
        JCOLUMN_TEXT_INT = 512
        JCOLUMN_TEXT_CANTEDIT = 1024
        JCOLUMN_TEXT_FONTSIZE = 2048
        
        JCOLUMN_TEXT_FILTERED = 8192
        JCOLUMN_TEXT_STRINGOBJECT = 16384
        JCOLUMN_TEXT_PITCH = 32768

    ctypedef enum:
        JCOLUMN_SLIDER_NOTEXTBOX = 1,
        JCOLUMN_SLIDER_ZERO_TO_ONE = 2

    cdef int JCOLUMN_DISABLED

    ctypedef struct t_celldesc
    ctypedef struct t_jcolumn
    ctypedef struct  t_jdataview
    ctypedef struct t_jdv_notifier
    ctypedef struct t_privatesortrec
    ctypedef struct t_jsection

    cdef void jdataview_initclass()
    cdef void *jdataview_new()
    cdef void jdataview_setclient(t_object *dv, t_object *client)
    cdef t_object *jdataview_getclient(t_object *dv)
    cdef void *jdataview_newsection(t_object *dv, char *name, void *assoc, t_jsurface *icon, char initiallyopen, char headervisible)
    cdef int jdataview_numsections(t_object *dv)
    cdef void *jdataview_getnthsection(t_object *dv, long index)
    cdef int jdataview_section_getnumrows(t_object *dv, void *section)
    cdef long jdataview_section_isopen(t_object *dv, void *section)
    cdef void jdataview_section_setopen(t_object *dv, void *section, long way)
    cdef void jdataview_getsectionopenness(t_object *dv, char **state)
    cdef void jdataview_setsectionopenness(t_object *dv, char *state)
    cdef long jdataview_section_headervisible(t_object *dv, void *section)
    cdef void jdataview_section_setheadervisible(t_object *dv, void *section, long way)
    cdef t_symbol *jdataview_section_getname(t_object *dv, void *section)
    cdef t_jsurface *jdataview_section_geticon(t_object *dv, void *section)
    cdef void jdataview_patchervis(t_object *dv, t_object *pv, t_object *box)
    cdef void jdataview_patcherinvis(t_object *dv, t_object *pv)
    cdef void jdataview_obscuring(t_object *dv, t_object *pv)
    cdef void jdataview_setheight(t_object *dv, long height)
    cdef long jdataview_getheight(t_object *dv)
    cdef void jdataview_setautoheight(t_object *dv, long way)
    cdef short jdataview_getautoheight(t_object *dv)
    cdef void jdataview_setcolumnheaderheight(t_object *dv, double height)
    cdef double jdataview_getcolumnheaderheight(t_object *dv)
    cdef void jdataview_setrowcolor1(t_object *dv, t_jrgba *c)
    cdef void jdataview_getrowcolor1(t_object *dv, t_jrgba *c)
    cdef void jdataview_setrowcolor2(t_object *dv, t_jrgba *c)
    cdef void jdataview_getrowcolor2(t_object *dv, t_jrgba *c)
    cdef void jdataview_getselectcolor(t_object *dv, t_jrgba *c)
    cdef void jdataview_setselectcolor(t_object *dv, t_jrgba *c)
    cdef void jdataview_setusegradient(t_object *dv, long way)
    cdef long jdataview_getusegradient(t_object *dv)
    cdef void jdataview_setcanselectmultiple(t_object *dv, long way)
    cdef short jdataview_getcanselectmultiple(t_object *dv)
    cdef void jdataview_setcancopy(t_object *dv, long way)
    cdef short jdataview_getcancopy(t_object *dv)
    cdef void jdataview_setcanpaste(t_object *dv, long way)
    cdef short jdataview_getcanpaste(t_object *dv)
    cdef void jdataview_setinset(t_object *dv, long inset)
    cdef long jdataview_getinset(t_object *dv)
    cdef void jdataview_setautosizeright(t_object *dv, long way)
    cdef long jdataview_getautosizeright(t_object *dv)
    cdef void jdataview_setautosizebottom(t_object *dv, long way)
    cdef long jdataview_getautosizebottom(t_object *dv)
    cdef void jdataview_setautosizerightcolumn(t_object *dv, long way)
    cdef long jdataview_getautosizerightcolumn(t_object *dv)
    cdef void jdataview_setusecharheightfont(t_object *dv, long way)
    ctypedef long (*t_containersizechange_fun)(t_object *x, double cw, double ch, double *width, double *height, int asr, int asb) 
    cdef t_atom_long jdataview_containersizechange(t_object *x, double cw, double ch, double *width, double *height, int asr, int asb)
    cdef t_max_err jdataview_gethorizscrollvalues(t_object *x, double *min, double *max, double *start, double *size)
    cdef void jdataview_sethorizscrollvalues(t_object *x, double start, double size)
    cdef t_max_err jdataview_getvertscrollvalues(t_object *x, double *min, double *max, double *start, double *size)
    cdef void jdataview_setvertscrollvalues(t_object *x, double start, double size)
    cdef t_max_err jdataview_setscrollvisible(t_object *x, long vbar, long hbar)
    cdef void jdataview_setborderthickness(t_object *dv, long val)
    cdef long jdataview_getborderthickness(t_object *dv)
    cdef void jdataview_setkeyfocusable(t_object *x, long val)
    cdef long jdataview_getkeyfocusable(t_object *x)
    cdef void jdataview_setenabledeletekey(t_object *dv, long way) 
    cdef long jdataview_getenabledeletekey(t_object *dv) 
    cdef void jdataview_setfontname(t_object *dv, t_symbol *fontname)
    cdef t_symbol *jdataview_getfontname(t_object *dv)
    cdef void jdataview_setfontsize(t_object *dv, double fsize)
    cdef double jdataview_getfontsize(t_object *dv)
    cdef double jdataview_getclientfontsize(t_object *dv)
    cdef void jdataview_columnheadermouse(t_object *dv, t_object *col, long msg)
    cdef int jdataview_getdragenabled(t_object *dv)
    cdef void jdataview_setdragenabled(t_object *dv, long way)
    cdef void jdataview_setcolumnheadercluemsg(t_object *dv, t_symbol *msg)
    cdef t_symbol *jdataview_getcolumnheadercluemsg(t_object *dv)
    cdef int jdataview_getdrawgrid(t_object *dv)
    cdef void jdataview_setdrawgrid(t_object *dv, int way)
    cdef void jdataview_setrowinset(t_object *dv, long top, long bottom)
    cdef void jdataview_getrowinset(t_object *dv, long *top, long *bottom)
    cdef t_object *jdataview_getsearchcolumn(t_object *dv)
    cdef void jdataview_setsearchcolumn(t_object *dv, t_object *col)
    cdef void jdataview_setoverridefocus(t_object *dv, long way)
    cdef long jdataview_getoverridefocus(t_object *dv)
    cdef void jdataview_setreturnkeycolumn(t_object *dv, t_object *col)
    cdef t_object *jdataview_getreturnkeycolumn(t_object *dv)
    cdef int jdataview_keynavigate(t_object *dv, char *buffer)
    cdef void jdataview_setheaderbgcolor(t_object *dv, t_jrgba *c)
    cdef void jdataview_setheadertextcolor(t_object *dv, t_jrgba *c)
    cdef void jdataview_getheaderbgcolor(t_object *dv, t_jrgba *c)
    cdef void jdataview_getheadertextcolor(t_object *dv, t_jrgba *c)
    cdef long jdataview_getheadersolidcolor(t_object *dv)
    cdef void jdataview_setheadersolidcolor(t_object *dv, long way)
    cdef void jdataview_setcontext(t_object *dv, void *context)
    cdef void *jdataview_getcontext(t_object *dv)
    cdef t_object *jdataview_addcolumn(t_object *dv, t_symbol *name, t_symbol *before, short unused)
    cdef t_object *jdataview_addcolumn_hidden(t_object *dv, t_symbol *name, t_symbol *before, short unused)
    cdef void *jcolumn_new()
    cdef void jcolumn_setdataview(t_object *c, t_object *dv)
    cdef void jdataview_colname_delete(t_object *dv, t_symbol *name)
    cdef void jdataview_deletecolumn(t_object *dv, t_object *col)
    cdef t_object *jdataview_getnamedcolumn(t_object *dv, t_symbol *name)
    cdef t_object *jdataview_getnthcolumn(t_object *dv, long index)
    cdef int jdataview_colname2index(t_object *dv, t_symbol *name)
    cdef void jdataview_colname_setvisible(t_object *dv, t_symbol *name, long way)
    cdef short jdataview_colname_getvisible(t_object *dv, t_symbol *name)
    cdef int jdataview_getnumcolumns(t_object *dv)
    cdef int jcolumn_getwidth(t_object *col)
    cdef void jcolumn_setwidth(t_object *col, long width)
    cdef int jcolumn_getmaxwidth(t_object *col)
    cdef void jcolumn_setmaxwidth(t_object *col, long width)
    cdef int jcolumn_getminwidth(t_object *col)
    cdef void jcolumn_setminwidth(t_object *col, long width)
    cdef long jcolumn_getid(t_object *col)
    cdef int jcolumn_getautosize(t_object *col)
    cdef void jcolumn_setdataview(t_object *col, t_object *dv)
    cdef t_symbol *jcolumn_getname(t_object *col)
    cdef void jcolumn_setname(t_object *col, t_symbol *name)
    cdef void jcolumn_setlabel(t_object *col, t_symbol *label)
    cdef t_symbol *jcolumn_getlabel(t_object *col)
    cdef void jcolumn_setinsertbefore(t_object *col, t_symbol *before)
    cdef t_symbol *jcolumn_getinsertbefore(t_object *col)
    cdef void jcolumn_setnumeric(t_object *col, long way)
    cdef int jcolumn_getnumeric(t_object *col)
    cdef void jcolumn_setcustomsort(t_object *col, t_symbol *msg)
    cdef t_symbol *jcolumn_getcustomsort(t_object *col)
    cdef void jcolumn_setoverridesort(t_object *col, char val)
    cdef char jcolumn_getoverridesort(t_object *col)
    cdef void jcolumn_setcustompaint(t_object *col, t_symbol *msg)
    cdef t_symbol *jcolumn_getcustompaint(t_object *col)
    cdef void jcolumn_setcustommenu(t_object *col, t_symbol *setmsg, t_symbol *resultmsg)
    cdef t_symbol *jcolumn_getcustommenu_setmsg(t_object *col)
    cdef t_symbol *jcolumn_getcustommenu_resultmsg(t_object *col)
    cdef void jcolumn_setsortable(t_object *col, long way)
    cdef int jcolumn_getsortable(t_object *col)
    cdef void jcolumn_setdraggable(t_object *col, long way)
    cdef int jcolumn_getdraggable(t_object *col)
    cdef void jcolumn_setinitiallysorted(t_object *col, long way)
    cdef int jcolumn_getinitiallysorted(t_object *col)
    cdef void jcolumn_sethideable(t_object *col, long way)
    cdef int jcolumn_gethideable(t_object *col)
    cdef void jcolumn_setvisible(t_object *col, long way)
    cdef int jcolumn_getvisible(t_object *col)
    cdef void jcolumn_setcasesensitive(t_object *col, long way)
    cdef int jcolumn_getcasesensitive(t_object *col)
    cdef void jcolumn_setreference(t_object *col, void *ref)
    cdef void *jcolumn_getreference(t_object *col)
    cdef void jcolumn_setcheckbox(t_object *col, t_symbol *msg)
    cdef void jcolumn_setvaluemsg(t_object *col, t_symbol *msg, t_symbol *beginmsg, t_symbol *endmsg)
    cdef t_symbol *jcolumn_getvaluemsg(t_object *col)
    cdef t_symbol *jcolumn_getbeginchangemsg(t_object *col)
    cdef t_symbol *jcolumn_getendchangemsg(t_object *col)
    cdef int jcolumn_getcomponent(t_object *col)
    cdef void jcolumn_setrowcomponentmsg(t_object *col, t_symbol *msg)
    cdef t_symbol *jcolumn_getrowcomponentmsg(t_object *col)
    cdef double jcolumn_getindentspacing(t_object *col)
    cdef void jcolumn_setindentspacing(t_object *col, double spacing)
    cdef void jcolumn_setcellcluemsg(t_object *col, t_symbol *msg)
    cdef t_symbol *jcolumn_getcellcluemsg(t_object *col)
    cdef t_symbol *jcolumn_getcelltextcolormsg(t_object *col)
    cdef void jcolumn_setcelltextcolormsg(t_object *col, t_symbol *msg)
    cdef t_symbol *jcolumn_getcelltextstylemsg(t_object *col)
    cdef void jcolumn_setcelltextstylemsg(t_object *col, t_symbol *msg)
    cdef void jcolumn_setcellentermsg(t_object *col, t_symbol *msg)
    cdef void jcolumn_setcellexitmsg(t_object *col, t_symbol *msg)
    cdef void jcolumn_setcellmovedmsg(t_object *col, t_symbol *msg)
    cdef void jcolumn_setcellclickmsg(t_object *col, t_symbol *msg)
    cdef void jcolumn_setshowinfobutton(t_object *col, long way)
    cdef long jcolumn_getshowinfobutton(t_object *col)
    cdef t_symbol *jcolumn_getcellentermsg(t_object *col)
    cdef t_symbol *jcolumn_getcellexitmsg(t_object *col)
    cdef t_symbol *jcolumn_getcellmovedmsg(t_object *col)
    cdef t_symbol *jcolumn_getcellclickmsg(t_object *col)
    cdef void jcolumn_update(t_object *col, t_symbol *msg)
    cdef t_object *jdataview_addcolumnfromdictionary(t_object *dv, t_object *d)
    ctypedef void *t_rowref
    cdef void jdataview_addrowtosection(t_object *dv, void *section, t_rowref rr)
    cdef void jdataview_addrow(t_object *dv, t_rowref rr)
    cdef void jdataview_addrowstosection(t_object *dv, void *section, long count, t_rowref *rrs)
    cdef void jdataview_addrows(t_object *dv, long count, t_rowref *rrs)
    cdef void jdataview_deleterowfromsection(t_object *dv, void *section, t_rowref rr)
    cdef void jdataview_deleterow(t_object *dv, t_rowref rr)
    cdef void jdataview_deleterowsfromsection(t_object *dv, void *section, long count, t_rowref *rrs)
    cdef void jdataview_deleterows(t_object *dv, long count, t_rowref *rrs)
    cdef void jdataview_deleteselectedrows(t_object *dv) 
    cdef void jdataview_deleteselectedrowsforview(t_object *dv, t_object *patcherview)
    cdef void jdataview_clear(t_object *dv)
    cdef int jdataview_getnumrows(t_object *dv)
    cdef void jdataview_gettextinrows(t_object *dv, t_rowref *rows, char *cellsep, char **text)
    cdef int jdataview_selectedrowcountforview(t_object *dv, t_object *patcherview)
    cdef int jdataview_selectedrowcount(t_object *dv)
    cdef t_rowref *jdataview_getallrows(t_object *dv)
    cdef t_rowref *jdataview_section_getallrows(t_object *dv, void *section, long *count)
    cdef t_rowref *jdataview_getselectedrowsforview(t_object *dv, t_object *patcherview)
    cdef t_rowref *jdataview_getselectedrows(t_object *dv)
    cdef void jdataview_applytoselectedrows(t_object *dv, t_symbol *msg, long bycell)
    cdef void jdataview_applytorows(t_object *dv, t_symbol *msg, long bycell, t_rowref *srs)
    cdef void jdataview_enablerow(t_object *dv, t_rowref rr, long way)
    cdef void jdataview_selectall(t_object *dv)
    cdef void jdataview_selectallforview(t_object *dv, t_object *patcherview)
    cdef void jdataview_selectnone(t_object *dv)
    cdef void jdataview_selectnoneforview(t_object *dv, t_object *patcherview)
    cdef t_object *jdataview_id2column(t_object *dv, int id)
    cdef t_symbol *jdataview_id2colname(t_object *dv, int id)
    cdef int jdataview_colname2id(t_object *dv, t_symbol *name)
    cdef int jdataview_column2id(t_object *dv, t_object *col)
    cdef int jdataview_row2id(t_object *dv, t_rowref rr, void **section)
    cdef t_rowref jdataview_id2row(t_object *dv, void *section, int id)
    cdef void jdataview_showrow(t_object *dv, t_rowref rr)
    cdef void jdataview_celldesc(t_jdataview *x, t_symbol *colname, t_rowref rr, t_celldesc *desc)
    cdef void jdataview_selectcellinview(t_object *dv, t_object *pv, t_symbol *colname, t_rowref rr)
    cdef void jdataview_selectcell(t_object *dv, t_symbol *colname, t_rowref rr)
    cdef int jdataview_getcelltextlength(t_object *dv, t_symbol *colname, t_rowref rr, long *length)
    cdef int jdataview_getcelltext(t_object *dv, t_symbol *colname, t_rowref rr, char *text, long maxlen)
    cdef int jdataview_getcellunits(t_object *dv, t_symbol *colname, t_rowref rr, char *text, long maxlen)
    cdef int jdataview_setcellunits(t_object *dv, t_symbol *colname, t_rowref rr, t_symbol *val)
    cdef int jdataview_getcellunitsyms(t_object *dv, t_symbol *colname, t_rowref rr, long *argc, t_atom **argv)
    cdef int jdataview_getcelldescription(t_object *dv, t_symbol *colname, t_rowref rr, char *text)
    cdef int jdataview_getcellvalue(t_object *dv, t_symbol *colname, t_rowref rr, long *argc, t_atom *argv)
    cdef void jdataview_getcelltextcolor(t_object *dv, t_symbol *colname, t_rowref rr, t_jrgba *color)
    cdef void jdataview_getcelltextstyle(t_object *dv, t_symbol *colname, t_rowref rr, long *style, long *align)
    cdef int jdataview_getcellmenu(t_object *dv, t_symbol *colname, t_rowref rr, long *argc, t_atom *argv, char **enabled, long *currentitemindex)
    cdef int jdataview_getcelltooltip(t_object *dv, t_symbol *colname, t_rowref rr, t_rect *cellrect, char *text, long maxlen) 
    cdef void jdataview_setcellvalue(t_object *dv, t_symbol *colname, t_rowref rr, long argc, t_atom *argv)
    cdef void jdataview_editcell(t_object *dv, t_symbol *colname, t_rowref rr)
    cdef int jdataview_iscelltextselected(t_object *dv, char justfocused)
    cdef void jdataview_cellclear(t_object *dv)
    cdef void jdataview_cellcut(t_object *dv)
    cdef void jdataview_cellcopy(t_object *dv)
    cdef void jdataview_cellpaste(t_object *dv)
    cdef int jdataview_getcellcomponent(t_object *dv, t_symbol *colname, t_rowref rr, long *options, t_symbol **label)
    cdef int jdataview_getcellfiletypes(t_object *dv, t_symbol *colname, t_rowref rr, long *count, t_fourcc **types, char *alloc)
    cdef t_symbol *jdataview_getcellfilterval(t_object *dv, t_symbol *colname, t_rowref rr)
    cdef void jdataview_redrawcell(t_object *dv, t_symbol *colname, t_rowref rr)
    cdef void jdataview_begincellchange(t_object *dv, t_symbol *colname, t_rowref rr)
    cdef void jdataview_endcellchange(t_object *dv, t_symbol *colname, t_rowref rr)
    cdef void jdataview_selected(t_object *dv, t_symbol *colname, t_rowref rr)
    cdef void jdataview_selectedrow(t_object *dv, t_rowref rr)
    cdef void jdataview_doubleclick(t_object *dv, t_symbol *colname, t_rowref rr)
    cdef void jdataview_contextualclick(t_object *dv, t_symbol *colname, t_rowref rr)
    cdef void jdataview_getcellicon(t_object *dv, t_symbol *colname, t_rowref rr, t_jsurface **surf)
    cdef void jdataview_getrowcolor(t_object *dv, t_rowref rr, long isoddrow, t_jrgba *c)
    cdef int jdataview_colorbycell(t_object *dv)
    cdef void jdataview_getcellcolor(t_object *dv, t_symbol *colname, t_rowref rr, long isoddrow, t_jrgba *c)
    cdef int jdataview_getcustomselectcolor(t_object *dv)
    cdef void jdataview_setcustomselectcolor(t_object *dv, int way)
    cdef double jdataview_getcellindent(t_object *dv, t_symbol *colname, t_rowref rr)
    cdef void jdataview_cellenter(t_object *dv, t_symbol *colname, t_rowref rr, int px, int py)
    cdef void jdataview_cellexit(t_object *dv, t_symbol *colname, t_rowref rr, int px, int py)
    cdef void jdataview_cellmove(t_object *dv, t_symbol *colname, t_rowref rr, int px, int py)
    cdef t_atom_long jdataview_getcelleditable(t_object *dv, t_symbol *colname, t_rowref rr)
    cdef long jdataview_getbordercolor(t_object *dv, t_jrgba *c)    
    cdef void jdataview_setbordercolor(t_object *dv, t_jrgba *c)
    cdef long jdataview_getusesystemfont(t_object *dv)
    cdef void jdataview_setusesystemfont(t_object *dv, long way)
    cdef void jdataview_enablecell(t_object *dv, t_symbol *colname, t_rowref rr, long way)
    cdef void jdataview_forcecellvisible(t_object *x, t_symbol *colname, t_rowref rr)
    cdef void jdataview_scrolltosection(t_object *dv, void *section)
    cdef void jdataview_scrolltotop(t_object *dv)
    cdef int jdataview_getpaintcellseparator(t_object *dv)
    cdef void jdataview_setpaintcellseparator(t_object *dv, int way)
    cdef void jdataview_getcellclue(t_object *dv, t_symbol *colname, t_rowref rr, char **str)
    cdef void jdataview_clientpaintcell(t_object *dv, t_symbol *msg, t_object *pv, t_symbol *colname, t_rowref rr, int width, int height, int rowIsSelected, int rowNumber)
    cdef void jdataview_getcolumnmenuforcell(t_object *dv, t_symbol *colname, t_rowref rr, long *argc, t_atom **argv, char **enabled)
    cdef void jdataview_cellcolumnmenuresult(t_object *dv, t_symbol *colname, t_rowref rr, long result)
    cdef void jdataview_sortcolumn(t_object *x, t_symbol *colname, int fwd)
    cdef void jdataview_sort(t_object *x, t_symbol *colname, int fwd)
    cdef void jdataview_resort(t_object *x)
    cdef long jdataview_getsortcolumn(t_object *x)
    cdef void jdataview_selectcell(t_object *dv, t_symbol *colname, t_rowref rr)
    cdef void jdataview_selectrow(t_jdataview *x, t_rowref rr)
    cdef void jdataview_selectcolumn(t_jdataview *x, t_symbol *colname)
    cdef void jdataview_selectallrows(t_jdataview *x)
    cdef long jdataview_iscellselected(t_jdataview *x, t_symbol *colname, t_rowref rr)
    cdef long jdataview_isrowselected(t_jdataview *x, t_rowref rr)
    cdef long jdataview_iscolumnselected(t_jdataview *x, t_symbol *colname)
    cdef void jdataview_savecolumnwidths(t_object *dv, t_dictionary **d)
    cdef void jdataview_restorecolumnwidths(t_object *dv, t_dictionary *d)
    cdef void jdataview_updatefontpanelforclient(t_object *dv, long show)
    cdef void jdataview_redrawrow(t_object *dv, t_rowref rr)
    cdef void jdataview_getcellrect(t_jdataview *x, t_symbol *colname, t_rowref rr, t_rect *r)
    cdef t_max_err jdataview_jgraphics_from_paintcontext(t_jdataview *x, void *ctx, t_jgraphics **g)
    cdef long jdataview_containersize(t_object *dv, t_object *box, double cw, double ch, double *width, double *height)
    cdef void jdataview_forcecellvisible(t_object *x, t_symbol *colname, t_rowref rr)
    cdef t_object *jdataview_getrowobject(t_object *dv, t_rowref rr)    
    cdef void jdataview_entermodalstate(t_object *dv, long way)
    cdef void *jdataview_getdragreceiver(t_object *dv)



