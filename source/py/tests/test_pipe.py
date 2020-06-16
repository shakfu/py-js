
"""
list reversed range 5 -> list(reversed(range(5)))

t1 = range(5)
t2 = reversed(t1)
t3 = list(t2)
return t3

pipe 5 range reversed list

"""

def pipe(x, funcs):
    res = x
    for f in funcs:
        res = f(res)
    return res

def test_pipe():
    # pipe 5 range reversed list
    assert pipe(5, [range, reversed, list]) == list(reversed(range(5)))


def topipe(arg):
    args = arg.split()
    assert args[0] == 'pipe', "does not begin with pipe"
    val = eval(args[1])
    funcs = [eval(f) for f in args[2:]]

    return pipe(val, funcs)


def test_topipe():
    assert topipe("pipe 5 range reversed list") == pipe(5, [range, reversed, list])


a = 5   

def test_topipe2():
    assert topipe("pipe a range reversed list") == pipe(5, [range, reversed, list])
