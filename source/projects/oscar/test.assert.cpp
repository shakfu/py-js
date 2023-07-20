//
// 	unit/integration test extension for max
//	tim place
//	cycling '74
//

#include "oscar.h"


// class variables
static t_class		*s_testassert_class = NULL;
t_linklist			*g_all_assert_instances = NULL;


/************************************************************************/

void testassert_classinit(void)
{
	t_class *c = class_new("test.assert",
				  (method)testassert_new,
				  (method)testassert_free,
				  sizeof(t_testassert),
				  (method)NULL,
				  A_GIMME,
				  0L);
		
	class_addmethod(c, (method)testassert_int,			"int",			A_LONG, 0);
	class_addmethod(c, (method)testassert_float,		"float",		A_FLOAT, 0);
	class_addmethod(c, (method)testassert_anything,		"list",			A_GIMME, 0);
	class_addmethod(c, (method)testassert_anything,		"anything",		A_GIMME, 0);
	class_addmethod(c, (method)testassert_pop,			"pop",			A_CANT, 0);
	class_addmethod(c, (method)testassert_loadbang,		"loadbang",		A_CANT, 0);
	class_addmethod(c, (method)testassert_assist,		"assist",		A_CANT, 0);
	
	CLASS_ATTR_SYM_VARSIZE(c, "tags", 0, t_testassert, a_tags, a_tagcount, MAX_TAG_COUNT);
	
	class_register(_sym_box, c);
	s_testassert_class = c;
}


/************************************************************************/


t_object *gettoplevelpatcher(t_object *patcher)
{
	t_object *toplevelpatcher = patcher;

	while ((patcher = object_attr_getobj(patcher, _sym_parentpatcher)))
		toplevelpatcher = patcher;
	
	return toplevelpatcher;
}



void* testassert_new(t_symbol *s, long argc, t_atom *argv)
{
	t_testassert	*x = (t_testassert*)object_alloc(s_testassert_class);
	long			attrstart = attr_args_offset((short)argc, argv);
	
	if (x) {
		x->a_outlet = outlet_new(x, NULL);
		x->a_test = (t_test*)gensym("#T")->s_thing;
		if (attrstart)
			x->a_name = atom_getsym(argv);
		else
			x->a_name = symbol_unique();
		attr_args_process(x, (short)argc, argv);
	}
	
	if (!g_all_assert_instances)
		g_all_assert_instances = linklist_new();
	linklist_append(g_all_assert_instances, x);
	
	autocolorbox((t_object*)x);
	return x;
}


void testassert_free(t_testassert *x)
{
	linklist_chuckobject(g_all_assert_instances, x);
}


#pragma mark -
/************************************************************************/

void testassert_assist(t_testassert *x, void *b, long m, long a, char *s)
{
	if (m==ASSIST_INLET) {
		switch (a) {
			case 0: snprintf_zero(s, ASSIST_MAX_STRING_LEN, "receive values from the system under test to compare against expectations"); break;
		}
	} 
	else {
		switch (a) {
			case 0: snprintf_zero(s, ASSIST_MAX_STRING_LEN, "sends input values to the system under test at loadbang time"); break;
		}
	}
}


void testassert_loadbang(t_testassert *x)
{
	if (x->a_inputcount) {
		if (atom_gettype(x->a_input) == A_LONG || atom_gettype(x->a_input) == A_FLOAT) {
			if (x->a_inputcount > 1)
				outlet_anything(x->a_outlet, _sym_list, (short)x->a_inputcount, x->a_input);
			else if(atom_gettype(x->a_input) == A_LONG)
				outlet_int(x->a_outlet, atom_getlong(x->a_input));
			else
				outlet_float(x->a_outlet, atom_getfloat(x->a_input));
		}
		else
			outlet_anything(x->a_outlet, atom_getsym(x->a_input), (short)x->a_inputcount-1, x->a_input+1);
	}
}


void testassert_int(t_testassert *x, long v)
{
	if (!x->a_test)
		return;
	
	if (x->a_inputcount == 0) {	// by default we just look for 0 (fail) or 1 (pass)
		if (v == 1) {
			x->a_status = TEST_ASSERT_PASS;
			x->a_passed = true;
		}
		else {
			x->a_status = TEST_ASSERT_FAIL;
			x->a_passed = false;
		}
	}
	else {
		t_atom a[1];
		
		atom_setlong(a, v);
		testassert_anything(x, _sym_int, 1, a);
	}
}


void testassert_float(t_testassert *x, double v)
{
	t_atom a[1];
	
	atom_setfloat(a, v);
	testassert_anything(x, _sym_float, 1, a);
}


void testassert_anything(t_testassert *x, t_symbol *s, long argc, t_atom *argv)
{
	if (!x->a_test)
		return;

	// 1. compare input to expected output
	
	// 2. save the result
	x->a_status = TEST_ASSERT_FAIL;
	test_assert(x->a_test, x->a_name->s_name, false, (t_symbol**)x->a_tags, x->a_tagcount);
}


// test is being closed down and the assertions are being popped, so log the result before we disappear...

void testassert_pop(t_testassert *x)
{
	if (!x->a_test)
		return;

	if (x->a_status == TEST_ASSERT_NOT_EXECUTED) {
		test_log(x->a_test, "assertion '%s' never returned results!", x->a_name->s_name);
	}
	test_assert(x->a_test, x->a_name->s_name, x->a_passed, (t_symbol**)x->a_tags, x->a_tagcount);
}
