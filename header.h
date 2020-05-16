#ifndef _EXT_DATABASE_H_
#define _EXT_DATABASE_H_
#if !defined( DEBUG ) && !defined ( NDEBUG ) && !defined (_MAXCOMMON_H_)
#include "ext.h"
#include "ext_obex.h"
#endif
#ifdef WIN_VERSION
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif
typedef t_object t_database
typedef t_object t_db_result
typedef t_object t_db_view
BEGIN_USING_C_LINKAGE
t_max_err db_open(t_symbol *dbname, const char *fullpath, t_database **db)
typedef enum _db_open_flags {
	DB_OPEN_FLAGS_NONE = 0,
	DB_OPEN_FLAGS_READONLY = 0x01
} t_db_open_flags
t_max_err db_open_ext(t_symbol *dbname, const char *fullpath, t_database **db, long flags)
t_max_err db_close(t_database **db)
t_max_err db_query(t_database *db, t_db_result **dbresult, const char *sql, ...)
t_max_err db_query_direct(t_database *db, t_db_result **dbresult, const char *sql)
t_max_err db_query_silent(t_database *db, t_db_result **dbresult, const char *sql, ...)
t_max_err db_query_getlastinsertid(t_database *db, long *id)
t_max_err db_query_table_new(t_database *db, const char *tablename)
t_max_err db_query_table_addcolumn(t_database *db, const char *tablename, const char *columnname, const char *columntype, const char *flags)
t_max_err db_transaction_start(t_database *db)
t_max_err db_transaction_end(t_database *db)
t_max_err db_transaction_flush(t_database *db)
t_max_err db_view_create(t_database *db, const char *sql, t_db_view **dbview)
t_max_err db_view_remove(t_database *db, t_db_view **dbview)
t_max_err db_view_getresult(t_db_view *dbview, t_db_result **result)
t_max_err db_view_setquery(t_db_view *dbview, char *newquery)
char** db_result_nextrecord(t_db_result *result)
void db_result_reset(t_db_result *result)
void db_result_clear(t_db_result *result)
long db_result_numrecords(t_db_result *result)
long db_result_numfields(t_db_result *result)
char* db_result_fieldname(t_db_result *result, long fieldindex)
char* db_result_string(t_db_result *result, long recordindex, long fieldindex)
long db_result_long(t_db_result *result, long recordindex, long fieldindex)
float db_result_float(t_db_result *result, long recordindex, long fieldindex)
t_ptr_uint db_result_datetimeinseconds(t_db_result *result, long recordindex, long fieldindex)
void db_util_stringtodate(const char *string, t_ptr_uint *date)
void db_util_datetostring(const t_ptr_uint date, char *string)
END_USING_C_LINKAGE
#endif 
