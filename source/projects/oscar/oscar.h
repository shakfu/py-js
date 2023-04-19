//
// 	The oscar extension for max
//
//	Oscar is the first name of the man commonly known as The Wizard of Oz
//		(we are pulling the strings of Max to automate its operation)
//	Oscar is also the name of the classic Grouch on Sesame Street
//		(because testing is an activity that can make us grumpy)
//	OSCar uses OSC to communicate
//	
//	Tim Place
//	Cycling '74
//

#include "ext.h"
#include "ext_obex.h"
#include "ext_common.h"
#include "ext_strings.h"
#include "ext_critical.h"
#include "ext_database.h"
#include "ext_packages.h"
#include "ext_test.h"

BEGIN_USING_C_LINKAGE
extern t_symbol *ps_testmaster;
extern t_symbol *ps_testport;
extern t_atom_long g_port_send;
extern t_atom_long g_port_listen;
extern char g_dbpath[MAX_PATH_CHARS];

void deferred_startup(void);
t_max_err loadextern(t_symbol *objectname, long argc, t_atom *argv, t_object **object);
void autocolorbox(t_object *x);
t_object *gettoplevelpatcher(t_object *patcher);

END_USING_C_LINKAGE


/************************************************************************/
#if 0
#pragma mark -
#pragma mark test.master
#endif 

/** Class that manages the whole shebang and provides a global entry point	*/
typedef struct _testmaster {
	t_object	m_ob;					///< header
	t_object	*m_db;					///< test.db object
	t_object	*m_testrunner;			///< test.runner instance
	
	// unit test members:
	long		m_object_name_count;	///< number of objects under test
	t_symbol	**m_object_names;		///< names of the objects under test
	long		m_test_count;			///< number of named tests to execute
	t_symbol	**m_test_names;			///< names of the tests
	
	// integration test members:
	long		m_integration_name_count;	///< number of test patchers
	t_symbol	**m_integration_names;		///< names of the test patchers
	t_qelem		*m_integration_qelem;		///< wait for one test to finish before starting the next
	long		m_first_iter;				///< is this the first iteration through the qelem?
	
} t_testmaster;


BEGIN_USING_C_LINKAGE

void	testmaster_classinit(void);
void	testmaster_quittask(void);
void*	testmaster_new(t_symbol *s, long argc, t_atom *argv);
void	testmaster_free(t_testmaster *m);
void	testmaster_integration_recurse(t_testmaster *m);
void	testmaster_integration(t_testmaster *m, t_symbol *s, long argc, t_atom *argv);
void	testmaster_run(t_testmaster *m, t_symbol *s, long argc, t_atom *argv);

END_USING_C_LINKAGE


/************************************************************************/
#if 0
#pragma mark -
#pragma mark test.db
#endif 

/**	Class that wraps a database for logging test results and statistics	*/
typedef struct _testdb {
	t_object	d_ob;				///< header
	t_database	*d_db;				///< test results are written to this database for persistence	
} t_testdb;


BEGIN_USING_C_LINKAGE

void	testdb_classinit(void);
void*	testdb_new(t_symbol *name, long argc, t_atom *argv);
void	testdb_free(t_testdb *d);
void	testdb_setup(t_testdb *d);
long	testdb_createcase(t_testdb *d, const char* test_name);
void	testdb_closecase(t_testdb *d, long test_id);
void	testdb_log(t_testdb *d, long test_id, const char* text, ...);

END_USING_C_LINKAGE


/************************************************************************/
#if 0
#pragma mark -
#pragma mark test.runner
#endif 

/** Class that actually executes any one given test.		*/
typedef struct _testrunner {
	t_object	r_ob;				///< header
	t_symbol	*r_testnames[256];	///< names of the tests to run
	long		r_numtestnames;		///< number of tests to run
	long		r_testid;			///< id of the test in the database
	long		r_running;			///< currently running a test (used for waiting on integration tests)
	t_qelem		*r_qelem;			///< used for waiting on integration test completion
	t_qelem		*r_qelem_iter;		///< used for iterating through integration tests
	long		r_terminated;		///< integration test has terminated
	t_test		*r_testunit;		///< the test currently running
} t_testrunner;


BEGIN_USING_C_LINKAGE

void		testrunner_classinit(void);
void*		testrunner_new(t_symbol *s, long argc, t_atom *argv);
void		testrunner_free(t_testrunner *r);
void		testrunner_notify(t_testrunner *r, t_symbol *s, t_symbol *msg, void *sender, void *data);
void		testrunner_one_integration(t_testrunner *r, t_symbol *testname);
void		testrunner_integration(t_testrunner *r);
void		testrunner_dointegration(t_testrunner *r);

END_USING_C_LINKAGE


/************************************************************************/
#if 0
#pragma mark -
#pragma mark test.port
#endif 

/** Class providing a udp portal for remote communication with Max.	*/
typedef struct _testport {
	t_object	u_ob;				///< header
	t_object	*u_udpreceive;		///< udpreceive instance
	t_object	*u_udpsend;			///< udpsend instance
} t_testport;


BEGIN_USING_C_LINKAGE

