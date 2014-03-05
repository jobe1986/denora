#include "denora.h"
#define AUTHOR "Jobe"
#define VERSION "1.0"
#define MYNAME "ss_gstats"

#define GSTATS_HELP 0

#define LANG_NUM_STRINGS 1

int do_gstats(User *u, int ac, char **av);
void do_help(User *user);
void mAddLanguages(void);
int DenoraInit(int argc, char **argv);
void DenoraFini(void);

int DenoraInit(int argc, char **argv)
{
	Command *c;

    if (denora->debug >= 2) {
        protocol_debug(NULL, argc, argv);
    }

    moduleAddAuthor(AUTHOR);
    moduleAddVersion(VERSION);
    
    c = createCommand("GSTATS", do_gstats, NULL, -1, -1, -1, -1);
	moduleAddCommand(STATSERV, c, MOD_UNIQUE);
	moduleSetOperHelp(do_help);

    if (!denora->do_sql) {
        return MOD_STOP;
    }
    
    mAddLanguages();

#ifdef USE_MYSQL
    return MOD_CONT;
#else
    return MOD_STOP;
#endif
}

void DenoraFini(void)
{

}

int do_gstats(User *u, int ac, char **av)
{
#ifdef USE_MYSQL
    MYSQL_RES *mysql_res;
#endif

    if (!denora->do_sql) {
		return MOD_CONT;
    }
    if (!u->sgroup) {
		return MOD_CONT;
    }
	rdb_query
		(QUERY_HIGH, "SELECT * FROM %s WHERE chan=\'global\' AND type=0 AND uname=\'%s\';",
		UStatsTable, u->sgroup);
#ifdef USE_MYSQL
	mysql_res = mysql_store_result(mysql);
	if (mysql_num_rows(mysql_res) > 0) {
		SET_SEGV_LOCATION();
		while ((mysql_row = mysql_fetch_row(mysql_res)) != NULL) {
			notice_lang(s_StatServ, u, STATS_USER_NETWORK, u->nick);
			notice_lang(s_StatServ, u, STATS_MESSAGE_ONE, mysql_row[3],
				mysql_row[4], mysql_row[5], mysql_row[7], mysql_row[6]);
		}
		SET_SEGV_LOCATION();
		mysql_free_result(mysql_res);
    }
#endif

    return MOD_CONT;
}

void do_help(User *user)
{
	moduleNoticeLang(s_StatServ, user, GSTATS_HELP);
}

void mAddLanguages(void)
{
	const char *langtable_en_us[] = {
		"    GSTATS      User stats for the network",
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, (char **) langtable_en_us);
}
