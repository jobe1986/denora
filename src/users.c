/*
 *
 * (c) 2004-2013 Denora Team
 * Contact us at info@denorastats.org
 *
 * Please read COPYING and README for furhter details.
 *
 * Based on the original code of Anope by Anope Team.
 * Based on the original code of Thales by Lucas.
 *
 *
 *
 */

#include "denora.h"

User *userlist[MAX_USERS];
Uid *uidlist[MAX_UIDS];
static Uid *ucurrent;
static User *current;
static User *uidcurrent;
static int next_index;
static int unext_index;
static int uidnext_index;
User *new_user(const char *nick);
GeoIP *gidb;
GeoIP *gidb_v6;

/*************************************************************************/

/**
 * SQL handle the user mode changes
 *
 * @param u is the user struct for which the modes changed
 * @param modes is the modes to add/remove
 *
 * @return void - no returend value
 *
 */
void sql_do_usermodes(User * u, char *modes)
{
	int atleastone = 0;
	char db[MAX_SQL_BUF];
	char tmp[14] = "mode_XX=\'X\', ";

	SET_SEGV_LOCATION();

	if (strlen(modes) == 1 && (*modes == '+' || *modes == '-'))
	{
		alog(LOG_DEBUG, "Received just %s as the users mode for %s", modes, u->nick);
		return;
	}

	if (!denora->do_sql)
	{
		return;
	}

	if (u->sqlid < 1 && db_getnick(u->sqlnick) == -1)
	{
		alog(LOG_NONEXISTANT, "Trying to update usermodes %s on nonexistant user %s", modes, u->nick);
		return;
	}

	SET_SEGV_LOCATION();

	*db = '\0';
	ircsnprintf(db, sizeof(db), "UPDATE %s SET ", UserTable);

	SET_SEGV_LOCATION();

	while (*modes)
	{
		switch (*modes)
		{
			case '+':
				tmp[9] = 'Y';
				break;
			case '-':
				tmp[9] = 'N';
				break;
			default:
				if (!strchr(ircd->usermodes, *modes))
				{
					alog(LOG_DEBUG, langstr(ALOG_UKNOWN_USER_MODE), *modes,
					     inbuf);
				}
				else
				{
					atleastone = 1;
					tmp[5] = ((*modes >= 'a') ? 'l' : 'u');
					tmp[6] = tolower(*modes);
					strlcat(db, tmp, sizeof(db));
				}
				break;
		}
		modes++;
	}
	SET_SEGV_LOCATION();

	if (atleastone)
	{
		ircsnprintf(&db[strlen(db) - 2], sizeof(db), " WHERE nickid=%d", u->sqlid);
		rdb_query(QUERY_LOW, db);
	}
	else
	{
		alog(LOG_DEBUG,
		     "There were no modes or atleastone (%d) was still 0", atleastone);
		alog(LOG_DEBUG, "buffer = %s", db);
	}

	return;
}

/*************************************************************************/

/**
 * Reset all user modes to N for the given nick ID
 */

void sql_reset_usermodes(User *u)
{
	const char *modes = sstrdup(ircd->usermodes);
	char db[MAX_SQL_BUF];
	char tmp[14] = "mode_XX=\'X\', ";

	*db = '\0';

	if (!denora->do_sql)
	{
		return;
	}

	if (!u)
	{
		alog(LOG_NONEXISTANT, "Trying to reset usermodes on a non-existing user");
		return;
	}

	if (u->sqlid < 1 && db_getnick(u->sqlnick) == -1)
	{
		alog(LOG_NONEXISTANT, "Trying to reset usermodes on nonexistant user %s", u->sqlnick);
		return;
	}

	ircsnprintf(db, sizeof(db), "UPDATE %s SET ", UserTable);

	while (*modes)
	{
		tmp[9] = 'N';
		tmp[5] = ((*modes >= 'a') ? 'l' : 'u');
		tmp[6] = tolower(*modes);
		strlcat(db, tmp, sizeof(db));
		modes++;
	}
	SET_SEGV_LOCATION();

	ircsnprintf(&db[strlen(db) - 2], sizeof(db), " WHERE nickid=%d", u->sqlid);

	rdb_query(QUERY_LOW, db);

	return;
}

/*************************************************************************/

/**
 * Store the SWHOIS message
 *
 * @param user is the nick of the user changing SWHOIS for
 * @param msg is the swhois message
 *
 * @return void - no returend value
 *
 */
void do_swhois(char *user, char *msg)
{
	User *u;
	char *sqlmsg = NULL;

	SET_SEGV_LOCATION();

	/* Find the user struct for the given user */
	u = user_find(user);
	if (!u)
	{
		alog(LOG_NONEXISTANT, langstr(ALOG_SWHOIS_ERROR), user);
		return;		 /* Can't find them exit */
	}

	/* Check if current set, if so we should free it before using it */
	if (u->swhois)
	{
		free(u->swhois);
		u->swhois = NULL;
	}
	u->swhois = (!msg || !*msg) ? NULL : sstrdup(msg);

	SET_SEGV_LOCATION();

	if (denora->do_sql)
	{
		if (u->sqlid < 1 && db_getnick(u->sqlnick) == -1)
		{
			alog(LOG_NONEXISTANT, langstr(ALOG_SWHOIS_ERROR), user);
		}
		else
		{
			sqlmsg = (!msg || !*msg) ? NULL : rdb_escape(msg);
			rdb_query(QUERY_LOW,
				  "UPDATE %s SET swhois=\'%s\' WHERE nickid=%d",
				  UserTable, sqlmsg, u->sqlid);
			if (sqlmsg)
			{
				free(sqlmsg);
			}
		}
	}

	SET_SEGV_LOCATION();
	return;
}

