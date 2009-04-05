
#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <errno.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gthread.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fuse.h>

#define PROGRAM_NAME "HateXMPP"
#define DEFAULT_CONFIG "hatexmpp.conf"

#define MUC 2
#define GUY 1

#define PRESENCE_OFFLINE 0
#define PRESENCE_ONLINE 1

typedef struct resourceitem_s {
	gchar *name;
	unsigned presence;
} resourceitem;

typedef struct rosteritem_s {
	gchar *jid;
	GHashTable *resources;
	GArray *log;
	unsigned presence;
	time_t lastmsgtime;
	unsigned type;
} rosteritem;


extern struct fuse_operations fuseoper;
extern char HateXMPP_ver[];

//extern void * mainloopthread(void *loop);

extern GMainLoop *main_loop;
extern GMainContext *context;
extern GHashTable *config;
extern GHashTable *roster;

/* Logging stuff */
extern int fuseinit(int argc, char **argv);
extern void logs(const char *, size_t);
extern char * logstr(char *);
#define logf(FMT,ARGS...) free(logstr(g_strdup_printf(FMT, ##ARGS)))
extern GArray *LogBuf;

extern gchar *get_resource(const gchar *jid);
extern gchar *get_jid(const gchar *jid);
extern void xmpp_connect();
extern void xmpp_send(const gchar *to, const gchar *body);
extern int joinmuc(const char *jid, const char *password, const char *nick);
extern int partmuc(const char *jid, const char *nick);

extern int addri(const char *jid, GHashTable *resources, unsigned type);
extern int destroyri(rosteritem *RI);

extern gchar *conf_read(GKeyFile *cf, gchar *section, gchar *key, gchar *def);
extern void free_all();
extern void xmpp_connect();
