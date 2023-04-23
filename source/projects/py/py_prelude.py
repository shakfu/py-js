"""py_prelude.py: extends the `py` external with pure python functions

"""
import os
import subprocess
import shlex
import collections.abc
from functools import reduce

# ---------------------------------------------------------
# global constants

EDITOR = "Sublime Text"  # can be changed of course



# ---------------------------------------------------------
# private utilities

def is_sequence(obj):
    if isinstance(obj, str):
        return False
    return isinstance(obj, collections.abc.Sequence)


def __compose(f, g):
    return lambda x: f(g(x))


def __analyze(s: str):
    fs = []
    args = []
    kwargs = []
    str_args = s.split()
    for str_arg in str_args:
        if '=' in str_arg:
            k, v = str_arg.split('=')
            kwargs.append((
                eval(repr(k), locals(), globals()),
                eval(v, locals(), globals())
            ))
        else:
            elem = eval(str_arg, locals(), globals())
            if callable(elem):
                fs.append(elem)
            else:
                args.append(elem)
    return fs, args, kwargs


# ---------------------------------------------------------
# test funcs

identity = lambda x: x

add100 = lambda x: x+100

sub20 = lambda x: x-20

div2 = lambda x: x/2

mul2 = lambda x: x*2

mul10 = lambda x: x*10

mul5 = lambda x: x*5

mul6 = lambda x: x*7

sumargs = lambda *args, **kwargs: sum(args)

sumvals = lambda *args, **kwargs: sum(v for (k,v) in kwargs.items())

# ---------------------------------------------------------
# misc funcs

def edit(path: str):
    editor = os.getenv("EDITOR", EDITOR)
    path = os.path.expanduser(path)
    shell(f'open -a "{editor}" "{path}"')


# ---------------------------------------------------------
# funcs are used by methods

def shell(cmd: str, err_func=None):
    result = None
    try:
        elems = shlex.split(cmd)
        elems[-1] = os.path.expanduser(elems[-1])  # ~/a/b.c -> /Users/xx/a/b.c
        result = subprocess.check_output(elems, encoding="utf8").strip()
    except subprocess.CalledProcessError as e:
        if err_func:
            err_func(e.stderr)
    except FileNotFoundError as e:
        if err_func:
            err_func(e.strerror)
    if result:
        return result


def out_dict(py_dict: dict):
    res = []
    for k, v in py_dict.items():
        res.append(k)
        res.append(":")
        if type(v) in [list, set, tuple]:
            for i in v:
                res.append(i)
        else:
            res.append(v)
    return res



def pipe(s: str):
    """pipe variable(s) through a list of functions

    With a single variable a list of function:

    >>> pipe('10 math.sin math.cos')
    0.8556343548213665

    Acts like a chain of maps With many variables
    and many functions:

    >>> pipe('math.sin math.cos 10 20 30')
    [0.8556343548213665, 0.6114178044194122, 0.5503344099628433]

    Consistent with the pipe function can apply several
    variables to a single function like `sum` in this case:

    >>> pipe('math.sin math.cos sum 10 20 30')
    2.017386569203622

    :param      s:    code string
    :type       s:    str
    """
    fs, args, kwargs = __analyze(s)
    if args and fs:
        if len(args) == 1:
            arg = args[:].pop()
            for f in fs:
                arg = f(arg)
            return arg
        else:
            for f in fs:
                try:
                    args = list(map(f, args))
                except TypeError:
                    args = f(args)
            if args:
                return args



def call(s: str):
    """
    Applies args to a function

    >>> call('sum 1 2 3')
    6
    
    >>> f = lambda *args, **kwargs: print(args, kwargs)

    >>> call('f 10 20 a=1')
    (10, 20) {'a': 1}
    
    :param      s:    code string
    :type       s:    str
    """
    fs, args, kwargs = __analyze(s)
    if len(fs) == 1:
        f = fs[0]
        try:
            return f(*args, **dict(kwargs))
        except TypeError:
            return f(args, **dict(kwargs))
        except:
            return f(args[0])


def foldl(s: str):
    """
    Apply function of two arguments cumulatively to the items of iterable,
    from left to right, so as to reduce the iterable to a single value.

    >>> foldl('add 0 10 20 30 40')
    100
    
    :param      s:    code string
    :type       s:    str
    """
    fs, args, kwargs = __analyze(s)
    if len(fs) == 1:
        f = fs[0]
        accum, seq = args[0], args[1:]
        return reduce(f, seq, accum)