/*************************************************************************/

/**
 * Update SQL for the new nick
 *
 * @param u is the user struct from which we will pull the information
 *
 * @return void - no returend value
 *
 */
void sql_do_nick(User * u)
{
	int servid;
	char *username, *account, *host, *vhost, *server, *realname,
	     *countryname, *countrycode;

	SET_SEGV_LOCATION();

	if (!u)
	{
		return;
	}

	username = (u->username) ? rdb_escape(u->username) : NULL;
	account = (u->account) ? rdb_escape(u->account) : NULL;
	host = (u->host) ? rdb_escape(u->host) : NULL;
	server = (u->server->name) ? rdb_escape(u->server->name) : NULL;
	vhost = (ircd->vhost) ? rdb_escape(u->vhost) : NULL;
	realname = (u->realname) ? rdb_escape(u->realname) : NULL;
	servid = db_getserver(u->server->name);

	countryname = rdb_escape(u->country_name);
	countrycode = rdb_escape(u->country_code);

	SET_SEGV_LOCATION();

	if ((LargeNet || !UplinkSynced || UserStatsRegistered) && u->sqlid < 1)
	{
		db_getnick(u->sqlnick);
	}
	else if (u->sqlid < 1)
	{
		db_checknick_nt(u->nick);
	}
	
	if (u->sqlid > 0)
	{
		SET_SEGV_LOCATION();
		rdb_query(QUERY_LOW,
		          "UPDATE %s SET nick='%s', hopcount=%d, nickip='%s', countrycode='%s', country='%s', realname='%s', hostname='%s', hiddenhostname='%s', username='%s', swhois='', account='%s', connecttime=FROM_UNIXTIME(%ld), servid=%d, server='%s', lastquit=NULL, online='Y', away='%s', awaymsg='%s' WHERE nickid=%d",
		          UserTable, u->sqlnick, u->hopcount, u->ip, countrycode, countryname, realname, host, vhost, username, account,
		          (long int) u->timestamp, servid, server, u->isaway ? "Y" : "N", u->isaway && u->awaymsg ? u->awaymsg : NULL, u->sqlid);
	}
	else
	{
		SET_SEGV_LOCATION();
		rdb_query(QUERY_HIGH,
		          "INSERT INTO %s (nick,hopcount,nickip,realname,hostname,hiddenhostname,username,swhois,account,connecttime,servid,server,countrycode,country) VALUES('%s',%d,'%s','%s','%s','%s','%s','','%s',FROM_UNIXTIME(%ld),%d,'%s','%s','%s') ON DUPLICATE KEY UPDATE nick='%s',hopcount=%d,nickip='%s',realname='%s',hostname='%s',hiddenhostname='%s',username='%s',account='%s',connecttime=FROM_UNIXTIME(%ld),servid=%d,server='%s',countrycode='%s',country='%s',lastquit=NULL,online='Y',away='%s',awaymsg='%s'",
			  UserTable, u->sqlnick, u->hopcount, u->ip, realname,
			  host, vhost, username, account, (long int) u->timestamp,
			  servid, server, countrycode, countryname, u->sqlnick,
			  u->hopcount, u->ip, realname, host, vhost, username,
			  account, (long int) u->timestamp, servid, server,
			  countrycode, countryname, u->isaway ? "Y" : "N", u->isaway && u->awaymsg ? u->awaymsg : NULL);

		if (u->sqlid < 1 && db_getnick(u->sqlnick) == -1)
		{
			alog(LOG_DEBUG, "debug: Line 293 : Something went wrong trying to create user %s", u->sqlnick);
		}
	}

	sql_reset_usermodes(u);

	SET_SEGV_LOCATION();

	if (UserCacheTime)
	{
		db_cleanuser();
	}

	SET_SEGV_LOCATION();

	free(username);
	free(host);
	free(server);
	free(realname);
	free(countrycode);
	free(countryname);

	SET_SEGV_LOCATION();

	if (account)
	{
		free(account);
	}

	SET_SEGV_LOCATION();

	if (ircd->vhost && vhost)
	{
		free(vhost);
	}

	SET_SEGV_LOCATION();

	return;
}

/*************************************************************************/

/**
 * Update SQL when the nick has been changed
 *
 * @param newnick is the nick being changed to
 * @param oldnick is the nick being changed from
 *
 * @return void - no returend value
 *
 */
