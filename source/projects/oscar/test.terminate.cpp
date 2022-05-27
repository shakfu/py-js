//
// 	unit/integration test extension for max
//	tim place
//	cycling '74
//

#include "oscar.h"


// class variables
static t_class		*s_testterminate_class = NULL;
extern t_linklist	*g_all_assert_instances;


/************************************************************************/

void testterminate_classinit(void)
{
	t_class *c = class_new("test.terminate",
				  (method)testterminate_new,
				  (method)testterminate_free,
				  sizeof(t_testterminate),
				  (method)NULL,
				  A_GIMME,
				  0L);
	
	class_addmethod(c, (method)testterminate_bang,		"bang",			0);
	class_addmethod(c, (method)testterminate_assist,	"assist",		A_CANT, 0);
		
	class_register(_sym_box, c);
	s_testterminate_class = c;
}


/************************************************************************/

void* testterminate_new(t_symbol *s, long argc, t_atom *argv)
{
	t_testterminate *x = (t_testterminate*)object_alloc(s_testterminate_class);
	
	if (x) {
		object_obex_lookup(x, _sym_pound_P, &x->x_patcher);
		x->x_test = (t_test*)gensym("#T")->s_thing;
		attr_args_process(x, (short)argc, argv);
	}
	autocolorbox((t_object*)x);
	return x;
}


void testterminate_free(t_testterminate *x)
{
	// Notify the test harness that we are done so it can start another test if desired
	if (x->x_test && !NOGOOD(x->x_test))
		object_method(x->x_test, gensym("terminate"));
}


#pragma mark -
/************************************************************************/


void testterminate_qfn(t_testterminate *x)
{
	if (x->x_test) {
		// 1. Iterate through all the assert instances and  tell them to log their results
		linklist_methodall(g_all_assert_instances, gensym("pop"));
		
		// 2. Close the patcher
		object_method(x->x_patcher, _sym_wclose);
	}
}


void testterminate_dobang(t_testterminate *x)
{
	object_method(_sym_dsp->s_thing, _sym_stop);
	defer_low(x, (method)testterminate_qfn, NULL, 0, NULL);
}


void testterminate_bang(t_testterminate *x)
{
	defer_low(x, (method)testterminate_dobang, NULL, 0, NULL);
}


void testterminate_assist(t_testterminate *x, void *b, long m, long a, char *s)
{
	strcpy(s, "bang to signal the test to terminate");
}

