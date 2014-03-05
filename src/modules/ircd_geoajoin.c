#include "denora.h"
#define AUTHOR "Jobe"
#define VERSION "2.1"
#define MYNAME "ircd_geoajoin"

#define GEOAJOIN_USAGE 0
#define GEOAJOIN_USAGE_ADD 1
#define GEOAJOIN_USAGE_DEL 2
#define GEOAJOIN_USAGE_FROM 3
#define GEOAJOIN_FROM 4
#define GEOAJOIN_NOFROM 5
#define GEOAJOIN_LIST 6
#define GEOAJOIN_NOLIST 7
#define GEOAJOIN_ADD 8
#define GEOAJOIN_UPDATE 9
#define GEOAJOIN_DEL 10
#define GEOAJOIN_NODEL 11
#define GEOAJOIN_HELP 12
#define GEOAJOIN_HELP_GEOAJOIN 13

#define LANG_NUM_STRINGS 14

char *geoajointable = "ircd_geoajoin";

int my_event_newnick(int argc, char **argv);
int my_cmd_geoajoin(User *user, int ac, char **av);
void do_help(User *user);
int do_help_geoajoin(User *user);
void mAddLanguages(void);
void create_table();
void do_ajoin(User *u, char *chans);
int do_delayed_ajoin(int argc, char *argv[]);

int DenoraInit(int argc, char **argv)
{
	EvtHook *hook = NULL;
	Command *c;
	int status;

	if ((denora_get_ircd() != IRC_UNREAL32) && (denora_get_ircd() != IRC_NEFARIOUS)) {
		alog(LOG_NORMAL, "[%s%s] unsupported IRCd detected", MYNAME, MODULE_EXT);
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_NEWNICK, my_event_newnick);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog(LOG_NORMAL, "[%s%s] unable to bind to EVENT_NEWNICK error [%d][%s]", MYNAME, MODULE_EXT, status, ModuleGetErrStr(status));
		return MOD_STOP;
	}

	c = createCommand("GEOAJOIN", my_cmd_geoajoin, is_stats_admin, -1, -1, -1, -1);
	moduleAddCommand(STATSERV, c, MOD_UNIQUE);
	moduleAddAdminHelp(c, do_help_geoajoin);
	moduleSetOperHelp(do_help);

	mAddLanguages();

	create_table();

	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void create_table(void)
{
	MYSQL_RES *mysql_res, *mysql_res2;

	rdb_query(QUERY_LOW, "SHOW TABLES LIKE '%s';", geoajointable);
	mysql_res = mysql_store_result(mysql);
	if (mysql_num_rows(mysql_res) == 0) {
		rdb_query(QUERY_LOW, "CREATE TABLE %s (`countrycode` VARCHAR( 2 ) NOT NULL, `channel` VARCHAR( 255 ) NOT NULL, `delay` INT(11) NOT NULL DEFAULT 0);", geoajointable);
	} else {
		rdb_query(QUERY_LOW, "SHOW COLUMNS FROM `%s` WHERE `Field` = 'delay';", geoajointable);
		mysql_res = mysql_store_result(mysql);
		if (mysql_num_rows(mysql_res) == 0) {
			rdb_query(QUERY_LOW, "ALTER TABLE `%s` ADD `delay` INT(11) NOT NULL DEFAULT '0'", geoajointable);
		}
		mysql_free_result(mysql_res2);
	}
	mysql_free_result(mysql_res);
}

int my_event_newnick(int argc, char **argv)
{
	User *u;
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	char *sqlcc;
	char *av[3];
	int sqldelay = 0;
	u = user_find(argv[0]);

	if (denora->debug)
	        protocol_debug(NULL, argc, argv);
	
	if (u) {
		av[0] = (ircd->p10 ? u->uid : u->nick);

		sqlcc = rdb_escape(u->country_code);
		rdb_query(QUERY_LOW, "SELECT `channel`, `delay` FROM `%s` WHERE `countrycode` = '**' OR `countrycode` = '%s' ORDER BY `countrycode`;", geoajointable, sqlcc);
		mysql_res = mysql_store_result(mysql);
		if (mysql_num_rows(mysql_res) > 0 ) {
			while ((mysql_row = mysql_fetch_row(mysql_res)) != NULL) {
				if (mysql_row[1] != NULL)
					sqldelay = atoi(mysql_row[1]);
				if (sqldelay < 0)
					sqldelay = 0;
				if (mysql_row[0] != NULL) {
					if (sqldelay == 0)
						do_ajoin(u, mysql_row[0]);
					else {
						av[1] = sstrdup(mysql_row[0]);
						moduleAddCallback("geoajoin_delayed", time(NULL) + sqldelay, do_delayed_ajoin, 2, av);
					}
				}
			}
		}
		mysql_free_result(mysql_res);
	}

	return MOD_CONT;
}

