#include "ext.h"
#include "ext_obex.h"


typedef struct _pyjss
{
	t_object ob;
	double myattr;
} t_pyjs;

void *pyjs_new(t_symbol *s, long argc, t_atom *argv);
void pyjs_free(t_pyjs *x);
void pyjs_print(t_pyjs *x, t_symbol *s, long ac, t_atom *av);
t_max_err pyjs_doabs(t_pyjs *x, t_symbol *s, long ac, t_atom *av, t_atom *rv);


static t_class *pyjs_class;


void ext_main(void *r)
{
	t_class *c;

	c = class_new("pyjs", 
		(method)pyjs_new, 
		(method)pyjs_free, 
		(long)sizeof(t_pyjs),
		0L /* leave NULL!! */, A_GIMME, 0);

	class_addmethod(c, (method)pyjs_print, "print",	0);
	class_addmethod(c, (method)pyjs_doabs, "doAbs",	A_GIMMEBACK, 0);

	CLASS_ATTR_DOUBLE(c, "myattr", 0, t_pyjs, myattr);

	c->c_flags = CLASS_FLAG_POLYGLOT;
	class_register(CLASS_NOBOX, c);
	pyjs_class = c;
}

void pyjs_free(t_pyjs *x)
{
	;
}

void *pyjs_new(t_symbol *s, long argc, t_atom *argv)
{
	t_pyjs *x = NULL;

	// object instantiation, NEW STYLE
	if ((x = (t_pyjs *)object_alloc(pyjs_class))) {
		// Initialize values
		x->myattr = 74.;
	}
	return (x);
}

void pyjs_print(t_pyjs *x, t_symbol *s, long ac, t_atom *av)
{
	post("The value of myattr is: %f", x->myattr);
}

t_max_err pyjs_doabs(t_pyjs *x, t_symbol *s, long ac, t_atom *av, t_atom *rv)
{
	t_atom a[1];
	double f = 0;

	if (ac) {
		if (atom_gettype(av) == A_LONG)
            f = (double)llabs(atom_getlong(av));
		else if( atom_gettype(av) == A_FLOAT)
			f = fabs(atom_getfloat(av));
	} else
		error("missing argument for method doAbs()");

	// store the result in the a array.
	atom_setfloat(a, f);

	// return the result to js
	atom_setobj(rv, object_new(gensym("nobox"), gensym("atomarray"), 1, a));

	return MAX_ERR_NONE;
}



