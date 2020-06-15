"""
goose : honk pig : 0.05 frog : -8 horse : 1 2 foo 3 duck : "quack quack"

"""
def pydict_to_mxdict(d):
    res = []
    for k,v in d.items():
        res.append(k)
        res.append(':')
        if type(v) in [list, set, tuple]:
            for i in v:
                res.append(i)
        else:
            res.append(v)
    return res


def test_pydict_to_mxdict():
    d = dict(a='b', c=1.2, d=[1, 2, 34])
    assert " ".join(pydict_to_mxdict(d)) == 'a : b c : 1.2 d : 1 2 34'

