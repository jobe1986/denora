#include "denora.h"
#define AUTHOR "Jobe"
#define VERSION "1.0"
#define MYNAME "ircd_autologin"

int my_event_newnick(int argc, char **argv);
int my_msg_account(char *source, int ac, char **av);
void do_login(User *u, char *user);
void do_logout(User *u);

int DenoraInit(int argc, char **argv)
{
	EvtHook *hook = NULL;
	Message *msg = NULL;
	int status;
	
	if (!ircd->p10) {
		alog(LOG_NORMAL, "[%s%s] Youre IRCd is not supported, unloading ircd_autologin!", MYNAME, MODULE_EXT);
		return MOD_STOP;
	}

	hook = createEventHook(EVENT_NEWNICK, my_event_newnick);
	status = moduleAddEventHook(hook);
	if (status != MOD_ERR_OK) {
		alog(LOG_NORMAL, "[%s%s] unable to bind to EVENT_NEWNICK error [%d][%s]", MYNAME, MODULE_EXT, status, ModuleGetErrStr(status));
		return MOD_STOP;
	}
    
	msg = createMessage("AC", my_msg_account);
	status = moduleAddMessage(msg, MOD_HEAD);
	if (status != MOD_ERR_OK) {
		alog(LOG_NORMAL, "[%s%s] unable to bind to AC error [%d][%s]", MYNAME, MODULE_EXT, status, ModuleGetErrStr(status));
		return MOD_STOP;
	}
	
	moduleAddAuthor(AUTHOR);
	moduleAddVersion(VERSION);
	return MOD_CONT;
}

void DenoraFini(void)
{
	alog(LOG_NORMAL, "Unloading ircd_autologin%s", MODULE_EXT);
}

int my_event_newnick(int argc, char **argv)
{
	User *u;
	u = user_find(argv[0]);
	
	if (u)
	{
		if (u->account)
		{
			do_login(u, u->account);
		}
	}
	
	return MOD_CONT;
}

int my_msg_account(char *source, int ac, char **av)
{
	Server *s;
    User *u;
    
	if (denora->protocoldebug)
	{
		protocol_debug(source, ac, av);
	}

	if ((ac < 2) || !source || !(s = server_find(source)))
	{
		return MOD_CONT;
	}
	
	u = user_find(av[0]);
	if (!u)
		return MOD_CONT;
		
	if (!strcmp(av[1], "R"))			/* Set */
	{
		do_login(u, av[2]);
	}
	else if (!strcmp(av[1], "M"))		/* Rename */
	{
		do_logout(u);
		do_login(u, av[2]);
	}
	else if (!strcmp(av[1], "U"))		/* Remove */
	{
		do_logout(u);
	}
	else
	{
		do_login(u, av[1]);				/* For backward compatability */
	}
	
	return MOD_CONT;
}

void do_login(User *u, char *user)
{
	Dadmin *aadmin;
	
	aadmin = find_admin(user, u);
	if (!aadmin) {
		u->admin = 0;
	} else {
		u->admin = 1;
		if (aadmin->configfile == 1) {
			u->confadmin = 1;
		} else {
			u->confadmin = 0;
		}
		if (aadmin->language) {
			u->language = aadmin->language;
		}
	}
	
	if (u->admin) {
		denora_cmd_global(s_StatServ, langstring(NOW_STATS_ADMIN),
							u->nick, user);
		notice_lang(s_StatServ, u, STAT_ADMIN_LOGGED_IN);
	}
}

void do_logout(User *u)
{
	if (u->admin) {
		u->admin = 0;
		u->confadmin = 0;
		denora_cmd_global(s_StatServ, langstring(STATS_ADMIN_LOGOUT),
							u->nick);
		notice_lang(s_StatServ, u, STAT_ADMIN_LOGGED_OUT);
	}

}

