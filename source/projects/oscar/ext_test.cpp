/* 
	ext_test.h
	Copyright 2009 - Cycling '74
	Timothy Place, tim@cycling74.com	
*/

#include "ext_test.h"
#include "ext_strings.h"
#include "jpatcher_api.h"


void test_assert(t_test *t, const char *name, t_bool passed, t_symbol **tags, long tag_count)
{
	object_method((t_object*)t, gensym("assert"), name, passed, tags, tag_count);
}


void test_log(t_test *t, const char *text, ...)
{
	va_list	ap;
	char	str[MAX_PATH_CHARS];	// MAX_PATH_CHARS just choosen for a nice comfortable size
	size_t	len;
	
	va_start(ap, text);
	len = vsnprintf(str, MAX_PATH_CHARS, text, ap);
	str[MAX_PATH_CHARS-1] = 0;
	object_method((t_object*)t, gensym("log"), str);
	va_end(ap);
}