void sql_do_nick_chg(char *newnick, char *oldnick)
{
	int nickid;
	char *sqlnewnick, *sqloldnick;
	User *u;
#ifdef USE_MYSQL
	MYSQL_RES *mysql_res;
	char *uname = NULL;
#endif

	SET_SEGV_LOCATION();

	if (!denora->do_sql)
	{
		return;
	}

	/* NULL parameter checks */
	if (!newnick || !*newnick || !oldnick || !*oldnick)
	{
		return;
	}

	sqloldnick = rdb_escape(oldnick);
	sqlnewnick = rdb_escape(newnick);
	nickid = db_getnick(sqlnewnick);
	u = user_find(oldnick);

	/* the target nickname might already exist if caching is enabled */
	if (UserCacheTime && (strcasecmp(newnick, oldnick)) && nickid)
	{
		/* In this case, we don't keep a record of the old nick. It would be :
		 * - technically difficult, because we'd have to make a copy of the record
		 * - dangerous, because it would provide an easy way to fill up the DB */
		rdb_query(QUERY_HIGH,
			  "DELETE from %s WHERE nickid=%d",
			  UserTable, nickid);
	}

	rdb_query(QUERY_HIGH,
		  "SELECT nick FROM %s WHERE nick = \"%s\";",
		  UserTable, sqlnewnick);
	SET_SEGV_LOCATION();

#ifdef USE_MYSQL
	mysql_res = mysql_store_result(mysql);
	if (mysql_res)
	{
		if (mysql_num_rows(mysql_res) != 0)
		{
			rdb_query(QUERY_HIGH,
				  "DELETE from %s WHERE nick=\'%s\'",
				  UserTable, sqlnewnick);
		}
		mysql_free_result(mysql_res);
	}

	rdb_query(QUERY_HIGH,
		  "UPDATE %s SET nick=\'%s\' WHERE nick=\'%s\'",
		  UserTable, sqlnewnick, sqloldnick);

	SET_SEGV_LOCATION();

	/* we get the current sgroup or uname from aliases */
	if (u->sgroup)
	{
		uname = sstrdup(u->sgroup);
		alog(LOG_DEBUG, "We will use u->sgroup which is %s", uname);
	}
	else
	{
		alog(LOG_DEBUG,
		     "We check if oldnick %s already has a uname in aliases (it should)",
		     oldnick);
		rdb_query(QUERY_HIGH,
			  "SELECT uname FROM %s WHERE nick=\'%s\' ",
			  AliasesTable, sqloldnick);
		mysql_res = mysql_store_result(mysql);
		if (mysql_res)
		{
			if (mysql_num_rows(mysql_res))
			{
				mysql_row = mysql_fetch_row(mysql_res);
				uname = rdb_escape(mysql_row[0]);
				if (u)
				{
					u->sgroup = sstrdup(uname);
				}
				alog(LOG_DEBUG, "Yes indeed, we got uname %s", uname);
			}
			mysql_free_result(mysql_res);
		}
		else
		{
			alog(LOG_DEBUG, "No uname found");
		}
	}

	/* we insert a new alias record */
	rdb_query(QUERY_HIGH,
		  "INSERT INTO %s (nick, uname) VALUES (\'%s\', \'%s\') ON DUPLICATE KEY UPDATE uname=\'%s\'",
		  AliasesTable, sqlnewnick, uname ? uname : sqlnewnick,
		  uname ? uname : sqlnewnick);

	if (uname)
	{
		free(uname);
	}
#endif
	free(sqloldnick);
	free(sqlnewnick);
	return;
}

/*************************************************************************/

/**
 * Allocate a new User structure, fill in basic values, link it to the
 * overall list, and return it.  Always successful.
 *
 * @param nick is nickname of the user to create the struct for
 *
 * @return User struct
 *
 */
User *new_user(const char *nick)
{
	User *user, **list;

	SET_SEGV_LOCATION();
	user = calloc(sizeof(User), 1);
	if (!nick || !*nick)
	{
		nick = "";
	}
	strlcpy(user->nick, nick, NICKMAX);
	list = &userlist[USERHASH(user->nick)];
	user->next = *list;
	if (*list)
	{
		(*list)->prev = user;
	}
	*list = user;
	return user;
}

/*************************************************************************/

/**
 * Change the nickname of a user, and move pointers as necessary.
 *
 * @param user is the old user struct
 * @param nick is nickname of the user to create the struct for
 *
 * @return void - no returend value
 *
 */
static void change_user_nick(User * user, const char *nick)
{
	User **list;

	/* Sanity check to make sure we don't segfault */
	if (!user || !nick || !*nick)
	{
		return;
	}

	SET_SEGV_LOCATION();

	if (user->prev)
	{
		user->prev->next = user->next;
	}
	else
	{
		userlist[USERHASH(user->nick)] = user->next;
	}
	if (user->next)
	{
		user->next->prev = user->prev;
	}
	*user->nick = '\0';	  /* paranoia for zero-length nicks */
	strlcpy(user->nick, nick, NICKMAX);
	list = &userlist[USERHASH(user->nick)];
	user->next = *list;
	user->prev = NULL;
	if (*list)
	{
		(*list)->prev = user;
	}
	*list = user;
}

/*************************************************************************/

/**
 * Update internal and SQL on CHGHOST or SETHOST like commands
 *
 * @param source is the person we are setting the host for
 * @param host is the new virtual host
 *
 * @return void - no returend value
 *
 */
