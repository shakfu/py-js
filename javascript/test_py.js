var py = new PyExternal();	// instantiate a py object

// get the name of the py object
function bang()
{
	outlet(0, py.name);
}
