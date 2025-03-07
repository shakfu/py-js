"""
Provide a python function `list_to_dict` to convert a python list with colon separators into a python dictionary. 

The following is an example of how the function would work:


```python
xs = [4, ':', 5, 6, 7, ':', 10]
assert list_to_dict(xs) == {4: [5, 6], 7: 10}
```

"""


def out_dict(py_dict: dict):
    """Output python dict in Max dict-syntax
    
    >>> out_dict({'a':1, 'b': [1,2,3,4]})
    ['a', ':', 1, 'b', ':', 1, 2, 3, 4]
    """
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


def to_string(func, *args, **kwds):
    """creates max-friendly function calling syntax arguments
    
    >>> to_string('f2', 1, 2, 3, a=10, b=[1,2])
    'f2 1 2 3 a : 10 b : 1 2'
    """
    res = [func]
    res.extend(args)
    res.extend(out_dict(kwds))
    return " ".join(str(i) for i in res)


def list_to_dict(xs, d={}):
    """converts a max dict syntax represented as a list to a python dict
    
    >>> xs = [4, ':', 5, 'a', 'b', ':', 10, 'abv', 1, ':', 23]
    >>> list_to_dict(xs)
    {4: [5, 'a'], 'b': [10, 'abv'], 1: 23}
    """
    if not xs:
        return d
    seps =  []
    n = len(xs)
    for i, o in enumerate(xs):
        if o == ':':
            seps.append(i)

    it = iter(seps)
    start = next(it)
    try:
        end = next(it)
        key = xs[0]
        values = xs[start+1:end-1]
        if len(values) == 1:
            values = values[0]
        d[key] = values
    except StopIteration:
        key = xs[0]
        values = xs[start+1:]
        if len(values) == 1:
            values = values[0]
        d[key] = values
        return d
    return f(xs[end-1:], d)


def __list_to_dict(xs, d={}):
    """converts a max dict syntax represented as a list to a python dict
    
    >>> xs = ['a', ':', 5, 'a', 'b', ':', 10, 'abv', 'c', ':', 23]
    >>> __list_to_dict(xs)
   {'a': [5, 'a'], 'b': [10, 'abv'], 'c': 23}
    """
    if not xs:
        return d
    seps =  []
    n = len(xs)
    for i, o in enumerate(xs):
        if o == ':':
            seps.append(i)

    it = iter(seps)
    start = next(it)
    try:
        end = next(it)
        key = xs[0]
        # print("key1:", key)
        if not isinstance(key, str) or is_keyword(key) or not key.isidentifier():
            raise ValueError(f'key {key} is not a valid python identifier')
        values = xs[start+1:end-1]
        if len(values) == 1:
            values = values[0]
        d[key] = values
    except StopIteration:
        key = xs[0]
        # print("key2:", key)
        if not isinstance(key, str) or is_keyword(key) or not key.isidentifier():
            raise ValueError(f'key {key} is not a valid python identifier')
        values = xs[start+1:]
        if len(values) == 1:
            values = values[0]
        d[key] = values
        return d
    return __list_to_dict(xs[end-1:], d)

def list_to_dict(xs: list) -> dict:
    """claude.ai's simplification of my version!"""
    result = {}
    if not xs:
        return result
    
    # Find all separator positions
    seps = [i for i, x in enumerate(xs) if x == ':']
    
    if not seps:
        return result
    
    # Process each key-value pair
    prev_idx = 0
    for i in range(len(seps)):
        sep_idx = seps[i]
        key = xs[prev_idx]
        
        # Determine end of current value section
        if i < len(seps) - 1:
            end_idx = seps[i + 1] - 1
        else:
            end_idx = len(xs)
        
        # Extract values
        values = xs[sep_idx + 1:end_idx]
        
        # If single value, don't keep it as a list
        if len(values) == 1:
            values = values[0]
            
        result[key] = values
        prev_idx = end_idx
    
    return result



def from_string(s: str):
    """converts a max-friendly function calling 
    syntax from a string to py objects

    >>> s = 'f 1 2 3 a : 5 6 b : 10'
    >>> from_string(s)
    f(1, 2, 3, a=[5, 6], b=10)

    """
    args = []
    kwds = []
    xs = s.split()
    f = xs[0]
    xs = xs[1:]
    if ':' in xs:
        z = xs.index(':')
        kwds = xs[z-1:]
        args = xs[:z-1]
    else:
        kwds = []
        args = xs
    return f, tuple(args), list_to_dict(kwds, d={})