void change_user_host(char *source, char *host)
{
	User *user;
	char db[MAX_SQL_BUF];
	char *sqlvhost;

	user = user_find(source);
	if (!user)
	{
		alog(LOG_NONEXISTANT, langstr(ALOG_VHOST_NONEXISTANT_USER),
		     source);
		return;
	}

	SET_SEGV_LOCATION();

	if (user->vhost)
	{
		free(user->vhost);
	}
	user->vhost = sstrdup(host);

	alog(LOG_DEBUG, langstr(ALOG_VHOST_FOR), user->nick, host);

	if (denora->do_sql)
	{
		if (user->sqlid > 0 || db_getnick(user->sqlnick) != -1)
		{
			*db = '\0';
			ircsnprintf(db, sizeof(db), "UPDATE %s SET", UserTable);
			SET_SEGV_LOCATION();
			if (ircd->vhostchar)
			{
				if(ircd->vhostchar2)
				{
					ircsnprintf(&db[strlen(db)], sizeof(db), " mode_l%c=\'Y\',", ircd->vhostchar2);
				}
				ircsnprintf(&db[strlen(db)], sizeof(db), " mode_l%c=\'Y\',", ircd->vhostchar);
			}
			sqlvhost = rdb_escape(user->vhost);
			ircsnprintf(&db[strlen(db)], sizeof(db), " hiddenhostname=\'%s\' WHERE nickid=%d", sqlvhost, user->sqlid);
			rdb_query(QUERY_LOW, db);
			free(sqlvhost);
		}
	}

	SET_SEGV_LOCATION();

	return;
}

/*************************************************************************/

/**
 * Update internal and SQL on CHGNAME or SETNAME like commands
 *
 * @param source is the person we are setting the host for
 * @param host is the new virtual realname
 *
 * @return void - no returend value
 *
 */
void change_user_realname(char *source, char *realname)
{
	User *user;
	char *sqlrealname;

	SET_SEGV_LOCATION();

	if (!realname || !*realname || !source || !*source)
	{
		return;
	}

	user = user_find(source);
	if (!user)
	{
		alog(LOG_NONEXISTANT, langstr(ALOG_CHGNAME_NONEXISTANT_USER),
		     source);
		return;
	}

	SET_SEGV_LOCATION();

	if (user->realname)
	{
		free(user->realname);
	}
	user->realname = sstrdup(realname);

	alog(LOG_DEBUG, langstr(ALOG_REALNAME_FOR), user->nick, realname);

	if (denora->do_sql && (user->sqlid > 0 || db_getnick(user->sqlnick) != -1))
	{
		sqlrealname = rdb_escape(user->realname);
		rdb_query(QUERY_LOW,
			  "UPDATE %s SET realname=\'%s\' WHERE nickid=%d",
			  UserTable, sqlrealname, user->sqlid);
		free(sqlrealname);
	}

	return;
}

/*************************************************************************/

/**
 * Update internal and SQL on SETIDENT or CHGIDENT like commands
 *
 * @param source is the person we are setting the host for
 * @param host is the new virtual ident
 *
 * @return void - no returend value
 *
 */
void change_user_username(char *source, char *username)
{
	User *user;
	char *sqlusername;

	SET_SEGV_LOCATION();

	if (!username || !*username || !source || !*source)
	{
		return;
	}

	user = user_find(source);
	if (!user)
	{
		alog(LOG_NONEXISTANT, langstr(ALOG_CHGIDENT_NONEXISTANT_USER),
		     source);
			return;
	}

	if (user->username)
	{
		free(user->username);
	}
	user->username = sstrdup(username);

	alog(LOG_DEBUG, langstr(ALOG_USERNAME_FOR), user->nick, username);

	if (denora->do_sql && (user->sqlid < 1 || db_getnick(user->sqlnick) != -1))
	{
		sqlusername = rdb_escape(user->username);
		rdb_query(QUERY_LOW,
			  "UPDATE %s SET username=\'%s\' WHERE nickid=%d",
			  UserTable, sqlusername, user->sqlid);
		free(sqlusername);
	}

	return;
}

/*************************************************************************/

/**
 * Delete user from the hash
 *
 * @param user is the user struct to remove
 *
 * @return void - no returend value
 *
 */
