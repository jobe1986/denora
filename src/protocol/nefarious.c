/* Nefarious ircu IRCD functions
 *
 * (C) 2004-2006 Denora Team
 * Contact us at info@nomadirc.net
 *
 * Please read COPYING and README for furhter details.
 *
 * Based on the original code of Anope by Anope Team.
 * Based on the original code of Thales by Lucas.
 *
 * $Id$
 *
 */

#include "denora.h"
#include "nefarious.h"

int p10nickcnt = 0;

IRCDVar myIrcd[] = {
    {"Nefarious IRCu 0.4.0",    /* ircd name                 */
     "+iok",                    /* StatServ mode             */
     IRCD_DISABLE,              /* Vhost                     */
     IRCD_DISABLE,              /* Supports SGlines          */
     IRCD_DISABLE,              /* sgline sql table          */
     IRCD_ENABLE,               /* Supports SQlines          */
     IRCD_DISABLE,              /* sqline sql table          */
     IRCD_DISABLE,              /* Supports SZlines          */
     IRCD_DISABLE,              /* Has exceptions +e         */
     IRCD_DISABLE,              /* vidents                   */
     IRCD_ENABLE,               /* NICKIP                    */
     IRCD_DISABLE,              /* VHOST ON NICK             */
     IRCD_DISABLE,              /* +f                        */
     IRCD_DISABLE,              /* +j                        */
     IRCD_DISABLE,              /* +L                        */
     IRCD_DISABLE,              /* +f Mode                   */
     IRCD_DISABLE,              /* +j Mode                   */
     IRCD_DISABLE,              /* +L Mode                   */
     NULL,                      /* CAPAB Chan Modes          */
     IRCD_DISABLE,              /* We support TOKENS         */
     IRCD_DISABLE,              /* TOKENS are CASE Sensitive */
     IRCD_DISABLE,              /* TIME STAMPS are BASE64    */
     IRCD_DISABLE,              /* +I support                */
     IRCD_DISABLE,              /* SJOIN ban char            */
     IRCD_DISABLE,              /* SJOIN except char         */
     IRCD_DISABLE,              /* SJOIN invite char         */
     IRCD_DISABLE,              /* umode for vhost           */
     IRCD_DISABLE,              /* owner                     */
     IRCD_DISABLE,              /* protect                   */
     IRCD_ENABLE,               /* halfop                    */
     NULL,                      /* User modes                */
     NULL,                      /* Channel modes             */
     IRCD_DISABLE,              /* flood                     */
     IRCD_DISABLE,              /* flood other               */
     IRCD_DISABLE,              /* vhost                     */
     IRCD_DISABLE,              /* vhost other               */
     IRCD_DISABLE,              /* channek linking           */
     IRCD_ENABLE,               /* p10                       */
     IRCD_DISABLE,              /* TS6                       */
     IRCD_ENABLE,               /* numeric ie.. 350 etc      */
     IRCD_DISABLE,              /* channel mode gagged       */
     IRCD_DISABLE,              /* spamfilter                */
     'b',                       /* ban char                  */
     IRCD_DISABLE,              /* except char               */
     IRCD_DISABLE,              /* invite char               */
     IRCD_DISABLE,              /* zip                       */
     IRCD_DISABLE,              /* ssl                       */
     IRCD_ENABLE,               /* uline                     */
     NULL,                      /* nickchar                  */
     IRCD_DISABLE,              /* svid                      */
     IRCD_DISABLE,              /* hidden oper               */
     IRCD_ENABLE,               /* extra warning             */
     IRCD_ENABLE                /* Report sync state         */
     },
};

IRCDCAPAB myIrcdcap[] = {
    {
     0,                         /* NOQUIT       */
     0,                         /* TSMODE       */
     0,                         /* UNCONNECT    */
     0,                         /* NICKIP       */
     0,                         /* SJOIN        */
     0,                         /* ZIP          */
     0,                         /* BURST        */
     0,                         /* TS5          */
     0,                         /* TS3          */
     0,                         /* DKEY         */
     0,                         /* PT4          */
     0,                         /* SCS          */
     0,                         /* QS           */
     0,                         /* UID          */
     0,                         /* KNOCK        */
     0,                         /* CLIENT       */
     0,                         /* IPV6         */
     0,                         /* SSJ5         */
     0,                         /* SN2          */
     0,                         /* TOKEN        */
     0,                         /* VHOST        */
     0,                         /* SSJ3         */
     0,                         /* NICK2        */
     0,                         /* UMODE2       */
     0,                         /* VL           */
     0,                         /* TLKEXT       */
     0,                         /* DODKEY       */
     0,                         /* DOZIP        */
     0,
     0,
     0}
};