int my_cmd_geoajoin(User *user, int ac, char **av)
{
	User *u;
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;
	char *sqlcc, *sqlchannels;
	int sqldelay = 0;

	if (denora->debug)
        	protocol_debug(NULL, ac, av);

	if (ac < 1 || BadChar(av[0])) {
		moduleNoticeLang(s_StatServ, user, GEOAJOIN_USAGE);
	} else {
		if (!stricmp(av[0], "FROM")) {
			if (ac < 2 || BadChar(av[1])) {
				moduleNoticeLang(s_StatServ, user, GEOAJOIN_USAGE_FROM);
			} else {
				u = user_find(av[1]);
				if (u) {
					moduleNoticeLang(s_StatServ, user, GEOAJOIN_FROM, u->nick, u->country_name, u->country_code);
				} else {
					moduleNoticeLang(s_StatServ, user, GEOAJOIN_NOFROM, av[1]);
				}
			}
		} else if (!stricmp(av[0], "LIST")) {
			rdb_query(QUERY_LOW, "SELECT `countrycode`, `channel` FROM `%s` ORDER BY `countrycode`;", geoajointable);
			mysql_res = mysql_store_result(mysql);
			if (mysql_num_rows(mysql_res) > 0 ) {
				SET_SEGV_LOCATION();
				while ((mysql_row = mysql_fetch_row(mysql_res)) != NULL) {
					if ((mysql_row[0] != NULL) && (mysql_row[1] != NULL)) {
						if (!stricmp(mysql_row[0], "??")) {
							moduleNoticeLang(s_StatServ, user, GEOAJOIN_LIST, "Unknown", mysql_row[1]);
						} else if (!stricmp(mysql_row[0], "**")) {
							moduleNoticeLang(s_StatServ, user, GEOAJOIN_LIST, "All", mysql_row[1]);
						} else {
							moduleNoticeLang(s_StatServ, user, GEOAJOIN_LIST, mysql_row[0], mysql_row[1]);
						}
					}
				}
			} else {
				moduleNoticeLang(s_StatServ, user, GEOAJOIN_NOLIST);
			}
			mysql_free_result(mysql_res);
		} else if (!stricmp(av[0], "ADD")) {
			if (ac < 3 || BadChar(av[1]) || BadChar(av[2])) {
				moduleNoticeLang(s_StatServ, user, GEOAJOIN_USAGE_ADD);
			} else {
				sqlcc = rdb_escape(av[1]);
				sqlchannels = rdb_escape(av[2]);
				if (ac > 3)
					sqldelay = atoi(av[3]);
				if (sqldelay < 0)
					sqldelay = 0;
				rdb_query(QUERY_LOW, "SELECT `countrycode`, `channel` FROM `%s` WHERE `countrycode` = LEFT('%s', 2);", geoajointable, sqlcc);
				mysql_res = mysql_store_result(mysql);
				if (mysql_num_rows(mysql_res) > 0 ) {
					rdb_query(QUERY_LOW, "UPDATE `%s` SET `countrycode` = LEFT('%s', 2), `channel` = '%s', `delay` = '%d' WHERE `countrycode` = LEFT('%s', 2);", geoajointable, sqlcc, sqlchannels, sqldelay, sqlcc);
					moduleNoticeLang(s_StatServ, user, GEOAJOIN_UPDATE, sqlcc, sqlchannels, sqldelay);
				} else {
					rdb_query(QUERY_LOW, "INSERT INTO `%s` (`countrycode`, `channel`, `delay`) VALUES (LEFT('%s', 2), '%s', '%d');", geoajointable, sqlcc, sqlchannels, sqldelay);
					moduleNoticeLang(s_StatServ, user, GEOAJOIN_ADD, sqlcc, sqlchannels, sqldelay);
				}
				mysql_free_result(mysql_res);
			}
		} else if (!stricmp(av[0], "DEL") || !stricmp(av[0], "DELETE")) {
			if (ac < 2 || BadChar(av[1])) {
				moduleNoticeLang(s_StatServ, user, GEOAJOIN_USAGE_DEL);
			} else {
				sqlcc = rdb_escape(av[1]);
				rdb_query(QUERY_LOW, "SELECT `countrycode` FROM `%s` WHERE `countrycode` = LEFT('%s', 2);", geoajointable, sqlcc);
				mysql_res = mysql_store_result(mysql);
				if (mysql_num_rows(mysql_res) > 0) {
					rdb_query(QUERY_LOW, "DELETE FROM `%s` WHERE `countrycode` = LEFT('%s', 2) LIMIT 1;", geoajointable, sqlcc);
					moduleNoticeLang(s_StatServ, user, GEOAJOIN_DEL, sqlcc);
				} else {
					moduleNoticeLang(s_StatServ, user, GEOAJOIN_NODEL, sqlcc);
				}
				mysql_free_result(mysql_res);
			}
		} else if (!stricmp(av[0], "HELP")) {
			do_help_geoajoin(user);
		}
	}

	return MOD_CONT;
}