void delete_user(User * user)
{
	struct u_chanlist *c, *c2;
	struct u_modes *m, *m2;
	PrivMsg *p;
	Uid *ud;

	SET_SEGV_LOCATION();

	if (!user)
	{
		return;
	}
	send_event(EVENT_DELETE_USER, 1, user->nick);
	del_current_user(user->server);

	alog(LOG_EXTRADEBUG, "debug: delete_user() called for %s", user->nick);
	alog(LOG_EXTRADEBUG, "debug: delete_user(): remove from channels");
	c = user->chans;
	while (c)
	{
		if ((c2 = c->next))
		{
			chan_deluser(user, c->chan);
			free(c);
			c = c2;
		}
		else
		{
			break;
		}
	}
	if (c)
	{
		chan_deluser(user, c->chan);
		free(c);
	}

	if (is_oper(user))
	{
		alog(LOG_EXTRADEBUG, "debug: delete_user(): update oper count");
		del_oper_count(user);
	}
	
	alog(LOG_EXTRADEBUG, "debug: delete_user(): remove usermodes");
	m = user->modes;
	while (m)
	{
		if ((m2 = m->next))
		{
			if (m->mode)
			{
				free(m->mode);
			}
			free(m);
			m = m2;
		}
		else
		{
			break;
		}
	}
	if (m)
	{
		if (m->mode)
		{
			free(m->mode);
		}
		free(m);
	}

	alog(LOG_EXTRADEBUG, "debug: delete_user(): free founder data");

	alog(LOG_EXTRADEBUG, "debug: delete_user(): cleanups");
	if (user->isservice)
	{
		p = findPrivMsg(user->nick);
		delPrivMsg(p);
	}

	if (user->country_code)
	{
		tld_update(user->country_code);
		/* free(user->country_code); */
	}

	if (CTCPUsers && user->ctcp)
	{
		ctcp_update(user->ctcp);
	}

	alog(LOG_EXTRADEBUG, "debug: delete_user(): conditional free");
	if (user->vhost)
	{
		free(user->vhost);
	}

	if (user->uid)
	{
		free(user->uid);
	}

	if (user->account)
	{
		free(user->account);
	}

	if (user->sgroup)
	{
		free(user->sgroup);
	}

	if (user->lastuname)
	{
		free(user->lastuname);
	}

	if (user->country_name)
	{
		/* free(user->country_name); */
	}

	if (user->ctcp)
	{
		free(user->ctcp);
	}

	if (user->vident)
	{
		free(user->vident);
	}

	if (user->awaymsg)
	{
		free(user->awaymsg);
	}

	if (user->swhois)
	{
		free(user->swhois);
	}

	alog(LOG_EXTRADEBUG, "debug: delete_user(): unconditional free");
	free(user->username);
	free(user->host);
	free(user->realname);
	free(user->ip);
	free(user->sqlnick);

	if (ircd->p10 || (UseTS6 && ircd->ts6))
	{
		ud = find_uid(user->nick);
		if (ud)
		{
			delete_uid(ud);
		}
	}

	alog(LOG_EXTRADEBUG, "debug: delete_user(): update listpointers");
	if (user->prev)
	{
		user->prev->next = user->next;
	}
	else
	{
		userlist[USERHASH(user->nick)] = user->next;
	}
	if (user->next)
	{
		user->next->prev = user->prev;
	}

	alog(LOG_EXTRADEBUG, "debug: delete_user(): final free");
	free(user);

	return;
}

/*************************************************************************/

/**
 * Find a user by nick.  Return NULL if user could not be found.
 *
 * @param nick is the nickname to locate the struct of
 *
 * @return User struct
 *
 */
User *finduser(const char *nick)
{
	User *user;

	SET_SEGV_LOCATION();

	if (!nick || !*nick)
	{
		alog(LOG_DEBUG, "Error: finduser() called with NULL values");
		do_backtrace(0);
		return NULL;
	}

	alog(LOG_EXTRADEBUG, "debug: finduser(%p)", nick);
	user = userlist[USERHASH(nick)];
	while (user && stricmp(user->nick, nick) != 0)
	{
		user = user->next;
	}
	alog(LOG_EXTRADEBUG, "debug: finduser(%s) -> 0x%p", nick, (void *) user);
	return user;
}

/*************************************************************************/

/**
 * Iterate over all users in the user list.  Return NULL at end of list.
 *
 * @return User struct
 *
 */
User *firstuser(void)
{
	next_index = 0;

	SET_SEGV_LOCATION();

	while (next_index < MAX_USERS && current == NULL)
	{
		current = userlist[next_index++];
	}
	alog(LOG_EXTRADEBUG, "debug: User Hash index %d", next_index);
	alog(LOG_EXTRADEBUG, "debug: firstuser() returning %s",
	     current ? current->nick : "NULL (end of list)");

	return current ? current : NULL;
}

/*************************************************************************/

/**
 * Move to the next user in the list
 *
 * @return User struct
 *
 */
User *nextuser(void)
{
	SET_SEGV_LOCATION();

	if (current)
	{
		current = current->next;
	}
	if (!current && next_index < MAX_USERS)
	{
		while (next_index < MAX_USERS && current == NULL)
		{
			current = userlist[next_index++];
		}
	}
	alog(LOG_EXTRADEBUG, "debug: User Hash index %d", next_index);
	alog(LOG_EXTRADEBUG, "debug: nextuser() returning %s",
	     current ? current->nick : "NULL (end of list)");
	return current;
}

/*************************************************************************/

/**
 * Handle NICK commands from the ircd
 *
 * @param source is the orgin of the mssage
 * @param nick is the nick to handle (if change this is the new nick)
 * @param username is the ident
 * @param host is the host
 * @param server is the server the user is on
 * @param realname is the realname (geos) of the user
 * @param ts is the time stamp for the event (either change or connect time)
 * @param svid is the service time stamp if any
 * @param ip if nickip the ip in int32 fashion
 * @param vhost if vhost on nick it is passed now
 * @param uid if the ircd uses uid for the nick it is passed now
 * @param hopcount is the hop count
 * @param modes is the user modes if they are sent on nick
 * @param account is the user account, if sent on nick (P10 ircds only)
 *
 * @return void - no returend value
 *
 */