/*************************************************************************/

void IRCDModeInit(void)
{
    /* User Modes
     * a - admin
     * h - set host
     * f - fake host
     * n - no chan
     * i - marks a users as invisible;
     * s - marks a user for receipt of server notices;
     * w - user receives wallops;
     * o - operator flag.
     * d - Deaf & Dumb.  This user will not get any channel traffic. Used for bots.
     * k - This user cannot be kicked, deop'd or /kill'd.  This usermode may only
     *     be set by a server, it may not be set by a user.  This is used by
     *     undernet service bots (X/W/UWorld etc)
     * g - List channel HACK:'s
     * O - local op
     * R - account only
     * B - bot
     * X - xtra op
     * I - no idle
     * W - whois
     */

    ModuleSetUserMode(UMODE_a, IRCD_ENABLE);
    ModuleSetUserMode(UMODE_d, IRCD_ENABLE);
    ModuleSetUserMode(UMODE_f, IRCD_ENABLE);
    ModuleSetUserMode(UMODE_g, IRCD_ENABLE);
    ModuleSetUserMode(UMODE_h, IRCD_ENABLE);
    ModuleSetUserMode(UMODE_i, IRCD_ENABLE);
    ModuleSetUserMode(UMODE_k, IRCD_ENABLE);
    ModuleSetUserMode(UMODE_n, IRCD_ENABLE);
    ModuleSetUserMode(UMODE_o, IRCD_ENABLE);
    ModuleSetUserMode(UMODE_r, IRCD_ENABLE);
    ModuleSetUserMode(UMODE_s, IRCD_ENABLE);
    ModuleSetUserMode(UMODE_x, IRCD_ENABLE);
    ModuleSetUserMode(UMODE_w, IRCD_ENABLE);
    ModuleSetUserMode(UMODE_B, IRCD_ENABLE);
    ModuleSetUserMode(UMODE_I, IRCD_ENABLE);
    ModuleSetUserMode(UMODE_O, IRCD_ENABLE);
    ModuleSetUserMode(UMODE_R, IRCD_ENABLE);
    ModuleSetUserMode(UMODE_W, IRCD_ENABLE);
    ModuleSetUserMode(UMODE_X, IRCD_ENABLE);
    ModuleUpdateSQLUserMode();

    /* Channel Modes
     * o - give/take channel operator privileges;
     * h - halfop
     * p - private channel flag;
     * s - secret channel flag;
     * i - invite-only channel flag;
     * t - topic settable by channel operator only flag;
     * n - no messages to channel from clients on the outside;
     * m - moderated channel;
     * l - set the user limit to channel;
     * b - set a ban mask to keep users out;
     * v - give/take the ability to speak on a moderated channel;
     * k - set a channel key (password).
     * r - registered
     * e - ban excepts
     * c - no colour
     * S - strip colour
     * C - no ctcp
     * M - acconly
     * N - no notice
     * O - oper only
     * a - admin only
     * Q - no quit parts
     * Z - ssl only
     * T - no amsg
     * L - no list modes
     * z - persist
     */
    CreateChanBanMode(CMODE_b, add_ban, del_ban);
    CreateChanBanMode(CMODE_e, add_exception, del_exception);
    CreateChanMode(CMODE_a, NULL, NULL);
    CreateChanMode(CMODE_c, NULL, NULL);
    CreateChanMode(CMODE_i, NULL, NULL);
    CreateChanMode(CMODE_k, set_key, get_key);
    CreateChanMode(CMODE_l, set_limit, get_limit);
    CreateChanMode(CMODE_m, NULL, NULL);
    CreateChanMode(CMODE_n, NULL, NULL);
    CreateChanMode(CMODE_p, NULL, NULL);
    CreateChanMode(CMODE_r, NULL, NULL);
    CreateChanMode(CMODE_s, NULL, NULL);
    CreateChanMode(CMODE_t, NULL, NULL);
    CreateChanMode(CMODE_z, NULL, NULL);
    CreateChanMode(CMODE_C, NULL, NULL);
    CreateChanMode(CMODE_L, NULL, NULL);
    CreateChanMode(CMODE_M, NULL, NULL);
    CreateChanMode(CMODE_N, NULL, NULL);
    CreateChanMode(CMODE_O, NULL, NULL);
    CreateChanMode(CMODE_Q, NULL, NULL);
    CreateChanMode(CMODE_S, NULL, NULL);
    CreateChanMode(CMODE_T, NULL, NULL);
    CreateChanMode(CMODE_Z, NULL, NULL);
    ModuleSetChanUMode('v', 'v', STATUS_VOICE);
    ModuleSetChanUMode('h', 'h', STATUS_HALFOP);
    ModuleSetChanUMode('o', 'o', STATUS_OP);
    ModuleUpdateSQLChanMode();
}