void do_ajoin(User *u, char *chans)
{
	Uid *ud;
	
	if (denora_get_ircd() == IRC_UNREAL32) {
		send_cmd(s_StatServ, "SVSJOIN %s %s", u->nick, chans);
	} else if (denora_get_ircd() == IRC_NEFARIOUS) {
		ud = find_uid(s_StatServ);
		send_cmd((ud ? ud->uid : s_StatServ), "SVSJOIN %s %s", (ircd->p10 ? u->uid : u->nick), chans);
	}
}

int do_delayed_ajoin(int argc, char *argv[])
{
	User *u;

	if (argv[0])
		u = user_find(argv[0]);
	else
		return MOD_CONT;

	if (u) {
		do_ajoin(u, argv[1]);
	}

	return MOD_CONT;
}

void do_help(User *user)
{
	if (is_stats_admin(user)) {
		moduleNoticeLang(s_StatServ, user, GEOAJOIN_HELP);
	}
}

int do_help_geoajoin(User *user)
{
	if (is_stats_admin(user)) {
		moduleNoticeLang(s_StatServ, user, GEOAJOIN_HELP_GEOAJOIN);
	}
	return MOD_CONT;
}

void mAddLanguages(void)
{
	const char *langtable_en_us[] = {
		"Usage: \002GEOAJOIN {ADD|DEL|FROM}\002",
		"Usage: \002GEOAJOIN ADD countrycode channels [delay]\002",
		"Usage: \002GEOAJOIN DEL countrycode\002",
		"Usage: \002GEOAJOIN FROM nick\002",
		"%s is from %s (%s)",
		"Cannot find a user with the nick %s",
		"%s => %s",
		"There are currently no GeoIP ajoins set",
		"%s added to the GEOAJOIN list to join %s after %d seconds",
		"%s updated in the GEOAJOIN list to join %s after %d seconds",
		"%s deleted from the GEOAJOIN list",
		"%s could not be deleted from the GEOAJOIN list as it does not exist",
		"    GEOAJOIN    Manage the GeoIP AJOIN interface",
		"GEOAJOIN \002ADD\002 countrycode channels [delay]\n"
		"GEOAJOIN \002DEL\002 countrycode\n"
		"GEOAJOIN \002LIST\002\n"
		"\n"
		"The GEOAJOIN ADD command allows you to set the channel(s) to ajoin\n"
		"a user to for the specified country code\n"
		"\n"
		"The GEOAJOIN DEL command allows you to remove the specified\n"
		"countrycode"
		"\n"
		"The GEOAJOIN LIST command allows you to list all the countrycodes\n"
		"with an ajoin set\n"
		"\n"
		"countrycode is a 2 letter ISO 3166-1 alpha-2 country code, ?? for\n"
		"unknown or ** for everyone\n"
		"channels is a single channel or a comma (,) seperate list of channels\n"
		"delay is the delay before the join in seconds",
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, (char **) langtable_en_us);
}

