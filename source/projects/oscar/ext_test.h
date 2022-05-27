/* 
	ext_test.h
	Copyright 2009 - Cycling '74
	Timothy Place, tim@cycling74.com	
*/

#pragma once

#include "ext.h"
#include "ext_obex.h"

#ifdef WIN_VERSION
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif


/**	A test object.
	This object is passed to test methods to provide a means by which the test can communicate with the caller of the test.
	@ingroup	testing		*/
typedef t_object t_test;


/**	Possible test method return values.
 @ingroup	testing		*/
typedef enum _testresultvalue {
	kTestResult_Passed = 0,				///< Passed
	kTestResult_FailedGeneric,			///< Failed
	kTestResult_FailedNoSuchObject,		///< Failed, specifically because the object could not be found.
	kTestResult_FailedNoSuchTest		///< Failed, specifically because no such test could be found.
} t_testvalue;


BEGIN_USING_C_LINKAGE


/**	Assert that the result of an operation meets expectations.
	@ingroup	testing
	@param		t			The testrunner object calling our test method.
	@param		name			The name of the assertion.
	@param		passed		Pass true if the assertion passes, or false if it fails.
	@param		tags			Any user-defined tags that could be used to help search the results database.
	@param		tag_count	Number of tags.
 */
void test_assert(t_test *t, const char *name, t_bool passed, t_symbol **tags, long tag_count);


/**	Log some text in the search results.
	@ingroup	testing
	@param		t			The testrunner object calling our test method.
	@param		text		Text to be added to the test log.								*/
void test_log(t_test *t, const char *text, ...);


END_USING_C_LINKAGE
