var pyjs = new PyJS();

// ---------------------------------------------------------------------------
// Properties

function set_name(val)
{
	pyjs.name = val;
}

function get_name()
{
	outlet(0, pyjs.name);
}

function set_file(val)
{
	pyjs.file = val;
}

function get_file()
{
	outlet(0, pyjs.file);
}

function set_pythonpath(val)
{
	pyjs.pythonpath = val;
}

function get_pythonpath()
{
	outlet(0, pyjs.pythonpath);
}

function set_debug(val)
{
	pyjs.debug = val;
}

function get_debug()
{
	outlet(0, pyjs.debug);
}

// ---------------------------------------------------------------------------
// Methods


// not working!
function pj(str) {
	var arr = arrayfromargs(arguments);
	return pyjs.code(arr);
}

// not working
function bang()
{
	pyjs.exec("from sys import version");
	var res = pj("version");
	outlet(0, res);
}

function pyload(file)
{
	pyjs.execfile(file);
}

function py()
{
	var arr = arrayfromargs(arguments);
	// post(arr + "\n");
	var res = pyjs.code(arr)
	if (res)
		outlet(0, res);
}
