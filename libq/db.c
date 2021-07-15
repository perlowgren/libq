
#include <libq/config.h>

#include "libq.h"
#ifdef USE_DB

#include <stdlib.h>
#include <string.h>

#ifdef USE_MYSQL
#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#endif

#ifdef USE_POSTGRESQL
#include <postgresql/libpq-fe.h>
#endif

#ifdef USE_SQLITE3
#include <sqlite3.h>
#endif

#include "db.h"
#include "error.h"


Q_ERR(__FILE__)


struct _QDb {
	int type;
	const char *host;
	const char *usr;
	const char *pwd;
	const char *dbnm;
	union {
#ifdef USE_MYSQL
		MYSQL *mysql;
#endif
#ifdef USE_POSTGRESQL
		PGconn *postgre;
#endif
#ifdef USE_SQLITE3
		sqlite3 *sqlite3;
#endif
	} con;
};

struct _QDbResult {
	QDb db;
	union {
#ifdef USE_MYSQL
		MYSQL_RES *mysql;
#endif
#ifdef USE_POSTGRESQL
		PGresult *postgre;
#endif
#ifdef USE_SQLITE3
		sqlite3_stmt *sqlite3;
#endif
	} res;
	int ncols;
	int nrows;
	const char **row;
	int row_count;
};


static int db_check(QDb db) {
	if(db==NULL || (
#ifdef USE_MYSQL
		(db->type==DB_MYSQL && db->con.mysql==NULL)
#endif
#ifdef USE_POSTGRESQL
#ifdef USE_MYSQL
		||
#endif
		(db->type==DB_POSTGRE && db->con.postgre==NULL)
#endif
#ifdef USE_SQLITE3
#ifdef USE_MYSQL
		||
#else
#ifdef USE_POSTGRESQL
		||
#endif
#endif
		(db->type==DB_SQLITE3 && db->con.sqlite3==NULL)
#endif
	)) {
		q_err(ERR_DB_NOT_OPENED,"Database isn't opened, use q_db_open.");
		return 0;
	}
	return 1;
}

static const char *db_open(QDb db) {
	const char *err = NULL;
	if(db!=NULL) {
#ifdef USE_MYSQL
		if(db->type==DB_MYSQL) {
			my_bool reconnect = 1;
			db->con.mysql = mysql_init(NULL);
			mysql_options(db->con.mysql,MYSQL_OPT_RECONNECT,&reconnect);
			if(!mysql_real_connect(db->con.mysql,db->host,db->usr,db->pwd,db->dbnm,0,NULL,CLIENT_MULTI_STATEMENTS))
				err = mysql_error(db->con.mysql);
		}
#endif
#ifdef USE_POSTGRESQL
#ifdef USE_MYSQL
		else
#endif
		if(db->type==DB_POSTGRE) {
			char str[1025];
			sprintf(str,"dbname=%s host=%s user=%s password=%s",db->dbnm,db->host,db->usr,db->pwd);
			db->con.postgre = PQconnectdb(str);
			if(PQstatus(db->con.postgre)==CONNECTION_BAD)
				err = "PostgreSQL: Unable to connect to database.";
		}
#endif
#ifdef USE_SQLITE3
#ifdef USE_MYSQL
		else
#else
#ifdef USE_POSTGRESQL
		else
#endif
#endif
		if(db->type==DB_SQLITE3) {
			if(sqlite3_open(db->host,&db->con.sqlite3))
				err = sqlite3_errmsg(db->con.sqlite3);
		}
#endif
	}
	return err;
}

