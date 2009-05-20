#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gthread.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <loudmouth/loudmouth.h>

#define PROGRAM_NAME "HateXMPP"
#define DEFAULT_CONFIG "hatexmpp.conf"

#define MUC 2
#define GUY 1

#define PRESENCE_OFFLINE 0
#define PRESENCE_ONLINE 1

#define fatal(...) ixp_eprint("ixpsrv: fatal: " __VA_ARGS__)

typedef struct resourceitem_s {
	gchar *name;
	unsigned presence;
} resourceitem;

typedef struct rosteritem_s {
	gchar *jid;
	GHashTable *resources;
	GArray *log;
	resourceitem *self_resource;
	time_t lastmsgtime;
	unsigned type;
} rosteritem;

#define FD_NONBLOCK 1

typedef struct FD_s {
	int id;
	rosteritem *ri;
	unsigned type;	//unused
	GArray *readbuf;	//unused
	GArray *writebuf;
} FD;

extern char HateXMPP_ver[];

//extern void * mainloopthread(void *loop);

extern pthread_t thr;

extern GMainLoop *main_loop;
extern GMainContext *context;
extern GHashTable *config;
extern GHashTable *roster;

/* Logging'n'debug stuff */
#define OMGBUG g_on_error_query(PROGRAM_NAME);

extern int fs_init();

extern void logs(const char *, size_t);
extern char * logstr(char *);
#define logf(FMT,ARGS...) free(logstr(g_strdup_printf(FMT, ##ARGS)))
extern GArray *LogBuf;

extern gchar *get_resource(const gchar *jid);
extern gchar *get_jid(const gchar *jid);

extern void xmpp_send(const gchar *to, const gchar *body);
extern int joinmuc(const gchar *jid, const gchar *password, const gchar *nick);
extern int partmuc(const gchar *jid, const gchar *nick, const gchar *leave);

extern rosteritem *addri(const gchar *jid, GHashTable *resources, unsigned type);
extern int destroyri(rosteritem *RI);

extern void free_all();
extern int xmpp_connect();
extern void xmpp_disconnect();
extern void xmpp_add_to_roster(const gchar *jid);
extern void xmpp_del_from_roster(const gchar *jid);
extern void xmpp_muc_change_nick(const gchar *mucjid, const gchar *nick);
	
extern int fd_events;
extern gchar * eventstr(gchar *str);
#define eventf(FMT,ARGS...) g_free(eventstr(g_strdup_printf(FMT, ##ARGS)))

extern time_t last_activity_time;

extern int banmuc(const char *mucjid, const char *who);

enum connection_state_e {
	OFFLINE,
	CONNECTING,
	ONLINE
} connection_state;

