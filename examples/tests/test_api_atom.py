import api


def test_atom_init():
    ext = api.PyExternal()
    a1 = api.Atom([1, 2.5, b"hello", "world"])
    ext.out(a1.to_list())

def test_atom_from_seq():
    ext = api.PyExternal()
    a1 = api.Atom.from_seq([10, 20121.234, "world", b"hello"])
    ext.out(a1.to_list())

def test_atom_from_str():
    ext = api.PyExternal()
    a1 = api.Atom.from_str("foo bar 1 2 3.0")
    assert a1.size == 5
    ext.out(a1.to_list())

def test_atom_to_str():
    parsestr = "foo bar 1 2 3.0"
    ext = api.PyExternal()
    a1 = api.Atom.from_str(parsestr)
    assert a1.size == 5
    api.post(a1.to_string())
