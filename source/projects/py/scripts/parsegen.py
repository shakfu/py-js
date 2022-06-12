"""
script to parse related functions in a .pxd and convert them to 
functions which can be include in an extension type

note: does not try to be perfect: first arg of function 
is not deliberately included and not converted to self
to help manual check if conversion is correct.

should try to see if this can be done more
systematically by use of pycparser (see python cffi for examples)

"""

lines = """\
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
cdef t_max_err atom_alloc(long *ac, t_atom **av, char *alloc)
cdef t_max_err atom_alloc_array(long minsize, long *ac, t_atom **av, char *alloc)
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