char *nefarious_nickip(char *host)
{
    struct in_addr addr;
    int decoded;

    decoded = base64toIP(host);
    addr.s_addr = ntohl(decoded);
    return sstrdup(inet_ntoa(addr));
}

/* On Services connect the modes are given */
/* AB N Trystan` 1 1098031638 tslee comcast.net +i AYAmXj ABAAC : real */
/* On user join they are not */
/* AB N Trystan  1 1101443066 tslee comcast.net AYAmXj ABAAB : real */

/*
** NICK - new
**      source  = Ownering Server Prefix
**	    parv[0] = nickname
**      parv[1] = hopcount
**      parv[2] = timestamp
**      parv[3] = username
**      parv[4] = host
**      parv[5] = modes
**      parv[6] = base64 ip
**	    parv[7] = uid
**      parv[8] = info
** NICK - change 
**      source  = oldnick
**	parv[0] = new nickname
**      parv[1] = timestamp
*/
/* 
  AB N Trystan 1 1117327797 tslee c-24-2-101-227.hsd1.ut.comcast.net +i AYAmXj ABAAB :Dreams are answers to questions not yet asked
        0      1  2          2      3                                 4 5       6     7
 */

int denora_event_nick(char *source, int ac, char **av)
{
    User *user;
    Server *s;
    char *temp;
    char *ipchar;

    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    /* do_nick(const char *source, char *nick, char *username, char *host,
       char *server, char *realname, time_t ts, uint32 svid,
       uint32 ip, char *vhost, char *uid, int hopcount, char *modes) */

    temp = sstrdup(source);

    if (ac != 2) {
        s = server_find(source);
        *source = '\0';
        if (ac == 10) {
            ipchar = nefarious_nickip(av[7]);
            user =
                do_nick(source, av[0], av[3], av[4], (s ? s->name : temp),
                        av[9], strtoul(av[2], NULL, 10), 0,
                        ipchar, NULL, av[8], strtoul(av[1],
                                                     NULL, 10), av[5]);
            free(ipchar);
        } else if (ac == 9) {
            ipchar = nefarious_nickip(av[6]);
            user =
                do_nick(source, av[0], av[3], av[4], (s ? s->name : temp),
                        av[8], strtoul(av[2], NULL, 10), 0, ipchar, NULL,
                        av[7], strtoul(av[1], NULL, 10), av[5]);
            free(ipchar);
        } else if (ac == 8) {
            /* no user modes   */
            /* Source AB       
               av[0] = Trystan 
               av[1] = 1       
               av[2] = 1144080460 
               av[3] = tslee
               av[4] = c-67-186-230-12.hsd1.ut.comcast.net
               av[5] = BDuuYM
               av[6] = ABAAB
               av[7] = Dreams are answers to questions not yet asked
             */
            ipchar = nefarious_nickip(av[5]);
            do_nick(source, av[0], av[3], av[4], (s ? s->name : temp),
                    av[7], strtoul(av[2], NULL, 10), 0,
                    ipchar, NULL, av[6], strtoul(av[1], NULL, 10), NULL);
            free(ipchar);
        } else {
            alog(LOG_DEBUG,
                 "Unknown NICK formatted message please report the following");
            protocol_debug(temp, ac, av);
        }
    } else {
        user = find_byuid(source);
        do_nick((user ? user->nick : source), av[0], NULL, NULL, NULL,
                NULL, strtoul(av[1], NULL, 10), 0, NULL, NULL, NULL, 0,
                NULL);
    }
    free(temp);
    return MOD_CONT;
}

