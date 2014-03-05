#include "denora.h"
#define AUTHOR "Jobe"
#define VERSION "1.0"
#define MYNAME "ircd_admin"

int my_server(int argc, char **argv);
int my_squit(int argc, char **argv);
int my_event_256(char *source, int ac, char **av);
int my_event_257(char *source, int ac, char **av);
int my_event_258(char *source, int ac, char **av);
int my_event_259(char *source, int ac, char **av);

void create_table();
void init_table();

char *servadmintable = "ircd_admin";

int DenoraInit(int argc, char **argv)
{
    EvtHook *hook = NULL;
	Message *msg = NULL;
    int status;
    
    hook = createEventHook(EVENT_SERVER, my_server);
    status = moduleAddEventHook(hook);
    if (status != MOD_ERR_OK) {
        alog(LOG_NORMAL, "[%s%s] unable to bind to EVENT_SERVER error [%d][%s]", MYNAME, MODULE_EXT, status, ModuleGetErrStr(status));
        return MOD_STOP;
    }
    
    hook = createEventHook(EVENT_SQUIT, my_squit);
    status = moduleAddEventHook(hook);
    if (status != MOD_ERR_OK) {
        alog(LOG_NORMAL, "[%s%s] unable to bind to EVENT_SQUIT error [%d][%s]", MYNAME, MODULE_EXT, status, ModuleGetErrStr(status));
        return MOD_STOP;
    }
    
    msg = createMessage("256", my_event_256);
    status = moduleAddMessage(msg, MOD_HEAD);
    if (status != MOD_ERR_OK) {
        alog(LOG_NORMAL, "[%s%s] unable to bind to 256 error [%d][%s]", MYNAME, MODULE_EXT, status, ModuleGetErrStr(status));
        return MOD_STOP;
    }
    
    msg = createMessage("257", my_event_257);
    status = moduleAddMessage(msg, MOD_HEAD);
    if (status != MOD_ERR_OK) {
        alog(LOG_NORMAL, "[%s%s] unable to bind to 257 error [%d][%s]", MYNAME, MODULE_EXT, status, ModuleGetErrStr(status));
        return MOD_STOP;
    }
    
    msg = createMessage("258", my_event_258);
    status = moduleAddMessage(msg, MOD_HEAD);
    if (status != MOD_ERR_OK) {
        alog(LOG_NORMAL, "[%s%s] unable to bind to 258 error [%d][%s]", MYNAME, MODULE_EXT, status, ModuleGetErrStr(status));
        return MOD_STOP;
    }
    
    msg = createMessage("259", my_event_259);
    status = moduleAddMessage(msg, MOD_HEAD);
    if (status != MOD_ERR_OK) {
        alog(LOG_NORMAL, "[%s%s] unable to bind to 259 error [%d][%s]", MYNAME, MODULE_EXT, status, ModuleGetErrStr(status));
        return MOD_STOP;
    }
    
    create_table();
    rdb_clear_table(servadmintable);
    init_table();
	
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void DenoraFini(void)
{
	alog(LOG_NORMAL, "Unloading ircd_admin%s", MODULE_EXT);
}

int my_server(int argc, char **argv)
{
	Server *serv;
	Uid *ud;
	
	serv = server_find(argv[0]);
	
	if (serv) {
		ud = find_uid(s_StatServ);
		send_cmd((ud ? ud->uid : s_StatServ), "ADMIN %s", (ircd->p10 ? serv->suid : serv->name));
	}
	
	return MOD_CONT;
}

int my_squit(int argc, char **argv)
{
	Server *serv;
	
	serv = server_find(argv[0]);
	
	if (serv) {
		if (argc >= 2) {
			rdb_query(QUERY_LOW, "DELETE FROM %s WHERE server=\'%s\'", servadmintable, serv->name);
		}
	}
	
	return MOD_CONT;
}

int my_event_256(char *source, int ac, char **av)
{
	Server *serv;
	
	serv = server_find(source);
	
	if (serv) {
		MYSQL_RES *mysql_res;
		
		rdb_query(QUERY_LOW, "SELECT * FROM %s WHERE `server` = \'%s\';", servadmintable, serv->name);
		mysql_res = mysql_store_result(mysql);
		if (mysql_num_rows(mysql_res) == 0) {
			rdb_query(QUERY_LOW, "INSERT INTO %s (server) VALUES(\'%s\')", servadmintable, serv->name);
		}
		
		mysql_free_result(mysql_res);
	}
	
	return MOD_CONT;
}

int my_event_257(char *source, int ac, char **av)
{
	Server *serv;
	char *info;
	
	serv = server_find(source);
	
	info = rdb_escape(av[1]);
	
	if (serv) {
		MYSQL_RES *mysql_res;
		
		rdb_query(QUERY_LOW, "SELECT * FROM %s WHERE `server` = \'%s\';", servadmintable, serv->name);
		mysql_res = mysql_store_result(mysql);
		if (mysql_num_rows(mysql_res) == 0) {
			rdb_query(QUERY_LOW, "INSERT INTO %s (server, loc1) VALUES(\'%s\', \'%s\')", servadmintable, serv->name, info);
		} else {
			rdb_query(QUERY_LOW, "UPDATE %s SET loc1 = \'%s\' WHERE server = \'%s\' LIMIT 1;", servadmintable, info, serv->name);
		}
		
		mysql_free_result(mysql_res);
	}
	
	return MOD_CONT;
}

int my_event_258(char *source, int ac, char **av)
{
	Server *serv;
	char *info;
	
	serv = server_find(source);
	
	info = rdb_escape(av[1]);
	
	if (serv) {
		MYSQL_RES *mysql_res;
		
		rdb_query(QUERY_LOW, "SELECT * FROM %s WHERE `server` = \'%s\';", servadmintable, serv->name);
		mysql_res = mysql_store_result(mysql);
		if (mysql_num_rows(mysql_res) == 0) {
			rdb_query(QUERY_LOW, "INSERT INTO %s (server, loc2) VALUES(\'%s\', \'%s\')", servadmintable, serv->name, info);
		} else {
			rdb_query(QUERY_LOW, "UPDATE %s SET loc2 = \'%s\' WHERE server = \'%s\' LIMIT 1;", servadmintable, info, serv->name);
		}
		
		mysql_free_result(mysql_res);
	}
	
	return MOD_CONT;
}

int my_event_259(char *source, int ac, char **av)
{
	Server *serv;
	char *info;
	
	serv = server_find(source);
	
	info = rdb_escape(av[1]);
	
	if (serv) {
		MYSQL_RES *mysql_res;
		
		rdb_query(QUERY_LOW, "SELECT * FROM %s WHERE `server` = \'%s\';", servadmintable, serv->name);
		mysql_res = mysql_store_result(mysql);
		if (mysql_num_rows(mysql_res) == 0) {
			rdb_query(QUERY_LOW, "INSERT INTO %s (server, email) VALUES(\'%s\', \'%s\')", servadmintable, serv->name, info);
		} else {
			rdb_query(QUERY_LOW, "UPDATE %s SET email = \'%s\' WHERE server = \'%s\' LIMIT 1;", servadmintable, info, serv->name);
		}
		
		mysql_free_result(mysql_res);
	}
	
	return MOD_CONT;
}

void create_table()
{
    MYSQL_RES *mysql_res;

	rdb_query(QUERY_LOW, "SHOW TABLES LIKE '%s';", servadmintable);
	mysql_res = mysql_store_result(mysql);
	if (mysql_num_rows(mysql_res) == 0) {
        rdb_query(QUERY_LOW, "CREATE TABLE %s (`id` INT NOT NULL AUTO_INCREMENT PRIMARY KEY, `server` VARCHAR( 255 ) NOT NULL, `loc1` VARCHAR( 255 ) NULL DEFAULT NULL, `loc2` VARCHAR( 255 ) NULL DEFAULT NULL, `email` VARCHAR( 255 ) NULL DEFAULT NULL);", servadmintable);
    }
    mysql_free_result(mysql_res);
}

void init_table()
{
	ServStats *s, *next;
	Server *serv;

	Uid *ud;
	
	ud = find_uid(s_StatServ);
	
	s = first_statsserver();
	while (s) {
		next = next_statserver();
		serv = server_find(s->name);
		if (serv && (serv != me_server)) {
			send_cmd((ud ? ud->uid : s_StatServ), "ADMIN %s", (ircd->p10 ? serv->suid : serv->name));
		}
		s = next;
	}
}

