# hello.py
"""

This is an demo of a python script in the `py` package.


"""

import api # cythonized max c api

# basic examples
a = 10

b = 1.5

c = "HELLO WORLD!!!"

d = [1,2,3,4]

e = ['a','b', 'c']

f = lambda: "hello func"

g = lambda x: x+10


def pipe(arg):
    args = arg.split()
    val = eval(args[0])
    funcs = [eval(f) for f in args[1:]]
    for f in funcs:
        val = f(val)
    return val