int denora_event_436(char *source, int ac, char **av)
{
    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    if (ac < 1)
        return MOD_CONT;

    m_nickcoll(av[0]);
    return MOD_CONT;
}

int denora_event_error(char *source, int ac, char **av)
{
    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    if (ac >= 1) {
        alog(LOG_ERROR, "ERROR: %s", av[0]);
    }
    return MOD_CONT;
}

int denora_event_eob(char *source, int ac, char **av)
{
    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    send_cmd(NULL, "%s EA", p10id);
    update_sync_state(source, SYNC_COMPLETE);
    return MOD_CONT;
}

/* *INDENT-OFF* */
void moduleAddIRCDMsgs(void) {
    Message *m;
    m = createMessage("ERROR",    denora_event_error); addCoreMessage(IRCD,m);
    m = createMessage("NOTICE",   denora_event_null); addCoreMessage(IRCD,m);
    m = createMessage("SERVER",   denora_event_server); addCoreMessage(IRCD,m);
    m = createMessage("PASS",     denora_event_null); addCoreMessage(IRCD,m);
 
    /* let the p10 tokens begin */
      
    /* end of burst */
    m = createMessage("EB",       denora_event_eob); addCoreMessage(IRCD,m);
    /* nick */
    m = createMessage("N",        denora_event_nick); addCoreMessage(IRCD,m);
    /* ping */
    m = createMessage("G",        denora_event_ping); addCoreMessage(IRCD,m);
    /* pong */
    m = createMessage("Z",        denora_event_null); addCoreMessage(IRCD,m);
    /* MODE */
    m = createMessage("M",        denora_event_mode); addCoreMessage(IRCD,m);
    /* CREATE */
    m = createMessage("C",        denora_event_create); addCoreMessage(IRCD,m);
    /* JOIN */
    m = createMessage("J",        denora_event_join); addCoreMessage(IRCD,m);
    /* QUIT */
    m = createMessage("Q",        denora_event_quit); addCoreMessage(IRCD,m);
    /* TOPIC */
    m = createMessage("T",        denora_event_topic); addCoreMessage(IRCD,m);
    /* MOTD */
    m = createMessage("MO",       denora_event_motd); addCoreMessage(IRCD,m);
    /* KICK */
    m = createMessage("K",        denora_event_kick); addCoreMessage(IRCD,m);
    /* AWAY */
    m = createMessage("A",        denora_event_away); addCoreMessage(IRCD,m);
    /* ADMIN */
    m = createMessage("AD",       denora_event_null); addCoreMessage(IRCD,m);
    /* PART */
    m = createMessage("L",        denora_event_part); addCoreMessage(IRCD,m);
    /* BURST */
    m = createMessage("B",        denora_event_sjoin); addCoreMessage(IRCD,m);
    /* PRIVMSG */
    m = createMessage("P",        denora_event_privmsg); addCoreMessage(IRCD,m);
    /* NOTICE */
    m = createMessage("O",        denora_event_notice); addCoreMessage(IRCD,m);
    /* SQUIT */
    m = createMessage("SQ",       denora_event_squit); addCoreMessage(IRCD,m);
    /* INVITE */
    m = createMessage("I",        denora_event_null); addCoreMessage(IRCD,m);
    /* DESYNCH */
    m = createMessage("DS",       denora_event_error); addCoreMessage(IRCD,m);
    /* WALLOP */
    m = createMessage("WA",       denora_event_null); addCoreMessage(IRCD,m);
    /* VERSION */
    m = createMessage("V",        m_version); addCoreMessage(IRCD,m);
    /* WHOIS */
    m = createMessage("W",        denora_event_whois); addCoreMessage(IRCD,m);
    /* SERVER */
    m = createMessage("S",        denora_event_server); addCoreMessage(IRCD,m);
    /* PONG */
    m = createMessage("Z",        denora_event_pong); addCoreMessage(IRCD,m);
    /* STATS */
    m = createMessage("R",        m_stats); addCoreMessage(IRCD,m);
    /* ACCCOUNT */
    m = createMessage("AC",       denora_event_null); addCoreMessage(IRCD,m);
    /* GLINE */
    m = createMessage("GL",       denora_event_sgline); addCoreMessage(IRCD,m);
    /* INFO */
    m = createMessage("F",        denora_event_null); addCoreMessage(IRCD,m);
    /* SETTIME */
    m = createMessage("SE",       denora_event_null); addCoreMessage(IRCD,m);
    /* TIME */
    m = createMessage("TI",       m_time); addCoreMessage(IRCD,m);
    /* TRACE */
    m = createMessage("TR",       denora_event_null); addCoreMessage(IRCD,m);
    /* RPING */
    m = createMessage("RI",       denora_event_null); addCoreMessage(IRCD,m);
    /* SILENCE */
    m = createMessage("U",        denora_event_null); addCoreMessage(IRCD,m);    
    /* End of Burst Acknowledge */
    m = createMessage("EA",       denora_event_null); addCoreMessage(IRCD,m);
}

