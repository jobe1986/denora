/*
 * log_kill module for Denora 1.4+
 * (c) Jobe (jobe@mdbnet.co.uk)
 */

#include "denora.h"
#define AUTHOR "Jobe"
#define VERSION "1.0.0"
#define MYNAME "log_kill"

#define KILLTABLE "kills"

int DenoraInit(int argc, char **argv);
void DenoraFini(void);
int my_kill(int argc, char **argv);
int my_skill(int argc, char **argv);
void create_table(void);

int DenoraInit(int argc, char **argv)
{
	EvtHook *hook = NULL;
	int status;

	hook = createEventHook(EVENT_GLOBAL_KILL, my_kill);
	if ((status = moduleAddEventHook(hook)) != MOD_ERR_OK) {
		alog(LOG_NORMAL, "[%s%s] unable to bind to EVENT_GLOBAL_KILL error [%d][%s]", MYNAME, MODULE_EXT, status, ModuleGetErrStr(status));
		return MOD_STOP;
	}

	create_table();

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void DenoraFini(void)
{
	alog(LOG_NORMAL, "Unloading %s%s", MYNAME, MODULE_EXT);
}

int my_kill(int argc, char **argv)
{
	char *oper, *victim, *reason;

	if (argc > 2) {
		/* argv[0] = oper, argv[1] = victim, argv[2] = reason */
		oper = rdb_escape(argv[0]);
		victim = rdb_escape(argv[1]);
		reason = rdb_escape(argv[2]);

		rdb_query(QUERY_LOW, "INSERT INTO %s (time, oper, victim, reason) VALUES(%d,\'%s\',\'%s\', \'%s\')",
			KILLTABLE, (int) time(NULL), oper, victim, reason);
	}
	return MOD_CONT;
}

void create_table(void)
{
	MYSQL_RES *mysql_res;

	rdb_query(QUERY_LOW, "SHOW TABLES LIKE '%s';", KILLTABLE);
	mysql_res = mysql_store_result(mysql);
	if (mysql_num_rows(mysql_res) == 0) {
		rdb_query(QUERY_LOW, "CREATE TABLE %s (time INT NOT NULL DEFAULT '0', oper VARCHAR( 31 ) NULL DEFAULT NULL, victim VARCHAR( 31 ) NULL DEFAULT NULL, reason TEXT NULL DEFAULT NULL);", KILLTABLE);
	}

	mysql_free_result(mysql_res);
}

