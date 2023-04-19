import os
import subprocess
import shlex

# ---------------------------------------------------------
# global constants

EDITOR = 'Sublime Text' # can be changed of course


# ---------------------------------------------------------
# misc funcs

def edit(path):
    editor = os.getenv('EDITOR', EDITOR)
    path = os.path.expanduser(path)
    shell(f'open -a "{editor}" "{path}"')



# ---------------------------------------------------------
# funcs are used by methods

def shell(cmd, err_func=None):
    result = None
    try:
        elems = shlex.split(cmd)
        elems[-1] = os.path.expanduser(elems[-1]) # ~/a/b.c -> /Users/xx/a/b.c
        result = subprocess.check_output(
            elems, encoding='utf8').strip()
    except subprocess.CalledProcessError as e:
        if err_func:
            err_func(e.stderr)
    except FileNotFoundError as e:
        if err_func:
            err_func(e.strerror)
    if result:
        return result


def out_dict(py_dict):
    res = []
    for k,v in py_dict.items():
        res.append(k)
        res.append(':')
        if type(v) in [list, set, tuple]:
            for i in v:
                res.append(i)
        else:
            res.append(v)
    return res

def pipe(arg):
    args = arg.split()
    val = eval(args[0], locals(), globals())
    funcs = [eval(f, locals(), globals()) for f in args[1:]]
    for f in funcs:
        val = f(val)
    return val

