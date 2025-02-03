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

def test_atom_setlong_array():
    ext = api.PyExternal()
    a1 = api.Atom.from_str("foo bar 1 2 3.0")
    a1.setlong_array([10, 20, 30, 40, 50])
    ext.out(a1.to_list())

def test_atom_getlong_array():
    ext = api.PyExternal()
    a1 = api.Atom.from_str("foo bar 1 2 3.0")
    a1.setlong_array([10, 20, 30, 40, 50])
    ext.out(a1.getlong_array(5))

def test_atom_setfloat_array():
    ext = api.PyExternal()
    a1 = api.Atom.from_str("foo bar 1 2 3.0")
    a1.setfloat_array([10.0, 20.0, 30.0, 40.0, 50.0])
    ext.out(a1.to_list())

def test_atom_getfloat_array():
    ext = api.PyExternal()
    a1 = api.Atom.from_str("foo bar 1 2 3.0")
    a1.setfloat_array([10.0, 20.0, 30.0, 40.0, 50.0])
    ext.out(a1.getfloat_array(5))

def test_atom_setdouble_array():
    ext = api.PyExternal()
    a1 = api.Atom.from_str("foo bar 1 2 3.0")
    a1.setdouble_array([10.0, 20.0, 30.0, 40.0, 50.0])
    ext.out(a1.to_list())

def test_atom_getdouble_array():
    ext = api.PyExternal()
    a1 = api.Atom.from_str("foo bar 1 2 3.0")
    a1.setdouble_array([10.0, 20.0, 30.0, 40.0, 50.0])
    ext.out(a1.getdouble_array(5))
