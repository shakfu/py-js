lines = """\
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
""".splitlines()

import re

pattern = re.compile(r"^(\w+) ([\w|\*]+) ([\w|\*]+)\((.+)\)")
mx = lambda s: f'mx.{s}'
mp = lambda s: f'mp.{s}'

startswith_t = lambda s: s.startswith('t_')
is_named = lambda s: s in ['method']
type_tests = [startswith_t, is_named]
is_mtype = lambda s: any(f(s) for f in type_tests)

prefix = lambda s: mx(s) if is_mtype(s) else s

def depoint(typ, name):
    starcount = sum(i=='*' for i in name)
    if starcount > 0:
        name = name.replace('*','')
        stars = '*' * starcount
        typ = f'{typ}{stars}'
    return (typ, name)

def deprefix(s):
    head, *tail = s.split('_')
    return '_'.join(tail)

def funcdef(cdef, ftype, fname, args):
    _args = []
    for arg in args:
        typ, name = arg.split()
        typ, name = depoint(typ, name)
        _args.append(f"{prefix(typ)} {name}")
    _args.insert(0, 'self')
    _args = ", ".join(_args)
    print(f"cdef {prefix(ftype)} {deprefix(fname)}({_args}):")

def funcbody(ftype, fname, args, **replace):
    _args = []
    for arg in args:
        typ, name = arg.split()
        _, _name = depoint(typ, name)
        _args.append(_name)
    if replace:
        head, *tail = _args
        if head in replace:
            head = replace[head]
        _args = [head] + tail
    _args = ", ".join(_args)
    space = " "*4
    if ftype == 'void':
        print(f"{space} {mx(fname)}({_args})")
    else:
        print(f"{space} return {mx(fname)}({_args})")

errors = []

for line in lines:
    m = pattern.match(line)
    if m:
        # print(m.groups())
        cdef, ftype, fname, args = m.groups()
        ftype, fname = depoint(ftype, fname)
        args = [arg.strip() for arg in args.split(',')]
        try:
            funcdef(cdef, ftype, fname, args)
            funcbody(ftype, fname, args, x='self.lst')
            print()
        except ValueError:
            errors.append(line)
print()
print('# ERRORS')
for err in errors:
    print('#', err)
    print()




