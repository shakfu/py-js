# dispatch.py

"""

patterns to parse:

1. func arg1 arg2 key1=val1 key2=val2

2. func arg1 arg2 key1:val1 key2:val2

3. func arg1 arg2 dict(key1=val1, key2=val2)

4. obj.func arg1 arg2 key1=val1 key2=val2

"""

def func(*args, **kwargs):
    return args, kwargs


class Klass:
    def func(self, *args, **kwargs):
        return args, kwargs

obj = Klass()

gdict = {'func': func, 'obj': obj}


s1 = 'func arg1 arg2 key1=val1 key2=val2'


head = lambda x: x[:1]
tail = lambda x: x[1:]


def process_s1(s, sep='='):
    
    args=[]
    kwargs=[]
    elems = s.split()
    f = head(elems)[0]
    for elem in tail(elems):
        if mapping_symbol in elem:
            kwargs.append(elem)
        else: args.append(elem)

    targs = []
    for arg in args:
        if '.' in arg:
            try:
                targs.append(float(arg))
            except TypeError:
                pass
        try:
            targs.append(int(arg))
        except TypeError:
            pass
        