/* *INDENT-ON* */

/* AK GL * +*!*@*.aol.com 864000 :testing */
int denora_event_sgline(char *source, int ac, char **av)
{
    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    p10_gline(source, ac, av);
    return MOD_CONT;
}

void nefarious_cmd_stats(char *sender, const char *letter, char *server)
{
    Uid *ud;
    Server *s;

    ud = find_uid(sender);
    s = server_find(server);

    send_cmd((ud ? ud->uid : sender), "STATS %s :%s", letter,
             (s ? (s->suid ? s->suid : server) : server));
}

/* PART */
void nefarious_cmd_part(char *nick, char *chan, char *buf)
{
    Uid *ud;
    ud = find_uid(nick);
    if (buf) {
        send_cmd((ud ? ud->uid : nick), "L %s :%s", chan, buf);
    } else {
        send_cmd((ud ? ud->uid : nick), "L %s", chan);
    }
}

/* ABAAB J #ircops 1098031328 */
void nefarious_cmd_join(char *user, char *channel, time_t chantime)
{
    Uid *ud;

    ud = find_uid(user);

    if (started) {
        send_cmd((ud ? ud->uid : user), "J %s %ld", channel,
                 (long int) chantime);
    } else {
        send_cmd(p10id, "B %s %ld %s:o", channel,
                 (long int) time(NULL), (ud ? ud->uid : user));
    }
}

void nefarious_cmd_squit(char *servname, char *message)
{
    alog(LOG_PROTOCOL, "Servername %s Message %s", servname, message);
    send_cmd(p10id, "SQ %s :%s", p10id, message);
}

void nefarious_cmd_connect(void)
{

    if (!BadPtr(Numeric)) {
        inttobase64(p10id, atoi(Numeric), 2);
        me_server =
            do_server(NULL, ServerName, (char *) "0", ServerDesc, p10id);
        nefarious_cmd_pass(RemotePassword);
        nefarious_cmd_server(ServerName, 1, ServerDesc);
    } else {
        alog(LOG_NORMAL, "Numeric is required for P10 ircds");
        exit(1);
    }
}

int denora_event_pong(char *source, int ac, char **av)
{
    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    server_store_pong(source, time(NULL));
    return MOD_CONT;
}

/* PASS */
void nefarious_cmd_pass(char *pass)
{
    send_cmd(NULL, "PASS :%s", pass);
}

void nefarious_cmd_capab()
{
    /* not used by p10 */
}

/* SERVER [SERVERNAME] [HOPCOUNT] [START TIME] [LINK TIME] [PROTOCOL] [NUMERIC/MAXCONN] :[DESCRIPTION] */
/* SERVER irc.undernet.org 1 933022556 947908144 J10 AA]]] :[127.0.0.1] A Undernet Server */
void nefarious_cmd_server(char *servname, int hop, char *descript)
{
    send_cmd(NULL, "SERVER %s %d %ld %lu J10 %s]]] +s :%s", servname, hop,
             (long int) denora->start_time, (long int) time(NULL), p10id,
             descript);
}

/* GLOBOPS */
void nefarious_cmd_global(char *source, char *buf)
{
    Uid *id;

    id = find_uid(source);

    send_cmd((id ? id->uid : ServerName), "WA :%s", buf);
}