static void db_close(QDb db) {
	if(db_check(db)) {
#ifdef USE_MYSQL
		if(db->type==DB_MYSQL && db->con.mysql!=NULL) {
			mysql_close(db->con.mysql);
			db->con.mysql = NULL;
		}
#endif
#ifdef USE_POSTGRESQL
#ifdef USE_MYSQL
		else
#endif
		if(db->type==DB_POSTGRE && db->con.postgre!=NULL) {
			PQfinish(db->con.postgre);
			db->con.postgre = NULL;
		}
#endif
#ifdef USE_SQLITE3
#ifdef USE_MYSQL
		else
#else
#ifdef USE_POSTGRESQL
		else
#endif
#endif
		if(db->type==DB_SQLITE3 && db->con.sqlite3!=NULL) {
			sqlite3_close(db->con.sqlite3);
			db->con.sqlite3 = NULL;
		}
#endif
	}
}

static int db_reconnect(QDb db) {
	const char *err = NULL;
	db_close(db);
	err = db_open(db);
	if(err!=NULL) {
		q_err(ERR_DB_OPEN,err);
		db_close(db);
		return 0;
	}
	return 1;
}


static int result_check(QDbResult res) {
	if(res==NULL || res->db==NULL) {
		q_err(ERR_NULL_POINTER,NULL);
		return 0;
	}
	if(
#ifdef USE_MYSQL
		(res->db->type==DB_MYSQL && res->res.mysql==NULL)
#endif
#ifdef USE_POSTGRESQL
#ifdef USE_MYSQL
		||
#endif
		(res->db->type==DB_POSTGRE && res->res.postgre==NULL)
#endif
#ifdef USE_SQLITE3
#ifdef USE_MYSQL
		||
#else
#ifdef USE_POSTGRESQL
		||
#endif
#endif
		(res->db->type==DB_SQLITE3 && res->res.sqlite3==NULL)
#endif
	) {
		q_err(ERR_DB_EMPTY_RESULT,"Result is empty.");
		return 0;
	}
	return 1;
}

QDb q_db_open(int type,const char *host,const char *usr,const char *pwd,const char *dbnm) {
	QDb db = (QDb)malloc(sizeof(_QDb));
	const char *err = NULL;
	if(db==NULL) q_err(ERR_MALLOC,NULL);
	else {
		memset(db,0,sizeof(_QDb));
		db->type = type;
		db->host = host;
		db->usr = usr;
		db->pwd = pwd;
		db->dbnm = dbnm;

		err = db_open(db);
		if(err!=NULL) {
			q_err(ERR_DB_OPEN,err);
			free(db);
			return NULL;
		}
	}
	return db;
}

void q_db_close(QDb db) {
	if(db!=NULL) {
		db_close(db);
		free(db);
	}
}

uint64_t q_db_escape_string(QDb db,char *to,const char *from,size_t len) {
	if(db_check(db) && to!=NULL && from!=NULL) {
		if(len==0) len = strlen(from);
#ifdef USE_MYSQL
		if(db->type==DB_MYSQL) {
			return (uint64_t)mysql_real_escape_string(db->con.mysql,to,from,len);
		}
#endif
#ifdef USE_POSTGRESQL
#ifdef USE_MYSQL
		else
#endif
		if(db->type==DB_POSTGRE) {
		}
#endif
#ifdef USE_SQLITE3
#ifdef USE_MYSQL
		else
#else
#ifdef USE_POSTGRESQL
		else
#endif
#endif
		if(db->type==DB_SQLITE3) {
		}
#endif
	}
	return 0;
}

int q_db_exec(QDb db,const char *sql) {
	if(!db_check(db)) return -1;
	else {
		const char *err = NULL;
#ifdef USE_MYSQL
		int eid = 0;
		if(db->type==DB_MYSQL) {
			if((eid=mysql_query(db->con.mysql,sql))!=0)
				err = mysql_error(db->con.mysql);
		}
#endif
#ifdef USE_POSTGRESQL
#ifdef USE_MYSQL
		else
#endif
		if(db->type==DB_POSTGRE) {
			PQexec(db->con.postgre,sql);
		}
#endif
#ifdef USE_SQLITE3
#ifdef USE_MYSQL
		else
#else
#ifdef USE_POSTGRESQL
		else
#endif
#endif
		if(db->type==DB_SQLITE3) {
			int msg = sqlite3_exec(db->con.sqlite3,sql,0,0,(char **)&err);
			if(msg==SQLITE_OK) err = NULL;
		}
#endif
		if(err!=NULL) {
			q_err(ERR_DB_QUERY,err);
#ifdef USE_MYSQL
			if((eid==CR_SERVER_GONE_ERROR || eid==CR_SERVER_LOST) && db_reconnect(db)) return 0;
#endif
			return -1;
		}
		return 0;
	}
}

