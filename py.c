/**
	@file
	py - a python max object
	shakfu - github.com/shakfu

	@ingroup	examples
*/

#include "ext.h"			// standard Max include, always required
#include "ext_obex.h"		// required for new style Max object

////////////////////////// object struct
typedef struct _py
{
	t_object	ob;
	t_atom		val;
	t_symbol	*name;
	void		*out;
} t_py;

///////////////////////// function prototypes
//// standard set
void *py_new(t_symbol *s, long argc, t_atom *argv);
void py_free(t_py *x);
void py_assist(t_py *x, void *b, long m, long a, char *s);

void py_int(t_py *x, long n);
void py_float(t_py *x, double f);
void py_anything(t_py *x, t_symbol *s, long ac, t_atom *av);
void py_bang(t_py *x);
void py_identify(t_py *x);
void py_dblclick(t_py *x);
void py_acant(t_py *x);

//////////////////////// global class pointer variable
void *py_class;


void ext_main(void *r)
{
	t_class *c;

	c = class_new("py", (method)py_new, (method)py_free, (long)sizeof(t_py),
				  0L /* leave NULL!! */, A_GIMME, 0);

	class_addmethod(c, (method)py_bang,			"bang", 0);
	class_addmethod(c, (method)py_int,			"int",		A_LONG, 0);
	class_addmethod(c, (method)py_float,		"float",	A_FLOAT, 0);
	class_addmethod(c, (method)py_anything,		"anything",	A_GIMME, 0);
	class_addmethod(c, (method)py_identify,		"identify", 0);
	CLASS_METHOD_ATTR_PARSE(c, "identify", "undocumented", gensym("long"), 0, "1");

	// we want to 'reveal' the otherwise hidden 'xyzzy' method
	class_addmethod(c, (method)py_anything,		"xyzzy", A_GIMME, 0);
	// here's an otherwise undocumented method, which does something that the user can't actually
	// do from the patcher however, we want them to know about it for some weird documentation reason.
	// so let's make it documentable. it won't appear in the quickref, because we can't send it from a message.
	class_addmethod(c, (method)py_acant,			"blooop", A_CANT, 0);
	CLASS_METHOD_ATTR_PARSE(c, "blooop", "documentable", gensym("long"), 0, "1");

	/* you CAN'T call this from the patcher */
	class_addmethod(c, (method)py_assist,			"assist",		A_CANT, 0);
	class_addmethod(c, (method)py_dblclick,			"dblclick",		A_CANT, 0);

	CLASS_ATTR_SYM(c, "name", 0, t_py, name);

	class_register(CLASS_BOX, c);
	py_class = c;
}

void py_acant(t_py *x)
{
	object_post((t_object *)x, "can't touch this!");
}

void py_assist(t_py *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { //inlet
		sprintf(s, "I am inlet %ld", a);
	}
	else {	// outlet
		sprintf(s, "I am outlet %ld", a);
	}
}

void py_free(t_py *x)
{
	;
}

void py_dblclick(t_py *x)
{
	object_post((t_object *)x, "I got a double-click");
}

void py_int(t_py *x, long n)
{
	atom_setlong(&x->val, n);
	py_bang(x);
}

void py_float(t_py *x, double f)
{
	atom_setfloat(&x->val, f);
	py_bang(x);
}

void py_anything(t_py *x, t_symbol *s, long ac, t_atom *av)
{
	if (s == gensym("xyzzy")) {
		object_post((t_object *)x, "A hollow voice says 'Plugh'");
	} else {
		atom_setsym(&x->val, s);
		py_bang(x);
	}
}

void py_bang(t_py *x)
{
	switch (x->val.a_type) {
	case A_LONG: outlet_int(x->out, atom_getlong(&x->val)); break;
	case A_FLOAT: outlet_float(x->out, atom_getfloat(&x->val)); break;
	case A_SYM: outlet_anything(x->out, atom_getsym(&x->val), 0, NULL); break;
	default: break;
	}
}

void py_identify(t_py *x)
{
	object_post((t_object *)x, "my name is %s", x->name->s_name);
}

void *py_new(t_symbol *s, long argc, t_atom *argv)
{
	t_py *x = NULL;

	if ((x = (t_py *)object_alloc(py_class))) {
		x->name = gensym("");
		if (argc && argv) {
			x->name = atom_getsym(argv);
		}
		if (!x->name || x->name == gensym(""))
			x->name = symbol_unique();

		atom_setlong(&x->val, 0);
		x->out = outlet_new(x, NULL);
	}
	return (x);
}
