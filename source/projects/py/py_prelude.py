"""py_prelude.py: extends the `py` external with pure python functions





"""
import os
import subprocess
import shlex

# ---------------------------------------------------------
# global constants

EDITOR = "Sublime Text"  # can be changed of course


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
    """pipe a variable through a list of functions

    >>> pipe('10 math.sin math.cos')
    0.8556343548213665

    :param      s:    { parameter_description }
    :type       s:    { type_description }
    """
    str_args = s.split()
    val = eval(str_args[0], locals(), globals())
    funcs = [eval(f, locals(), globals()) for f in str_args[1:]]
    for f in funcs:
        val = f(val)
    return val


def rpipe(s: str):
    """pipe a list of variables through a list of functions

    >>> rpipe('math.sin math.cos 10 20 30')
    0.8556343548213665

    :param      s:    { parameter_description }
    :type       s:    { type_description }
    """
    str_args = s.split()
    fs = []
    vs = []
    objs = [eval(x, locals(), globals()) for x in str_args]
    for i in objs:
        if isinstance(i, (types.FunctionType, types.BuiltinFunctionType)):
            fs.append(i)
        else:
            vs.append(i)
    for f in reversed(fs):
        vs = list(map(f, vs))
    return vs

def sumrpipe(s):
    """pipe a list of variables through a list of functions and sum the result

    >>> sumrpipe('math.sin math.cos 10 20 30')
    0.19353298442897152

    :param      s:    { parameter_description }
    :type       s:    { type_description }
    """
    return sum(rpipe(s))


def call(s: str):
    """
    Applies args to a function

    >>> call('sum 1 2 3')
    6
    
    >>> f = lambda *args, **kwargs: print(args, kwargs)

    >>> call('f 10 20 a=1')
    (10, 20) {'a': 1}
    
    :param      s:    { parameter_description }
    :type       s:    { type_description }
    """
    str_args = s.split()
    args = []
    kwargs = []
    # if len(str_args) > 2:
    f = eval(str_args[0], locals(), globals())
    for str_arg in str_args[1:]:
        if '=' in str_arg:
            k, v = str_arg.split('=')
            kwargs.append((
                eval(repr(k), locals(), globals()),
                eval(v, locals(), globals())
            ))
        else:
            args.append(eval(str_arg, locals(), globals()))
    try:
        return f(*args, **dict(kwargs))
    except TypeError:
        return f(args, **dict(kwargs))
    except:
        return f(args[0])