QDbResult q_db_query(QDb db,const char *sql,int store) {
	QDbResult res = (QDbResult)malloc(sizeof(_QDbResult));
	if(res==NULL) q_err(ERR_MALLOC,NULL);
	else {
		memset(res,0,sizeof(_QDbResult));
		res->db = db;
		res->row_count = 0;
#ifdef USE_MYSQL
		if(db->type==DB_MYSQL) {
			if(q_db_exec(db,sql)==-1)
				goto db_query_free_result;
			if(store) res->res.mysql = mysql_store_result(db->con.mysql);
			else res->res.mysql = mysql_use_result(db->con.mysql);
			if(res->res.mysql==NULL)
				goto db_query_free_result;
			res->ncols = mysql_num_fields(res->res.mysql);
			res->nrows = mysql_num_rows(res->res.mysql);
			if(store && res->nrows<=0) {
				mysql_free_result(res->res.mysql);
				goto db_query_free_result;
			}
		}
#endif
#ifdef USE_POSTGRESQL
#ifdef USE_MYSQL
		else
#endif
		if(db->type==DB_POSTGRE) {
			res->res.postgre = PQexec(db->con.postgre,sql);
			if(res->res.postgre==NULL)
				goto db_query_free_result;
			if(PQresultStatus(res->res.postgre)!=PGRES_TUPLES_OK) {
				q_err(ERR_DB_QUERY,"PostgreSQL: Query returned empty result.");
				goto db_query_free_result;
			}
			res->ncols = PQnfields(res->res.postgre);
			res->nrows = PQntuples(res->res.postgre);
			if(res->nrows<=0) {
				PQclear(res->res.postgre);
				goto db_query_free_result;
			}
			res->row = (const char **)malloc(sizeof(const char *)*res->ncols);
			if(res->row==NULL) {
				q_err(ERR_MALLOC,NULL);
				PQclear(res->res.postgre);
				goto db_query_free_result;
			}
		}
#endif
#ifdef USE_SQLITE3
#ifdef USE_MYSQL
		else
#else
#ifdef USE_POSTGRESQL
		else
#endif
#endif
		if(db->type==DB_SQLITE3) {
			int msg = sqlite3_prepare_v2(db->con.sqlite3,sql,strlen(sql)+1,&res->res.sqlite3,NULL);
			if(msg!=SQLITE_OK) {
				q_err(ERR_DB_QUERY,sqlite3_errmsg(db->con.sqlite3));
				goto db_query_free_result;
			}
			res->ncols = sqlite3_column_count(res->res.sqlite3);
			res->row = (const char **)malloc(sizeof(const char *)*res->ncols);
			if(res->row==NULL) {
				q_err(ERR_MALLOC,NULL);
				sqlite3_finalize(res->res.sqlite3);
				goto db_query_free_result;
			}
		}
#endif
	}
	return res;

db_query_free_result:

	free(res);
	return NULL;
}

