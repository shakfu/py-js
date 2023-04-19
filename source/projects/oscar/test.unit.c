//
// 	unit test extension for max
//	tim place
//	cycling '74
//

#include "oscar.h"


// class variables
static t_class		*s_testunit_class = NULL;


/************************************************************************/

void testunit_classinit(void)
{
	t_class *c = class_new("test.unit",
				  (method)testunit_new,
				  (method)testunit_free,
				  sizeof(t_testunit),
				  (method)NULL,
				  A_GIMME,
				  0L);
		
	class_addmethod(c, (method)testunit_log,		"log",			A_CANT, 0);
	class_addmethod(c, (method)testunit_assert,		"assert",		A_CANT, 0);
	class_addmethod(c, (method)testunit_terminate,	"terminate",	A_CANT, 0);
		
	class_register(_sym_nobox, c);
	s_testunit_class = c;
}


/************************************************************************/

void* testunit_new(t_symbol *s, long argc, t_atom *argv)
{
	t_testunit	*u = (t_testunit*)object_alloc(s_testunit_class);
	
	if (u) {
		attr_args_process(u, (short)argc, argv);	
	}
	return u;
}


void testunit_free(t_testunit *u)
{
	;
}


void testunit_log(t_testunit *u, const char* text)
{
	testdb_log(u->o_db, u->o_id, text);
}


void testunit_assert(t_testunit *u, const char* assertion_name, t_bool passed, t_symbol **tags, long tag_count)
{
	t_ptr_uint		timestamp = systime_seconds();
	t_datetime		datetime;
	char			ctags[4096];
	long			i;
	
	ctags[0] = 0;
	for (i=0; i<tag_count; i++) {
		if (i == 0)
			strncpy_zero(ctags, tags[i]->s_name, 4096);
		else {
			strncat_zero(ctags, " ", 4096);
			strncat_zero(ctags, tags[i]->s_name, 4096);
		}
	}
	
	systime_secondstodate(timestamp, &datetime);

	db_query(u->o_db->d_db, NULL, "INSERT INTO assertions ( test_id_ext , assertion_name , assertion_finish, assertion_value, assertion_tags ) \
			 VALUES   ( %i         , \"%s\"   , '%4u-%02u-%02u %02u:%02u:%02u', \'%s\', \"%s\" )",
			 u->o_id, assertion_name, 
			 datetime.year, datetime.month, datetime.day, datetime.hour, datetime.minute, datetime.second,
			 (passed?"Pass":"Fail"),
			 ctags
			 );
}


void testunit_terminate(t_testunit *u)
{
	object_notify(u, gensym("terminate"), NULL);
}

