#include "denora.h"
#define AUTHOR "Jobe"
#define VERSION "1.0"
#define MYNAME "ircd_alist"

int my_event_alist(char *source, int ac, char **av);

void create_table();

char *alisttable = "ircd_alist";

int DenoraInit(int argc, char **argv)
{
	EvtHook *hook = NULL;
	Message *msg = NULL;
	int status;
	
	msg = createMessage("ALIST", my_event_alist);
	status = moduleAddMessage(msg, MOD_HEAD);
	if (status != MOD_ERR_OK) {
		alog(LOG_NORMAL, "[%s%s] unable to bind to ALIST error [%d][%s]", MYNAME, MODULE_EXT, status, ModuleGetErrStr(status));
		return MOD_STOP;
	}
	
	create_table();
	rdb_clear_table(alisttable);
	
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void DenoraFini(void)
{
	alog(LOG_NORMAL, "Unloading ircd_alist%s", MODULE_EXT);
}

int my_event_alist(char *source, int ac, char **av)
{
	char *chan;
	int lastactive = 0;

	chan = rdb_escape(av[0]);
	lastactive = strtol(av[1], NULL, 10);
	
	if (chan) {
		MYSQL_RES *mysql_res;
		rdb_query(QUERY_LOW, "SELECT * FROM %s WHERE chan = '%s';", alisttable, chan);
		mysql_res = mysql_store_result(mysql);
		if (mysql_res && mysql_num_rows(mysql_res) == 0) {
			rdb_query(QUERY_LOW, "INSERT INTO %s (`chan`, `lastactive`) VALUES ('%s', %d)", alisttable, chan, lastactive);
		} else {
			rdb_query(QUERY_LOW, "UPDATE %s SET lastactive = %d WHERE chan = '%s' LIMIT 1;", alisttable, lastactive, chan);
		}
		if (mysql_res) {
			mysql_free_result(mysql_res);
		}
	}
	
	free(chan);
	return MOD_CONT;
}

void create_table()
{
	MYSQL_RES *mysql_res;
	
	rdb_query(QUERY_LOW, "SHOW TABLES LIKE '%s';", alisttable);
	mysql_res = mysql_store_result(mysql);
	if (mysql_res) {
		if (mysql_num_rows(mysql_res) == 0) {
			rdb_query(QUERY_LOW, "CREATE TABLE %s (`chan` VARCHAR(255) NOT NULL, `lastactive` INT(15) NULL DEFAULT 0);", alisttable);
		}
		mysql_free_result(mysql_res);
	}
	
	rdb_query(QUERY_LOW, "TRUNCATE TABLE %s;", alisttable);
}