void q_db_free_result(QDbResult res) {
	if(res==NULL) return;
#ifdef USE_MYSQL
	if(res->db->type==DB_MYSQL && res->res.mysql!=NULL) {
		mysql_free_result(res->res.mysql);
		res->res.mysql = NULL;
	}
#endif
#ifdef USE_POSTGRESQL
#ifdef USE_MYSQL
		else
#endif
	if(res->db->type==DB_POSTGRE && res->res.postgre!=NULL) {
		PQclear(res->res.postgre);
		res->res.postgre = NULL;
	}
#endif
#ifdef USE_SQLITE3
#ifdef USE_MYSQL
		else
#else
#ifdef USE_POSTGRESQL
		else
#endif
#endif
	if(res->db->type==DB_SQLITE3 && res->res.sqlite3!=NULL) {
		sqlite3_finalize(res->res.sqlite3);
		res->res.sqlite3 = NULL;
	}
#endif
	if(res->row!=NULL) {
		free(res->row);
		res->row = NULL;
	}
	free(res);
}

const char ** const q_db_fetch_row(QDbResult res) {
	if(result_check(res)) {
#ifdef USE_MYSQL
		if(res->db->type==DB_MYSQL) {
			++res->row_count;
			return (const char ** const)mysql_fetch_row(res->res.mysql);
		}
#endif
#ifdef USE_POSTGRESQL
#ifdef USE_MYSQL
		else
#endif
		if(res->db->type==DB_POSTGRE && res->row!=NULL) {
			int col;
			for(col=0; col<res->ncols; ++col)
				res->row[col] = (const char *)PQgetvalue(res->res.postgre,res->row_count,col);
			++res->row_count;
			return res->row;
		}
#endif
#ifdef USE_SQLITE3
#ifdef USE_MYSQL
		else
#else
#ifdef USE_POSTGRESQL
		else
#endif
#endif
		if(res->db->type==DB_SQLITE3 && res->row!=NULL) {
			if(sqlite3_step(res->res.sqlite3)==SQLITE_ROW) {
				int col;
				for(col=0; col<res->ncols; ++col)
					res->row[col] = (const char *)sqlite3_column_text(res->res.sqlite3,col);
				return res->row;
			}
		}
#endif
	}
	return NULL;
}

uint64_t q_db_affected_rows(QDb db) {
	if(db_check(db)) {
#ifdef USE_MYSQL
		if(db->type==DB_MYSQL) {
			return (uint64_t)mysql_affected_rows(db->con.mysql);
		}
#endif
#ifdef USE_POSTGRESQL
#ifdef USE_MYSQL
		else
#endif
		if(db->type==DB_POSTGRE) {
		}
#endif
#ifdef USE_SQLITE3
#ifdef USE_MYSQL
		else
#else
#ifdef USE_POSTGRESQL
		else
#endif
#endif
		if(db->type==DB_SQLITE3) {
		}
#endif
	}
	return 0;
}

uint64_t q_db_insert_id(QDb db) {
	if(db_check(db)) {
#ifdef USE_MYSQL
		if(db->type==DB_MYSQL) {
			return (uint64_t)mysql_insert_id(db->con.mysql);
		}
#endif
#ifdef USE_POSTGRESQL
#ifdef USE_MYSQL
		else
#endif
		if(db->type==DB_POSTGRE) {
		}
#endif
#ifdef USE_SQLITE3
#ifdef USE_MYSQL
		else
#else
#ifdef USE_POSTGRESQL
		else
#endif
#endif
		if(db->type==DB_SQLITE3) {
			return (uint64_t)sqlite3_last_insert_rowid(db->con.sqlite3);
		}
#endif
	}
	return 0;
}


const char *q_db_error(QDb db) {
	if(db_check(db)) {
#ifdef USE_MYSQL
		if(db->type==DB_MYSQL) {
			return mysql_error(db->con.mysql);
		}
#endif
#ifdef USE_POSTGRESQL
#ifdef USE_MYSQL
		else
#endif
		if(db->type==DB_POSTGRE) {
		}
#endif
#ifdef USE_SQLITE3
#ifdef USE_MYSQL
		else
#else
#ifdef USE_POSTGRESQL
		else
#endif
#endif
		if(db->type==DB_SQLITE3) {
			return sqlite3_errmsg(db->con.sqlite3);
		}
#endif
	}
	return NULL;
}



#endif /* USE_DB */


