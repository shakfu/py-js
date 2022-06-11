"""
script to parse related functions in a .pxd and convert them to 
functions which can be include in an extension type

note: does not try to be perfect: first arg of function 
is not deliberately included and not converted to self
to help manual check if conversion is correct.

"""

lines = """\
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
            funcbody(ftype, fname, args, x='self.x')
            print()
        except ValueError:
            errors.append(line)
print()
print('# ERRORS')
for err in errors:
    print('#', err)
    print()