/* ABAAC A :I go away */
/* ABAAC A */
int denora_event_away(char *source, int ac, char **av)
{
    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    m_away(source, (ac ? av[0] : NULL));
    return MOD_CONT;
}

int denora_event_topic(char *source, int ac, char **av)
{
    char *newav[4];

    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    if (ac < 4)
        return MOD_CONT;

    newav[0] = av[0];
    newav[1] = av[1];
    newav[2] = av[ac - 2];
    newav[3] = av[ac - 1];

    do_topic(4, newav);
    return MOD_CONT;
}

int denora_event_squit(char *source, int ac, char **av)
{
    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    if (ac != 2)
        return MOD_CONT;
    do_squit(av[0]);
    return MOD_CONT;
}

/* ABAAB Q :Quit */
int denora_event_quit(char *source, int ac, char **av)
{
    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    if (ac != 1)
        return MOD_CONT;
    do_quit(source, ac, av);
    return MOD_CONT;
}

/* User modes */
/* ABAAA M Trystan +wg */

/* Channel modes */
/* ABAAA M #ircops +v ABAAB */
int denora_event_mode(char *source, int ac, char **av)
{
    User *u;
    char *sender;
    u = find_byuid(source);
    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }

    if (!u) {
        sender = source;
    } else {
        sender = u->nick;
    }

    if (ac < 2)
        return MOD_CONT;

    if (*av[0] == '#' || *av[0] == '&') {
        do_cmode(source, ac, av);
    } else {
        do_umode(sender, ac, av);
    }
    return MOD_CONT;
}

int denora_event_kill(char *source, int ac, char **av)
{
    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    if (ac != 2)
        return MOD_CONT;

    m_kill(source, av[0], av[1]);
    return MOD_CONT;
}

/* ABAAA K #ircops ABAAC :Trystan` */
/* ABAAA K #testchan ABAAB :test */
int denora_event_kick(char *source, int ac, char **av)
{
    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    do_p10_kick(source, ac, av);
    return MOD_CONT;
}

/* JOIN - is the same syntax */
/* ABAAB J #ircops 1098031328 */
int denora_event_join(char *source, int ac, char **av)
{
    User *u;

    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }

    u = find_byuid(source);

    if (ac != 2) {
        return MOD_CONT;
    }

    do_join((u ? u->nick : source), ac, av);
    return MOD_CONT;
}

/* CREATE - p10 don't join empty channels the CREATE them */
/* ABAAA C #ircops 1098031328 */
int denora_event_create(char *source, int ac, char **av)
{
    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    do_join(source, ac, av);
    return MOD_CONT;
}

/* BURST joins simlar to SJOIN */
/* AB B #denora 1113091975 +tn ABAAB,ABAAA:o :%*!*@*.aol.com */
/* s  c  0        1         2   3              4 */
/* AB B #test23 1115788230 ABAAB,ABAAA:o */
/* s  c 0         1         2 */
int denora_event_sjoin(char *source, int ac, char **av)
{
    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    alog(LOG_DEBUG, "Calling do_p10_burst");
    do_p10_burst(ac, av);
    return MOD_CONT;
}

/* ABAAA MO AG */
int denora_event_motd(char *source, int ac, char **av)
{
    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    m_motd(source);
    return MOD_CONT;
}

void nefarious_cmd_mode(char *source, char *dest, char *buf)
{
    alog(LOG_PROTOCOL, "MODE Source %s, dest %s, buf %s", source, dest,
         buf);
    send_cmd(p10id, "M %s %s %ld", dest, buf, (long int) time(NULL));
}

void nefarious_cmd_notice(char *source, char *dest, char *buf)
{
    User *u;
    Uid *ud;

    u = finduser(dest);
    ud = find_uid(source);

    send_cmd((ud ? ud->uid : source), "O %s :%s", (u ? u->uid : dest),
             buf);
}

void nefarious_cmd_privmsg(char *source, char *dest, char *buf)
{
    User *u;
    Uid *ud;

    u = finduser(dest);
    ud = find_uid(source);

    send_cmd((ud ? ud->uid : source), "P %s :%s", (u ? u->uid : dest),
             buf);
}

