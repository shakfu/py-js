//
// 	unit test extension for max
//	tim place
//	cycling '74
//

#include "oscar.h"


// class variables
static t_class		*s_testmaster_class = NULL;
static t_object		*s_testmaster_instance = NULL;


/************************************************************************/
#pragma mark -
#pragma mark test.master

void testmaster_classinit(void)
{
	t_class *c = class_new("test.master", (method)testmaster_new, (method)testmaster_free, sizeof(t_testmaster), (method)NULL, A_GIMME, 0L);
	
	class_addmethod(c, (method)testmaster_run,	"run",	A_GIMME, 0);	// all tests
	
	class_register(_sym_nobox, c);
	s_testmaster_class = c;

	{
		char filename[MAX_FILENAME_CHARS];
		short path = 0;
		t_fourcc type = 0;
		t_dictionary *d = NULL;
		t_max_err err;
		
		strncpy_zero(filename, "max-test-config.json", MAX_FILENAME_CHARS);
		locatefile_extended(filename, &path, &type, NULL, 0);
		err = dictionary_read(filename, path, &d);
		if (!err) {
			dictionary_getdeflong(d, gensym("port-send"), &g_port_send, 0);
			dictionary_getdeflong(d, gensym("port-listen"), &g_port_listen, 0);
		}
		object_free(d);
	}
}

void testmaster_quittask(void)
{
	if (s_testmaster_instance) {
		object_free(s_testmaster_instance);
		s_testmaster_instance = NULL;
		ps_testmaster->s_thing = NULL;
	}
}


void testmaster_createdb(t_testmaster *m)
{
	if (!m->m_db)
		m->m_db = (t_object*)testdb_new(NULL, 0, NULL);
}


void* testmaster_new(t_symbol *name, long argc, t_atom *argv)
{
	t_testmaster	*m = (t_testmaster*)object_alloc(s_testmaster_class);
	
	if (m) {
		m->m_integration_qelem = (t_qelem*)qelem_new(m, (method)testmaster_integration_recurse);
		attr_args_process(m, (short)argc, argv);
		defer_low(m, (method)testmaster_createdb, NULL, 0, NULL); // have to defer because the sqlite extension is not yet loaded
	}
	return m;
}

void testmaster_free(t_testmaster *m)
{
	qelem_free(m->m_integration_qelem);
	object_free(m->m_testrunner); // free any old testrunner instance
	object_free(m->m_db);
}


void testmaster_integration_recurse(t_testmaster *m)
{
	// if a test is already going, just hang out until it's done...
	if (m->m_testrunner && ((t_testrunner*)m->m_testrunner)->r_running) {
		qelem_set(m->m_integration_qelem);
		return;
	}
	if (!m->m_first_iter)
		testport_send((t_testport*)ps_testport->s_thing, gensym("/test/complete"), 0, NULL);
	if (m->m_integration_name_count == 0) {
		post("integration tests complete");
		testport_send((t_testport*)ps_testport->s_thing, gensym("/test/all/complete"), 0, NULL);
		return;
	}

	m->m_first_iter = false;
	m->m_integration_name_count--;
	
	{
		t_symbol	*testname = m->m_integration_names[m->m_integration_name_count];
		char		testnamestr[MAX_PATH_CHARS];
		char		testpath[MAX_PATH_CHARS];
		short		path = 0;
		t_fourcc	type = 0;
		t_fourcc	types[2] = {'JSON', 'mZip'};
		int			typecount = 2;
		t_atom		a[2];
		t_max_err	err = MAX_ERR_NONE;
		
		object_free(m->m_testrunner); // free any old testrunner instance
		
		strncpy_zero(testnamestr, testname->s_name, MAX_PATH_CHARS);
		err = locatefile_extended(testnamestr, &path, &type, types, typecount);
		if (err) {
			error("no test found with the name '%s'", testnamestr);
			goto out;
		}
		
		path_topathname(path, testnamestr, testpath);
		
		atom_setsym(a+0, gensym("@tests"));
		atom_setsym(a+1, gensym(testpath));
		m->m_testrunner = (t_object*)object_new_typed(_sym_nobox, gensym("test.runner"), 2, a);
		object_method(m->m_testrunner, gensym("integration"));
		
		// NOTE: we can't return a result directly from the 'integration' call because it is asynchronous
	}
out:
	qelem_set(m->m_integration_qelem);
}


void testmaster_integration(t_testmaster *m, t_symbol *s, long argc, t_atom *argv)
{
	long		i;
	long		test_count = 0;
	t_symbol	**test_names = (t_symbol**)sysmem_newptrclear(sizeof(t_symbol*) * argc);
	
	testmaster_createdb(m);
	
	// If there are args, they specify which test patchers to run
	if (argc) {
		test_names = (t_symbol**)sysmem_newptrclear(sizeof(t_symbol*) * argc);
		
		for (i=0; i<argc; i++) {
			test_names[i] = atom_getsym(argv+i);
			test_count++;
		}
	}
	else { // No args, so that means we want to run all patchers/projects ending in .maxtest.maxpat/.maxtest.maxzip in the searchpath
		short version = maxversion();

		if ( (((version >> 8) & 0xF) > 6) && (((version >> 8) & 0xF) < 15) ) {
			t_database		*db = NULL;
			t_db_result		*result = NULL;
			
			db_open(gensym("__maxdb__"), NULL, &db);
			db_query(db, &result, "SELECT DISTINCT _name FROM _things WHERE (_kind = 'patcher' OR _kind = 'project') AND _name LIKE '%%.maxtest' AND _status = ''");
			test_count = db_result_numrecords(result);
			
			test_names = (t_symbol**)sysmem_newptrclear(sizeof(t_symbol*) * test_count);
			for (i=0; i<test_count; i++) {
				// TODO: test names actually need to be converted complete paths here!
				test_names[i] = gensym(db_result_string(result, i, 0));
				
			}
			object_free(result);
			db_close(&db);
		}
		else { // max 5 or 6
			t_database		*db = NULL;
			t_db_result		*result = NULL;
			
			db_open(gensym("__max5db__"), NULL, &db);
			db_query(db, &result,
					 "SELECT DISTINCT name FROM objects \
					 JOIN files ON objects.file_id_ext = files.file_id \
					 JOIN roles ON objects.role_id_ext = roles.role_id \
					 WHERE \
					 ((( role_id_ext IN (SELECT role_id FROM roles WHERE rolename = 'patcher' OR rolename = 'project') AND (validity > 0) OR flags & 2))) \
					 AND \
					 name LIKE '%%.maxtest'");
			test_count = db_result_numrecords(result);

			test_names = (t_symbol**)sysmem_newptrclear(sizeof(t_symbol*) * test_count);
			for (i=0; i<test_count; i++) {
				// TODO: test names actually need to be converted complete paths here!
				test_names[i] = gensym(db_result_string(result, i, 0));

			}
			object_free(result);
			db_close(&db);
		}
	}
	
	// Now process the object list recursively
	m->m_integration_name_count = test_count;
	m->m_integration_names = test_names;
	m->m_first_iter = true;
	qelem_set(m->m_integration_qelem);
}


void testmaster_run(t_testmaster *m, t_symbol *s, long argc, t_atom *argv)
{
	testmaster_integration(m, s, argc, argv);
}

