#include "denora.h"
#define AUTHOR "Jobe"
#define VERSION "1.0"
#define MYNAME "ircd_sslclifp"

#define LOGTARGET "#services"

int do_mark(char *source, int ac, char **av);

int DenoraInit(int argc, char **argv)
{
  Message *msg = NULL;
  int status;

  if (denora_get_ircd() == IRC_NEFARIOUS) {
    msg = createMessage("MK", do_mark);
    status = moduleAddMessage(msg, MOD_HEAD);
    if (status != MOD_ERR_OK) {
      /* something went wrong say something about */
      alog(LOG_NORMAL, "[%s%s] unable to bind to 'MK' error [%d][%s]", MYNAME, MODULE_EXT, status, ModuleGetErrStr(status));
      return MOD_STOP;
    }
    msg = createMessage("MARK", do_mark);
    status = moduleAddMessage(msg, MOD_HEAD);
    if (status != MOD_ERR_OK) {
      /* something went wrong say something about */
      alog(LOG_NORMAL, "[%s%s] unable to bind to 'MARK' error [%d][%s]", MYNAME, MODULE_EXT, status, ModuleGetErrStr(status));
      return MOD_STOP;
    }
  } else {
    alog(LOG_NORMAL, "[%s%s] Youre IRCd is not supported, unloading!", MYNAME, MODULE_EXT);
    return MOD_STOP;
  }

  moduleAddAuthor(AUTHOR);
  moduleAddVersion(VERSION);
  return MOD_CONT;
}

void DenoraFini(void)
{
  alog(LOG_NORMAL, "Unloading ircd_sslclifp%s", MODULE_EXT);
}

int do_mark(char *source, int ac, char **av)
{
  User *u;

  if (ac < 3)
    return MOD_CONT;

  u = user_find(av[0]);

  if (u) {
    if (stricmp(av[1], "SSLCLIFP") == 0) {
      denora_cmd_privmsg(s_StatServ, LOGTARGET, "\x02SSLCLIFP\x02 %s has SSL fingerprint %s", av[0], av[2]);
    }
  }

  return MOD_CONT;
}
