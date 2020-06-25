/*

This is a just a placeholder.

Wrapping an external for js usage requires that the output
function is changed to returm arrays instead of pushing to 
outlets.

Code below 'works' if the class is created as follows

	c->c_flags = CLASS_FLAG_POLYGLOT;
	class_register(CLASS_NOBOX, c);

But is otherwise pretty useless unless py can return atom arrays to js

TBD later (maybe)

*/


var py = new PyExternal();	// instantiate a py object

// get the name of the py object
function bang()
{
	outlet(0, py.name);
}
