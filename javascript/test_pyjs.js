var pyjs = new PyJS();	// instantiate a simplejs object

// trigger the print method of simplejs
function bang()
{
	pyjs.print();
}

function set(val)
{
	pyjs.myattr = val;	// set the value of myattr
}

// output the contents of myattr
function get()
{
	outlet(0, pyjs.myattr);
}


function py(val)
{
	outlet(0, pyjs.code(val));
}

// sends a value to simplejs and gets the result back
function abs(val)
{
	outlet(0, pyjs.doAbs(val));
}