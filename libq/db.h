#ifndef _LIBQ_DB_H
#define _LIBQ_DB_H

/**
 * @file libq/db.h
 * @author Per Löwgren
 * @date Modified: 2017-10-26
 * @date Created: 2014-06-11
 */

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>


enum {
	ERR_DB_OPEN				= (__LIBQ_DB__<<16),
	ERR_DB_NOT_OPENED,
	ERR_DB_QUERY,
	ERR_DB_EMPTY_RESULT,
};

enum {
	DB_MYSQL,
	DB_POSTGRE,
	DB_SQLITE3,
};

typedef struct _QDb _QDb;
typedef _QDb *QDb;
typedef struct _QDbResult _QDbResult;
typedef _QDbResult *QDbResult;


QDb q_db_open(int type,const char *host,const char *usr,const char *pwd,const char *dbnm);
void q_db_close(QDb db);

uint64_t q_db_escape_string(QDb db,char *to,const char *from,size_t len);

int q_db_exec(QDb db,const char *sql);
QDbResult q_db_query(QDb db,const char *sql,int store);
void q_db_free_result(QDbResult res);
const char ** const q_db_fetch_row(QDbResult res);

uint64_t q_db_affected_rows(QDb db);
uint64_t q_db_insert_id(QDb db);

const char *q_db_error(QDb db);


#ifdef __cplusplus
}
#endif

#endif /* _LIBQ_DB_H */

