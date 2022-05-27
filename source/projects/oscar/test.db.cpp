//
// 	unit test extension for max
//	tim place
//	cycling '74
//

#include "oscar.h"


// class variables
static t_class		*s_testdb_class = NULL;
char				g_dbpath[MAX_PATH_CHARS];


/************************************************************************/

void testdb_classinit(void)
{
	t_class *c = class_new("test.db", (method)testdb_new, (method)testdb_free, sizeof(t_testdb), (method)NULL, A_GIMME, 0L);
	class_register(_sym_nobox, c);
	s_testdb_class = c;
}


void* testdb_new(t_symbol *name, long argc, t_atom *argv)
{
	t_testdb	*d = (t_testdb*)object_alloc(s_testdb_class);
	
	if (d) {
		attr_args_process(d, (short)argc, argv);
		testdb_setup(d);
	}
	return d;
}


void testdb_free(t_testdb *d)
{
	db_close(&d->d_db);
}


void testdb_setup(t_testdb *d)
{
	if (!d->d_db) {
		short			path = packages_getpackagepath("max-test");
		char			fullpath[MAX_PATH_CHARS];
		short			apppath = path_getapppath();
		char			appfullpath[MAX_PATH_CHARS];
		unsigned long	appfullpathlen;
		int				i;
		char			dbfilename[MAX_PATH_CHARS];
		t_db_result		*dbresult = NULL;
		
		path_topathname(apppath, "", appfullpath);
		appfullpathlen = strlen(appfullpath);
		for (i=0; i<appfullpathlen; i++) {
			if (appfullpath[i] == ':' || appfullpath[i] == '/')
				appfullpath[i] ='-';
		}
		strncpy_zero(dbfilename, appfullpath, MAX_PATH_CHARS);
		strncat_zero(dbfilename, "--maxtestresults.db3", MAX_PATH_CHARS);
		
		path_topathname(path, "", fullpath);
		strncat_zero(fullpath, "/", MAX_PATH_CHARS);
		strncat_zero(fullpath, dbfilename, MAX_PATH_CHARS);
		db_open(gensym("unittestdb"), fullpath, &d->d_db);
		
		// cache the fullpath so it can be requested from the outside world
		{
			short apath;
			char afilename[MAX_FILENAME_CHARS];
			path_frompathname(fullpath, &apath, afilename);
			path_toabsolutesystempath(apath, afilename, g_dbpath);
		}

		db_query(d->d_db, &dbresult, "SELECT name FROM sqlite_master WHERE type='table' AND name='tests'");
		if (!db_result_numrecords(dbresult)) {
			db_query_table_new(d->d_db, "tests");
			db_query_table_addcolumn(d->d_db, "tests", "test_name",		"VARCHAR(512)", 0);
			db_query_table_addcolumn(d->d_db, "tests", "test_start",	"DATETIME", 0);
			db_query_table_addcolumn(d->d_db, "tests", "test_finish",	"DATETIME", 0);

			db_query_table_new(d->d_db, "assertions");
			db_query_table_addcolumn(d->d_db, "assertions", "test_id_ext",		"INTEGER", 0);
			db_query_table_addcolumn(d->d_db, "assertions", "assertion_name",	"VARCHAR(512)", 0);
			db_query_table_addcolumn(d->d_db, "assertions", "assertion_value",	"VARCHAR(512)", 0);
			db_query_table_addcolumn(d->d_db, "assertions", "assertion_data",	"VARCHAR(512)", 0);
			db_query_table_addcolumn(d->d_db, "assertions", "assertion_start",	"DATETIME", 0);
			db_query_table_addcolumn(d->d_db, "assertions", "assertion_finish",	"DATETIME", 0);
			db_query_table_addcolumn(d->d_db, "assertions", "assertion_tags",	"VARCHAR(512)", 0);

			db_query_table_new(d->d_db, "logs");
			db_query_table_addcolumn(d->d_db, "logs", "test_id_ext",	"INTEGER", 0);
			db_query_table_addcolumn(d->d_db, "logs", "text",			"VARCHAR(512)", 0);
			db_query_table_addcolumn(d->d_db, "logs", "timestamp",		"DATETIME", 0);
		}
		object_free(dbresult);
	}
}


long testdb_createcase(t_testdb *d, const char* test_name)
{
	long			test_id = 0;
	t_ptr_uint		timestamp = systime_seconds();
	t_datetime		datetime;
	
	systime_secondstodate(timestamp, &datetime);
	db_query(d->d_db, NULL, "INSERT INTO tests ( test_name , test_start, test_finish ) \
										VALUES ( \"%s\" ,		 '%4u-%02u-%02u %02u:%02u:%02u',		 0 ) ",
							test_name, 
							datetime.year, datetime.month, datetime.day, datetime.hour, datetime.minute, datetime.second);
	db_query_getlastinsertid(d->d_db, &test_id);
	return test_id;
}


void testdb_closecase(t_testdb *d, long test_id)
{
	t_ptr_uint		timestamp = systime_seconds();
	t_datetime		datetime;
	
	systime_secondstodate(timestamp, &datetime);
	db_query(d->d_db, NULL, "UPDATE tests SET test_finish = '%4u-%02u-%02u %02u:%02u:%02u' \
			 WHERE test_id = %i", 
			 datetime.year, datetime.month, datetime.day, datetime.hour, datetime.minute, datetime.second,
			 test_id);
}


void testdb_log(t_testdb *d, long test_id, const char* text, ...)
{
	t_ptr_uint		timestamp = systime_seconds();
	t_datetime		datetime;
	va_list			ap;
	char			expandedtext[2048];

	va_start(ap, text);
	vsnprintf(expandedtext, 2048, text, ap);

	{
		t_atom a;
		atom_setsym(&a, gensym(expandedtext));
		testport_send((t_testport*)ps_testport->s_thing, gensym("/db/log"), 1, &a);
	}

	systime_secondstodate(timestamp, &datetime);
	db_query(d->d_db, NULL, "INSERT INTO logs ( test_id_ext , text , timestamp ) \
									VALUES   ( %i         , \"%s\"   , '%4u-%02u-%02u %02u:%02u:%02u' )", 
									test_id, expandedtext, 
									datetime.year, datetime.month, datetime.day, datetime.hour, datetime.minute, datetime.second);
}