User *do_nick(const char *source, char *nick, char *username, char *host,
	      char *server, char *realname, time_t ts, uint32 svid,
	      char *ipchar, char *vhost, char *uid, int hopcount,
	      char *modes, char *account)
{

	User *user = NULL;
	char *newav[5];
	char *country_code = NULL;
	char *country_name = NULL;
	Server *s = server_find(server);
	int country_id;
	TLD *tld;
	Uid *ud;
	char *temp;

	SET_SEGV_LOCATION();

	if (!*source)
	{
		if (!LargeNet)
		{
			if (host && !stricmp("localhost", host))
			{
				if (s && s->countrycode)
				{
					country_code = sstrdup(s->countrycode);
				}
				else
				{
					country_code = sstrdup("local");
				}
				if (s && s->country)
				{
					country_name = sstrdup(s->country);
				}
				else
				{
					country_name = sstrdup("localhost");
				}
			}
			else
			{
				if (strstr(ipchar,":") != NULL)
				{
					country_id = GeoIP_id_by_addr_v6(gidb_v6, ipchar);
				}
				else
				{
					country_id = GeoIP_id_by_addr(gidb, ipchar);
				}

				if (country_id > 0)
				{
					country_code = (char *)GeoIP_code_by_id(country_id);
					country_name = (char *)GeoIP_name_by_id(country_id);
				}
			}
		}
		if (!country_name)
		{
			country_name = sstrdup("Unknown");
		}
		if (!country_code)
		{
			country_code = sstrdup("??");
		}

		user = new_user(nick);
		user->country_name = country_name;
		user->country_code = country_code;

		/* Allocate User structure and fill it in. */
		user->username = (username ? sstrdup(username) : NULL);
		user->host = (host ? sstrdup(host) : NULL);
		user->server = (s ? s : me_server);
		add_current_user(user->server);
		user->realname = sstrdup((realname ? realname : ""));
		user->timestamp = ts;
		user->my_signon = time(NULL);
		user->vhost = (vhost ? sstrdup(vhost) : NULL);
		user->uid = (uid ? sstrdup(uid) : NULL);
		user->account = (account ? sstrdup(account) : NULL);
		user->admin = 0;	/* 0 by default, winner, eh? */
		user->hopcount = hopcount;
		user->ip = sstrdup(ipchar);
		user->cstats = 0;
		user->sgroup = NULL;
		user->lastuname = NULL;
		user->language = StatsLanguage;
		user->moduleData = NULL;

		if (ircd->p10 || (UseTS6 && ircd->ts6))
		{
			if (uid)
			{
				new_uid(user->nick, user->uid);
			}
		}

		if (!LargeNet)
		{
			tld = do_tld(user->country_name, user->country_code);
			if (denora->do_sql)
			{
				if (ircd->syncstate || UplinkSynced)
				{
					sql_do_tld(UPDATE, user->country_code,
						   user->country_name, tld->count,
						   tld->overall);
				}
			}
		}

		user->svid = (svid == (uint32) ts ? svid : 1);
		user->sqlnick = rdb_escape(user->nick);
		send_event(EVENT_NEWNICK, 1, user->nick);
		SET_SEGV_LOCATION();
		if (denora->do_sql)
		{
			sql_do_nick(user);
			if (modes)
			{
				sql_do_usermodes(user, modes);
			}
		}
		alog(LOG_EXTRADEBUG, "debug: CTCP settings LargeNet=%d CTCPUsers=%d CTCPUsersEOB=%d, UplinkSynced=%d", LargeNet, CTCPUsers, CTCPUsersEOB, UplinkSynced);

		if (!LargeNet && CTCPUsers && stricmp(s_StatServ, user->nick))
		{
			if ((CTCPUsersEOB && UplinkSynced) || !CTCPUsersEOB)
			{
				denora_cmd_privmsg(s_StatServ_alias ? s_StatServ_alias
						   : s_StatServ, user->nick,
						   "\1VERSION\1");
				send_event(EVENT_SENT_CTCP_VERSION, 1, user->nick);
			}
		}
		if (modes)
		{
			newav[0] = sstrdup(modes);
			denora_set_umode(user, 1, newav);
			free(newav[0]);
		}
	}
	else
	{
		user = user_find(source);
		if (!user)
		{
			return NULL;
		}
		user->timestamp = ts;

		if (stricmp(nick, user->nick))
		{
			if (user->sgroup)
			{
				if (user->lastuname)
				{
					free(user->lastuname);
				}
				user->lastuname = sstrdup(user->sgroup);	/* in case we need to merge later */
			}
			if (ircd->p10 || (UseTS6 && ircd->ts6))
			{
				ud = find_nickuid(user->uid);
				if (ud)
				{
					temp = sstrdup(ud->uid);
					delete_uid(ud);
					new_uid(nick, temp);
					free(temp);
				}
				else
				{
					alog(LOG_DEBUG, "find_nickuid: returned NULL");
				}
			}
			alog(LOG_DEBUG, "debug: %s has changed nicks to %s", source, nick);
			sql_do_nick_chg(nick, user->nick);
			change_user_nick(user, nick);
			send_event(EVENT_CHANGE_NICK, 2, source, nick);
			user->sqlnick = rdb_escape(user->nick);
		}
		else
		{
			user->my_signon = time(NULL);
		}

	}
	SET_SEGV_LOCATION();
	return user;
}

/*************************************************************************/

/**
 * Handle UMODE messages from the ircd
 *
 * @param source is the user whom just changed modes
 * @param ac is the array count
 * @param av is the array
 *
 * @return void - no returend value
 *
 */
