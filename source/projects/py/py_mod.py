import os
# import subprocess

EDITOR = 'Sublime Text'

# disabled by default for safety
# 
# def sysout(cmd):
#     result = None
#     try:
#         result = subprocess.check_output(
#             cmd.split(), encoding='utf8').strip()
#     except:
#         pass
#     return result


def edit(path):
    editor = os.getenv('EDITOR', EDITOR)
    os.system(f'open -a "{editor}" "{path}"')



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