void nefarious_cmd_serv_notice(char *source, char *dest, char *msg)
{
    send_cmd(source, "NOTICE $%s :%s", dest, msg);
}

void nefarious_cmd_serv_privmsg(char *source, char *dest, char *msg)
{
    send_cmd(source, "PRIVMSG $%s :%s", dest, msg);
}

/* QUIT */
void nefarious_cmd_quit(char *source, char *buf)
{
    Uid *ud;
    ud = find_uid(source);

    if (buf) {
        send_cmd((ud ? ud->uid : source), "QUIT :%s", buf);
    } else {
        send_cmd((ud ? ud->uid : source), "QUIT");
    }
}

/* [NUMERIC PREFIX] N [NICK] [HOPCOUNT] [TIMESTAMP] [USERNAME] [HOST] <+modes> [BASE64 IP] [NUMERIC] :[USERINFO] */
/* AF N Client1 1 947957573 User userhost.net +oiwg DAqAoB AFAAA :Generic Client. */
void nefarious_cmd_nick(char *nick, char *name, const char *modes)
{
    char nicknumbuf[6];
    send_cmd(p10id, "N %s 1 %ld %s %s %s B]AAAB %sAA%c :%s", nick,
             (long int) time(NULL), ServiceUser, ServiceHost, modes, p10id,
             (p10nickcnt + 'A'), name);
    ircsnprintf(nicknumbuf, 6, "%sAA%c", p10id, (p10nickcnt + 'A'));
    new_uid(nick, nicknumbuf);
    p10nickcnt++;
}

/* EVENT: SERVER */
/* SERVER Auska.Nomadirc.net 1 1098025304 1098036331 J10 ABAP] + :Test Server */
/* SERVER [SERVERNAME] [HOPCOUNT] [START TIME] [LINK TIME] [PROTOCOL] [NUMERIC/MAXCONN] [FLAGS] :[DESCRIPTION] */
/* (AB S trystan.nomadirc.net 2 0 1106520454 P10 ACAP] +h :Test Server) */
int denora_event_server(char *source, int ac, char **av)
{
    Server *s;
    char uplinknum[3];
    *uplinknum = '\0';

    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    strlcpy(uplinknum, av[5], sizeof(uplinknum));

    if (!stricmp(av[1], "1")) {
        denora->uplink = sstrdup(av[0]);
        do_server(source, av[0], av[1], av[7], uplinknum);
    } else {
        s = server_find(source);
        do_server((s ? s->name : source), av[0], av[1], av[7], uplinknum);
    }
    return MOD_CONT;
}

/* ABAAA P ADAAB :help */
int denora_event_privmsg(char *source, int ac, char **av)
{
    User *u;
    Uid *id;

    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    u = find_byuid(source);
    id = find_nickuid(av[0]);

    if (ac != 2)
        return MOD_CONT;
    m_privmsg((u ? u->nick : source), (id ? id->nick : av[0]), av[1]);
    return MOD_CONT;
}

/* ABAAA L #ircops */
int denora_event_part(char *source, int ac, char **av)
{
    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    do_part(source, ac, av);
    return MOD_CONT;
}

int denora_event_whois(char *source, int ac, char **av)
{
    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    if (source && ac >= 1) {
        m_whois(source, av[1]);
    }
    return MOD_CONT;
}

/* AB G !1098031985.558075 services.nomadirc.net 1098031985.558075 */
int denora_event_ping(char *source, int ac, char **av)
{
    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    if (ac < 1)
        return MOD_CONT;
    nefarious_cmd_pong(p10id, av[0]);
    return MOD_CONT;
}

/* PONG */
void nefarious_cmd_pong(char *servname, char *who)
{
    char *t, *s;
    uint32 ts, tsnow, value;
    t = myStrGetToken(who, '!', 1);
    s = myStrGetToken(t, '.', 0);
    if (!s) {
        ts = 0;
    } else {
        ts = strtol(s, NULL, 10);
    }
    tsnow = time(NULL);
    value = tsnow - ts;
    if (denora->protocoldebug) {
        alog(LOG_PROTOCOL, "PONG: Server Name %s : Who %s", servname, who);
    }
    send_cmd(p10id, "Z %s %ld %ld %ld %s", p10id, (long int) ts,
             (long int) tsnow, (long int) value, militime_float(NULL));
    if (s) {
        free(s);
    }
    if (t) {
        free(t);
    }
}

