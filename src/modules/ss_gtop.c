#include "denora.h"
#define AUTHOR "Jobe"
#define VERSION "1.0"
#define MYNAME "ss_gtop"

#define GTOP_HELP 0

#define LANG_NUM_STRINGS 1

int do_gtop(User *u, int ac, char **av);
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

    moduleAddAuthor( AUTHOR);
    moduleAddVersion(VERSION);
    
    c = createCommand("GTOP", do_gtop, NULL, -1, -1, -1, -1);
	moduleAddCommand(STATSERV, c, MOD_UNIQUE);
	moduleSetOperHelp(do_help);
    
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

int do_gtop(User *u, int ac, char **av)
{
#ifdef USE_MYSQL
    MYSQL_RES *mysql_res;
    int i = 1;
#endif

    if (!denora->do_sql) {
        return MOD_CONT;
    }

	SET_SEGV_LOCATION();
	rdb_query
		(QUERY_HIGH,
		"SELECT * FROM %s WHERE chan=\'global\' AND type=0 ORDER BY letters DESC LIMIT 3;",
		UStatsTable);
#ifdef USE_MYSQL
	mysql_res = mysql_store_result(mysql);
	if (mysql_num_rows(mysql_res) > 0) {
		notice_lang(s_StatServ, u, STATS_TOP_NETWORK, u->nick);
		SET_SEGV_LOCATION();
		while ((mysql_row = mysql_fetch_row(mysql_res)) != NULL) {
			notice_lang(s_StatServ, u, STATS_MESSAGE, i, mysql_row[0],
				mysql_row[3], mysql_row[4], mysql_row[5], mysql_row[7],
				mysql_row[6]);
			i++;
		}
	}
	SET_SEGV_LOCATION();
	mysql_free_result(mysql_res);
#endif

    return MOD_CONT;
}

void do_help(User *user)
{
	moduleNoticeLang(s_StatServ, user, GTOP_HELP);
}

void mAddLanguages(void)
{
	const char *langtable_en_us[] = {
		"    GTOP        The top 3  users of the whole network",
	};

	moduleInsertLanguage(LANG_EN_US, LANG_NUM_STRINGS, (char **) langtable_en_us);
}
