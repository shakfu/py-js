//
// 	The oscar extension for max
//
//	Oscar is the first name of the man commonly known as The Wizard of Oz
//		(we are pulling the strings of Max to automate its operation)
//	Oscar is also the name of the classic Grouch on Sesame Street
//		(because testing is an activity that can make us grumpy)
//	
//	Tim Place
//	Cycling '74
//

#include "oscar.h"


// Globals
t_symbol *ps_testmaster;
t_symbol *ps_testport;
t_atom_long g_port_send = 0;
t_atom_long g_port_listen = 0;


// Entry
void ext_main(void *r)
{
	common_symbols_init();
	
	testmaster_classinit();
	testrunner_classinit();
	testunit_classinit();
	testdb_classinit();
	testport_classinit();
	testassert_classinit();
	testequals_classinit();
	testlog_classinit();
	testterminate_classinit();
	testsample_classinit();
	
	ps_testmaster = gensym("test.master");
	ps_testport = gensym("test.port");

	ps_testmaster->s_thing = (t_object*)object_new_typed(_sym_nobox, ps_testmaster, 0, NULL);
	
	defer_low(ps_testmaster->s_thing, (method)deferred_startup, NULL, 0, NULL);
	quittask_install((method)testmaster_quittask, NULL);

	object_method(gensym("max")->s_thing, gensym("setmirrortoconsole"), 1);
}


void deferred_startup(void)
{
	t_atom a[2];
	
	atom_setlong(a+0, g_port_send);
	atom_setlong(a+1, g_port_listen);
	
	// cannot load classes from disk at the time that the extensions folder is processed
	ps_testport->s_thing = (t_object*)object_new_typed(_sym_nobox, ps_testport, 2, a);
	
	// notify anyone who is listening (e.g. a ruby script that launched max) that we are ready
	object_method(ps_testport->s_thing, _sym_send, gensym("/testport/ready"), 0, NULL);
}


// Load an external for internal use
t_max_err loadextern(t_symbol *objectname, long argc, t_atom *argv, t_object **object)
{
	t_class 	*c = NULL;
	t_object	*p = NULL;
	
	c = class_findbyname(_sym_box, objectname);
	if (!c) {
		p = (t_object*)newinstance(objectname, 0, NULL);
		if(p){
			c = class_findbyname(_sym_box, objectname);
			freeobject(p);
			p = NULL;
		}
		else{
			error("could not load extern (%s) within the oscar extension", objectname->s_name);
			return MAX_ERR_GENERIC;
		}
	}
	
	if (*object != NULL) {			// if there was an object set previously, free it first...
		object_free(*object);
		*object = NULL;
	}
	
	*object = (t_object*)object_new_typed(_sym_box, objectname, argc, argv);
	return MAX_ERR_NONE;
}


void autocolorbox(t_object *x)
{
	double		color[4] = {0.7, 0.4, 0.3, 1.0};
	t_object	*box = NULL;
	
	object_obex_lookup(x, _sym_pound_B, &box);
	object_attr_setdouble_array(box, _sym_color, 4, color);
}

