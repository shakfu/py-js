//
// 	unit test extension for max
//	tim place
//	cycling '74
//

#include "oscar.h"
#include "jpatcher_api.h"
t_object *jwind_gettopwindow(void);


// class variables
static t_class		*s_testrunner_class = NULL;
static t_symbol     *ps_test_terminate = NULL;


/************************************************************************/

void testrunner_classinit(void)
{
	t_class *c = class_new("test.runner", (method)testrunner_new, (method)testrunner_free, sizeof(t_testrunner), (method)NULL, A_GIMME, 0L);
	
	class_addmethod(c, (method)testrunner_integration,	"integration",	0);			// integration tests
	class_addmethod(c, (method)testrunner_notify,		"notify",		A_CANT, 0);	// notifications from integration tests
		
	CLASS_ATTR_SYM_VARSIZE(c,	"tests",			0,	t_testrunner,	r_testnames,	r_numtestnames,	256);
	
	class_register(_sym_nobox, c);
	s_testrunner_class = c;
    
    ps_test_terminate = gensym("test.terminate");
}


void* testrunner_new(t_symbol *name, long argc, t_atom *argv)
{
	t_testrunner	*r = (t_testrunner*)object_alloc(s_testrunner_class);
	
	if (r) {
		r->r_qelem = (t_qelem*)qelem_new(r, (method)testrunner_one_integration);
		r->r_qelem_iter = (t_qelem*)qelem_new(r, (method)testrunner_dointegration);
		attr_args_process(r, (short)argc, argv);	
	}
	return r;	
}


void testrunner_free(t_testrunner *r)
{
	qelem_free(r->r_qelem);
	qelem_free(r->r_qelem_iter);
}


void testrunner_notify(t_testrunner *r, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
	if (sender == r->r_testunit && msg == gensym("terminate")) {
		r->r_terminated = true;
	}
}


void testrunner_one_integration(t_testrunner *r, t_symbol *testname)
{
	t_testdb	*db = (t_testdb*)((t_testmaster*)(ps_testmaster->s_thing))->m_db;
	t_symbol	*pound_t = gensym("#T");
	
	if (!r->r_running) { // start it
		t_testunit	*testunit = (t_testunit*)testunit_new(NULL, 0, NULL);
		
		r->r_testid = testdb_createcase(db, testname->s_name);
		testunit->o_db = db;
		testunit->o_id = r->r_testid;
		testdb_log(db, r->r_testid, "preparing to run %s", testname->s_name);

		pound_t->s_thing = (t_object*)testunit;
		object_method(_sym_max->s_thing, _sym_openfile, symbol_unique(), testname, 0);
		
		object_attach_byptr_register(r, testunit, _sym_nobox);
		r->r_testunit = (t_test*)testunit;
		r->r_running = true;
	}
	else { // try to finish it
		if (r->r_terminated) {
			t_max_err	err = MAX_ERR_NONE;
			
			// moved to here from the block above because when loading a maxzip the patcher instantiation maybe deferred/asynchronous
			pound_t->s_thing = NULL;

			testdb_log(db, r->r_testid, "concluding test");
			
			err = object_free(r->r_testunit);
			
			testdb_log(db, r->r_testid, "test patcher freed with status %i", err);
			testdb_closecase(db, r->r_testid);	

			r->r_running = false;
			return;
		}
	}
	qelem_set(r->r_qelem);
}


static int s_current_test_index;

void testrunner_integration(t_testrunner *r)
{
	int i;
	
	for (i=0; i < r->r_numtestnames ;i++)
		testrunner_one_integration(r, r->r_testnames[i]);
}


void testrunner_dointegration(t_testrunner *r)
{
	if (r->r_running)
		; // need to wait until the current test is done before starting the next one...
	else {
		if (s_current_test_index >= r->r_numtestnames)
			return;
			
		testrunner_one_integration(r, r->r_testnames[s_current_test_index]);
		s_current_test_index++;
	}
	qelem_set(r->r_qelem_iter);
}