void		testport_classinit(void);
void*		testport_new(t_symbol *s, long argc, t_atom *argv);
void 		testport_free(t_testport *u);
t_max_err	testport_notify(t_testport *u, t_symbol *s, t_symbol *msg, void *sender, void *data);
t_max_err	testport_send(t_testport *u, t_symbol *msg, long argc, t_atom *argv);
void		testport_ping(t_testport *u);

END_USING_C_LINKAGE


/************************************************************************/
#if 0
#pragma mark -
#pragma mark test.unit
#endif 

/** Class that is passed to tests when they are run to provide hooks back into the system.	*/
typedef struct _testunit {
	t_object	o_ob;				///< header
	t_testdb	*o_db;				///< database interface for logging results
	long		o_id;				///< database id for this test
} t_testunit;


BEGIN_USING_C_LINKAGE

void	testunit_classinit(void);
void*	testunit_new(t_symbol *s, long argc, t_atom *argv);
void 	testunit_free(t_testunit *o);
void	testunit_log(t_testunit *u, const char* text);
void	testunit_assert(t_testunit *u, const char* assertion_name, t_bool passed, t_symbol **tags, long tag_count);
void	testunit_terminate(t_testunit *u);

END_USING_C_LINKAGE


/************************************************************************/
#if 0
#pragma mark -
#pragma mark test.assert
#endif 

enum {
	TEST_ASSERT_NOT_EXECUTED = 0,
	TEST_ASSERT_PASS,
	TEST_ASSERT_FAIL
};

#define MAX_TAG_COUNT 16

/** Class for instrumenting patchers that are used to execute tests 
 so that they can communicate results back to the testrunner.	*/
typedef struct _testassert {
	t_object	a_ob;					///< header
	void		*a_outlet;				///< outlet for providing input to the system under test
	t_symbol	*a_name;				///< name of the assertion
	t_atom		*a_input;				///< input to the system
	long		a_inputcount;			///< number of atoms in a_input
	t_atom		*a_output;				///< expected output to the system
	long		a_outputcount;			///< number of atoms in a_output
	long		a_status;				///< pass or fail status of the assertion
	t_test		*a_test;				///< test object that is calling this assertion
	t_bool		a_passed;				///< result
	t_symbol	*a_tags[MAX_TAG_COUNT];	///< any user-specified tags to be associated with the assertion for searching test results
	long		a_tagcount;				///< number of tags
} t_testassert;


BEGIN_USING_C_LINKAGE

void	testassert_classinit(void);
void*	testassert_new(t_symbol *s, long argc, t_atom *argv);
void 	testassert_free(t_testassert *x);
void	testassert_assist(t_testassert *x, void *b, long m, long a, char *s);
void 	testassert_loadbang(t_testassert *x);
void	testassert_int(t_testassert *x, long v);
void	testassert_float(t_testassert *x, double v);
void	testassert_anything(t_testassert *x, t_symbol *s, long argc, t_atom *argv);
void	testassert_pop(t_testassert *x);

END_USING_C_LINKAGE


/************************************************************************/
#if 0
#pragma mark -
#pragma mark test.equals
#endif

BEGIN_USING_C_LINKAGE
void	testequals_classinit(void);
END_USING_C_LINKAGE


/************************************************************************/
#if 0
#pragma mark -
#pragma mark test.log
#endif 


/** Class for instrumenting patchers that are used to execute tests 
 so that they can communicate results back to the testrunner.	*/
typedef struct _testlog {
	t_object	a_ob;		///< header
	t_test		*a_test;	///< test object
} t_testlog;


BEGIN_USING_C_LINKAGE

void	testlog_classinit(void);
void*	testlog_new(t_symbol *s, long argc, t_atom *argv);
void 	testlog_free(t_testlog *x);
void	testlog_assist(t_testlog *x, void *b, long m, long a, char *s);
void	testlog_int(t_testlog *x, long v);
void	testlog_float(t_testlog *x, double v);
void	testlog_anything(t_testlog *x, t_symbol *s, long argc, t_atom *argv);

END_USING_C_LINKAGE


/************************************************************************/
#if 0
#pragma mark -
#pragma mark test.terminate
#endif 

/** Class for instrumenting patchers that are used to execute tests 
 so that they can communicate results back to the testrunner.	*/
typedef struct _testterminate {
	t_object	x_ob;				///< header
	t_object	*x_patcher;			///< the patcher in which the object exists -- assumed to be a top-level patcher
	t_test		*x_test;			///< test object that is calling this assertion
} t_testterminate;


BEGIN_USING_C_LINKAGE

void	testterminate_classinit(void);
void*	testterminate_new(t_symbol *s, long argc, t_atom *argv);
void 	testterminate_free(t_testterminate *x);
void	testterminate_assist(t_testterminate *x, void *b, long m, long a, char *s);
void	testterminate_bang(t_testterminate *x);

END_USING_C_LINKAGE


/************************************************************************/
#if 0
#pragma mark -
#pragma mark test.sample~
#endif 

BEGIN_USING_C_LINKAGE

void	testsample_classinit(void);

END_USING_C_LINKAGE
