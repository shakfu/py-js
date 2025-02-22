import api


def test_matrix_init():
    m = api.Matrix("this")


def test_matrix_info():
    m = api.Matrix("this")
    api.post(f"size: {m.size}")
    api.post(f"type: {m.type}")
    api.post(f"flags: {m.flags}")
    api.post(f"dimcount: {m.dimcount}")
    api.post(f"dim: {m.dim}")
    api.post(f"dimstride: {m.dimstride}")
    api.post(f"planecount: {m.planecount}")


def test_matrix_get_data():
    m = api.Matrix("this")
    return m.get_data()


def test_matrix_set_data():
    m = api.Matrix("this")
    a = api.Atom.from_seq([3] * m.size)
    m.set_data(a)
