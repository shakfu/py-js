import api


mem = {}


def test_path_init():
	mem['p'] = api.Path('test_api_path.py')
	p = mem['p']
	api.bang_success()

def test_path_filename():
	p = mem['p']
	return p.filename

def test_path_pathname():
	p = mem['p']
	return p.pathname

def test_path_absolute():
	p = mem['p']
	return p.absolute

def test_path_type():
	p = mem['p']
	return p.type

def test_path_creator():
	p = mem['p']
	return p.creator

def test_path_open():
	with Path('/tmp/test.txt').open('w') as f:
		f.write("hello world")

def test_path_app_path():
	return api.Path.from_maxapp().pathname

def test_path_tempfolder():
	return api.Path.from_tempfolder().pathname

def test_path_desktopfolder():
	return api.Path.from_desktopfolder().pathname

def test_path_userdocfolder():
	return api.Path.from_userdocfolder().pathname

def test_path_usermaxfolder():
	return api.Path.from_usermaxfolder().pathname

def test_path_support():
	return api.Path.from_support_folder().pathname

def test_path_default():
	return api.Path.from_default().pathname

def test_path_moddate():
	p = mem['p']
	return p.get_moddate()

def test_path_file_moddate():
	p = mem['p']
	return p.get_file_moddate()


