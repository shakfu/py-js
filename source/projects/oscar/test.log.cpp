//
// 	unit/integration test extension for max
//	tim place
//	cycling '74
//

#include "oscar.h"


// class variables
static t_class		*s_testlog_class = NULL;


/************************************************************************/

void testlog_classinit(void)
{
	t_class *c = class_new("test.log",
				  (method)testlog_new,
				  (method)testlog_free,
				  sizeof(t_testlog),
				  (method)NULL,
				  A_GIMME,
				  0L);
		
	class_addmethod(c, (method)testlog_int,			"int",			A_LONG, 0);
	class_addmethod(c, (method)testlog_float,		"float",		A_FLOAT, 0);
	class_addmethod(c, (method)testlog_anything,	"list",			A_GIMME, 0);
	class_addmethod(c, (method)testlog_anything,	"anything",		A_GIMME, 0);
	class_addmethod(c, (method)testlog_assist,		"assist",		A_CANT, 0);
		
	class_register(_sym_box, c);
	s_testlog_class = c;
}


/************************************************************************/


void* testlog_new(t_symbol *s, long argc, t_atom *argv)
{
	t_testlog	*x = (t_testlog*)object_alloc(s_testlog_class);
	
	if (x) {
		x->a_test = (t_test*)gensym("#T")->s_thing;
		attr_args_process(x, (short)argc, argv);
	}		
	autocolorbox((t_object*)x);
	return x;
}


void testlog_free(t_testlog *x)
{
	;
}


#pragma mark -
/************************************************************************/

void testlog_assist(t_testlog *x, void *b, long m, long a, char *s)
{
	strcpy(s, "log messages to the test result, or to the max window");
}


void testlog_int(t_testlog *x, long v)
{
	t_atom a[1];

	if (!x->a_test) {
		object_post((t_object*)x, "%i", (int)v);
		return;
	}
		
	atom_setlong(a, v);
	testlog_anything(x, _sym_int, 1, a);
}


void testlog_float(t_testlog *x, double v)
{
	t_atom a;

	if (!x->a_test) {
		object_post((t_object*)x, "%f", v);
		return;
	}
	
	atom_setfloat(&a, v);
	testlog_anything(x, _sym_float, 1, &a);
}


void testlog_anything(t_testlog *x, t_symbol *s, long argc, t_atom *argv)
{
	char *text = NULL;
	long textsize = 0;
	
	atom_gettext(argc, argv, &textsize, &text, OBEX_UTIL_ATOM_GETTEXT_DEFAULT | OBEX_UTIL_ATOM_GETTEXT_NUM_HI_RES);
	if (!text && textsize) {
		object_error((t_object*)x, "no text to log");
		return;
	}
	
	if (x->a_test) {
		if (s == _sym_int || s == _sym_float || s == _sym_list)
			test_log(x->a_test, text);
		else
			test_log(x->a_test, "%s %s", s->s_name, text);
	} 
	else {
		if (s == _sym_int || s == _sym_float || s == _sym_list)
			object_post((t_object*)x, text);
		else
			object_post((t_object*)x, "%s %s", s->s_name, text);
	}

	sysmem_freeptr(text);
}