void nefarious_cmd_bot_nick(char *nick, char *user, char *host, char *real,
                            char *modes)
{
    char nicknumbuf[6];
    send_cmd(p10id, "N %s 1 %ld %s %s %s B]AAAB %sAA%c :%s", nick,
             (long int) time(NULL), user, host, modes, p10id,
             (p10nickcnt + 'A'), real);
    ircsnprintf(nicknumbuf, 6, "%sAA%c", p10id, (p10nickcnt + 'A'));
    new_uid(nick, nicknumbuf);
    p10nickcnt++;
}

void nefarious_cmd_eob(void)
{
    send_cmd(p10id, "EB");
}

void nefarious_cmd_ping(char *server)
{
    /* AB G !1115872042.64217 denora.nomadirc.net 1115872042.64217 */
    send_cmd(p10id, "G !%s %s %s", militime_float(NULL), server,
             militime_float(NULL));
}

void nefarious_cmd_ctcp(char *source, char *dest, char *buf)
{
    send_cmd(source, "NOTICE %s :\1%s \1", dest, buf);
}

void nefarious_cmd_version(char *server)
{
    Uid *ud;
    Server *s;
    ud = find_uid(s_StatServ);
    s = server_find(server);

    send_cmd((ud ? ud->uid : s_StatServ), "V :%s",
             (s ? (s->suid ? s->suid : server) : server));
}

void nefarious_cmd_motd(char *sender, char *server)
{
    Uid *ud;
    Server *s;
    ud = find_uid(sender);
    s = server_find(server);

    send_cmd((ud ? ud->uid : sender), "MO %s",
             (s ? (s->suid ? s->suid : server) : server));

}

int denora_event_notice(char *source, int ac, char **av)
{
    if (denora->protocoldebug) {
        protocol_debug(source, ac, av);
    }
    if (ac != 2) {
        return MOD_CONT;
    }
    m_notice(source, av[0], av[1]);
    return MOD_CONT;
}

void moduleAddIRCDCmds()
{
    pmodule_cmd_nick(nefarious_cmd_nick);
    pmodule_cmd_mode(nefarious_cmd_mode);
    pmodule_cmd_bot_nick(nefarious_cmd_bot_nick);
    pmodule_cmd_notice(nefarious_cmd_notice);
    pmodule_cmd_privmsg(nefarious_cmd_privmsg);
    pmodule_cmd_serv_notice(nefarious_cmd_serv_notice);
    pmodule_cmd_serv_privmsg(nefarious_cmd_serv_privmsg);
    pmodule_cmd_quit(nefarious_cmd_quit);
    pmodule_cmd_pong(nefarious_cmd_pong);
    pmodule_cmd_join(nefarious_cmd_join);
    pmodule_cmd_part(nefarious_cmd_part);
    pmodule_cmd_global(nefarious_cmd_global);
    pmodule_cmd_squit(nefarious_cmd_squit);
    pmodule_cmd_connect(nefarious_cmd_connect);
    pmodule_cmd_eob(nefarious_cmd_eob);
    pmodule_cmd_ctcp(nefarious_cmd_ctcp);
    pmodule_cmd_version(nefarious_cmd_version);
    pmodule_cmd_stats(nefarious_cmd_stats);
    pmodule_cmd_motd(nefarious_cmd_motd);
    pmodule_cmd_ping(nefarious_cmd_ping);
}

int DenoraInit(int argc, char **argv)
{
    if (denora->protocoldebug) {
        protocol_debug(NULL, argc, argv);
    }
    moduleAddAuthor("Denora");
    moduleAddVersion
        ("$Id$");
    moduleSetType(PROTOCOL);

    pmodule_ircd_version("Nefarious IRCu 0.4.0");
    pmodule_ircd_cap(myIrcdcap);
    pmodule_ircd_var(myIrcd);
    pmodule_ircd_useTSMode(0);
    pmodule_irc_var(IRC_NEFARIOUS);
    IRCDModeInit();
    pmodule_oper_umode(UMODE_o);

    moduleAddIRCDCmds();
    moduleAddIRCDMsgs();

    return MOD_CONT;
}