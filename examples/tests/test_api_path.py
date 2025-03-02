import api


def test_path_init():
	p = api.Path('test_api_path.py')
	return p.pathname
