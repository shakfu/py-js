import api


mem = {}


def test_path_init():
	mem['p'] = api.Path('test_api_path.py')
	p = mem['p']
	return p.pathname


def test_path_filename():
	p = mem['p']
	return p.filename


def test_path_type():
	p = mem['p']
	return p.type


def test_path_creator():
	p = mem['p']
	return p.creator