void do_umode(const char *source, int ac, char **av)
{
	User *user;

	SET_SEGV_LOCATION();

	if (!source || !*source)
	{
		return;
	}

	if (ac < 2)
	{
		return;
	}

	user = user_find(source);
	if (!user)
	{
		alog(LOG_NONEXISTANT, langstr(ALOG_UMODE_NONEXISTANT),
		     av[1], source, merge_args(ac, av));
		return;
	}

	denora_set_umode(user, ac - 1, &av[1]);
	if (denora->do_sql)
	{
		sql_do_usermodes(user, av[1]);
	}

	return;
}

/*************************************************************************/

/**
 * Handle SVSMODE messages from the ircd
 *
 * @param ac is the array count
 * @param av is the array
 *
 * @return void - no returend value
 *
 */
void do_svsumode(int ac, char **av)
{
	User *user;

	SET_SEGV_LOCATION();

	if (ac < 2)
	{
		return;
	}

	user = user_find(av[0]);
	if (!user)
	{
		alog(LOG_NONEXISTANT, langstr(ALOG_UMODE_NONEXISTANT),
		     av[1], av[0], merge_args(ac, av));
		return;
	}

	if (ircdcap->tsmode)
	{
		if (denora->capab & ircdcap->tsmode || UseTSMODE)
		{
			denora_set_umode(user, ac - 1, &av[2]);
			if (denora->do_sql)
			{
				sql_do_usermodes(user, av[2]);
			}
		}
		else
		{
			denora_set_umode(user, ac - 1, &av[1]);
			if (denora->do_sql)
			{
				sql_do_usermodes(user, av[1]);
			}
		}
	}
	else
	{
		denora_set_umode(user, ac - 1, &av[1]);
		if (denora->do_sql)
		{
			sql_do_usermodes(user, av[1]);
		}
	}

	return;
}

/*************************************************************************/

/**
 * Handle UMODE2 messages from the ircd
 *
 * @param source is the user whom just changed modes
 * @param ac is the array count
 * @param av is the array
 *
 * @return void - no returend value
 *
 */
void do_umode2(const char *source, int ac, char **av)
{
	User *user;

	SET_SEGV_LOCATION();

	if (!source || !*source)
	{
		return;
	}

	if (ac < 1)
	{
		return;
	}

	user = user_find(source);
	if (!user)
	{
		alog(LOG_NONEXISTANT, langstr(ALOG_UMODE_NONEXISTANT),
		     av[0], source, merge_args(ac, av));
		return;
	}
	SET_SEGV_LOCATION();

	denora_set_umode(user, ac, &av[0]);
	if (denora->do_sql)
	{
		sql_do_usermodes(user, av[0]);
	}

	return;
}

/*************************************************************************/

/**
 * Handle QUIT messages from the ircd
 *
 * @param nick is the user whom just quit
 * @param msg is the reason they quit
 *
 * @return void - no returend value
 *
 */
void do_quit(const char *source, int ac, char **av)
{
	User *user;

	SET_SEGV_LOCATION();

	if (!source || !*source)
	{
		return;
	}

	if (ac < 1)
	{
		return;
	}

	user = user_find(source);
	if (!user)
	{
		alog(LOG_NONEXISTANT, "user: QUIT from nonexistent user %s: %s",
		     source, merge_args(ac, av));
		return;
	}
	alog(LOG_DEBUG, "debug: %s quits (%s)", user->nick, (!BadPtr(av[0]) ? av[0] : "Quit"));

	if (denora->do_sql)
	{
		db_removenick(user->sqlnick, (!BadPtr(av[0]) ? av[0] : (char *) "Quit"));
		if (UserCacheTime)
		{
			db_cleanuser();
		}
	}
	send_event(EVENT_USER_LOGOFF, 2, user->nick, (!BadPtr(av[0]) ? av[0] : "Quit"));
	delete_user(user);

	return;
}

/*************************************************************************/

/**
 * Handle KILL messages from the ircd
 *
 * @param nick is the user whom just got killed
 * @param msg is the reason they got killed
 *
 * @return void - no returend value
 *
 */
void do_kill(char *nick, char *msg)
{
	User *user;

	if (!nick || !*nick || !msg || !*msg)
	{
		return;
	}

	user = user_find(nick);

	SET_SEGV_LOCATION();

	if (!user)
	{
		alog(LOG_NONEXISTANT, "debug: KILL of nonexistent nick: %s", nick);
		return;
	}
	alog(LOG_DEBUG, "debug: %s killed: %s", nick, msg);
	send_event(EVENT_USER_LOGOFF, 2, user->nick, msg);
	delete_user(user);

	return;
}

/*************************************************************************/
/* Used by TS6 ircds
 * Received: :00Z ENCAP * SU 002AAACOV :Pryan
 * Received: :00Z ENCAP * SU :002AAACOV
 */
