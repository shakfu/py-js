import os
import subprocess
import shlex

EDITOR = 'Sublime Text' # can be changed of course

# can be be disabled by default for safety
def shell(cmd, err_func=None):
    result = None
    try:
        result = subprocess.check_output(
            shlex.split(cmd), encoding='utf8').strip()
    except subprocess.CalledProcessError as e:
        if err_func:
            err_func(e.stderr)
    except FileNotFoundError as e:
        if err_func:
            err_func(e.strerror)
    if result:
        return result


def edit(path):
    editor = os.getenv('EDITOR', EDITOR)
    shell(f'open -a "{editor}" "{path}"')



# ---------------------------------------------------------
# protected funcs 

def __py_maxmsp_out_dict(py_dict):
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

def __py_maxmsp_pipe(arg):
    args = arg.split()
    val = eval(args[0], locals(), globals())
    funcs = [eval(f, locals(), globals()) for f in args[1:]]
    for f in funcs:
        val = f(val)
    return val

