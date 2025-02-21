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