void do_account(User * user, char *account)
{
	char *sqlaccount;

	if (!user)
	{
		alog(LOG_NONEXISTANT, "debug: ACCOUNT %s on nonexistent nick", account);
		return;
	}

	if (user->account)
	{
		free(user->account);
	}

	if (user->sqlid < 1 && db_getnick(user->sqlnick) == -1)
	{
		alog(LOG_NONEXISTANT, "ACCOUNT set for nonexistent user %s", user);
		return;
	}

	if (account)
	{
		alog(LOG_DEBUG, "debug: account %s set on %s", account, user->nick);
		user->account = sstrdup(account);
	}
	else
	{
		alog(LOG_DEBUG, "debug: account removed from %s", user->nick);
		user->account = NULL;
	}
	if (denora->do_sql)
	{
		sqlaccount = rdb_escape(user->account);
		rdb_query(QUERY_LOW,
			  "UPDATE %s SET account=\'%s\' WHERE nickid=%d",
			  UserTable, sqlaccount, user->sqlid);
		free(sqlaccount);
	}

	return;
}

/**
 * Handle ACCOUNT messages from the ircd (p10 only)
 *
 * @param nick is the user who the account is being set on
 * @param account is the account name
 * @param flag is 1 if the account is to be removed, or 3 if the account is to be renamed.
 *
 * @return void - no returend value
 *
 */
void do_p10account(User * user, char *account, int flag)
{
	char hhostbuf[255];
	char *sqlaccount, *sqlvhost;

	if (flag != 1 && (!account || !*account))
	{
		return;
	}

	SET_SEGV_LOCATION();

	if (!user)
	{
		alog(LOG_NONEXISTANT, "debug: ACCOUNT %s on nonexistent nick", account);
		return;
	}

	if (flag == 0)
	{
		alog(LOG_DEBUG, "debug: account %s set on %s", account, user->nick);
		user->account = sstrdup(account);
		if (!user->vhost && UserHasMode(user->nick, UMODE_x))
		{
			ircsnprintf(hhostbuf, sizeof(hhostbuf), "%s%s%s",
				    HiddenPrefix, account, HiddenSuffix);
			alog(LOG_DEBUG, "debug: setting vhost %s on %s", hhostbuf, user->nick);
			user->vhost = sstrdup(hhostbuf);
		}
	}
	else if (flag == 1)
	{
		alog(LOG_DEBUG, "debug: account removed from %s", user->nick);
		free(user->account);
		user->account = NULL;
		if (user->vhost)
		{
			alog(LOG_DEBUG, "debug: removing vhost from %s", user->nick);
			free(user->vhost);
		}
	}
	else if (flag == 2)
	{
		alog(LOG_DEBUG, "debug: account %s renaming on %s", account, user->nick);
		free(user->account);
		free(user->vhost);
		user->vhost = NULL;
		user->account = sstrdup(account);
		if (UserHasMode(user->nick, UMODE_x))
		{
			ircsnprintf(hhostbuf, sizeof(account) + sizeof(hhostbuf) + 2,
				    "%s%s%s", HiddenPrefix, account, HiddenSuffix);
			alog(LOG_DEBUG, "debug: setting vhost %s on %s", hhostbuf, user->nick);
			user->vhost = sstrdup(hhostbuf);
		}
	}

	SET_SEGV_LOCATION();

	if (denora->do_sql)
	{
		if (user->sqlid < 1 && db_getnick(user->sqlnick) == -1)
		{
			alog(LOG_NONEXISTANT, "ACCOUNT set for nonexistent user %s", user);
			return;
		}
		sqlaccount = rdb_escape(account);
		sqlvhost = rdb_escape(user->vhost);
		rdb_query(QUERY_LOW,
			  "UPDATE %s SET account=\'%s\', hiddenhostname=\'%s\' WHERE nickid=%d",
			  UserTable, sqlaccount, sqlvhost, user->sqlid);
		free(sqlaccount);
		free(sqlvhost);
	}

	SET_SEGV_LOCATION();

	return;
}

/*************************************************************************/

/**
 * Check if the given user is a ircop +o
 *
 * @param user is the user struct to check
 *
 * @return 1 if is oper, 0 if they are not
 *
 */
int is_oper(User * user)
{
	SET_SEGV_LOCATION();
	return user ? UserHasMode(user->nick, denora_get_oper_mode()) : 0;
}

/*************************************************************************/

/**
 * Get the vhost for the user, if set else return the host, on ircds without
 * vhost this returns the host
 *
 * @param u user to get the vhost for
 *
 * @return vhost
 */
char *common_get_vhost(User * u)
{
	SET_SEGV_LOCATION();

	if (!u)
	{
		return NULL;
	}
	if (ircd->vhostmode && (u->mode & ircd->vhostmode) && u->vhost)
	{
		return u->vhost;
	}
	else
	{
		return u->host;
	}
}

/*************************************************************************/

/**
 * Get the vident for the user, if set else return the ident, on ircds without
 * vident this returns the ident
 *
 * @param u user to get info the vident for
 *
 * @return vident
 */
char *common_get_vident(User * u)
{
	SET_SEGV_LOCATION();

	if (!u)
	{
		return NULL;
	}
	if (ircd->vhostmode && (u->mode & ircd->vhostmode) && u->vident)
	{
		return u->vident;
	}
	else
	{
		return u->username;
	}
}

/*************************************************************************/

User *user_find(const char *source)
{
	User *user = NULL;

	if (!source)
	{
		return NULL;
	}
	alog(LOG_EXTRADEBUG, "user_find(%s)", source);

	if (ircd->p10 || (UseTS6 && ircd->ts6))
	{
		user = find_byuid(source);
		return user ? user : finduser(source);
	}
	else
	{
		return finduser(source);
	}
}

/*************************************************************************/
