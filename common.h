
#define FUSE_USE_VERSION 26

#include <glib.h>
#include <glib/gprintf.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fuse.h>

#define DEFAULT_CONFIG "hatexmpp.conf"

/*
typedef struct {
        const char *server;
        const char *username;
        const char *password;
        const char *resource;
	const char *muc_default_nick;
} ClientConfig;
*/

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
	GPtrArray *resources;
	GArray *log;
	unsigned presence;
	time_t lastmsgtime;
	unsigned type;
} rosteritem;

extern char HateXMPP_ver[];

extern GMainLoop *main_loop;
extern GMainContext *context;
extern GHashTable *config;
extern GHashTable *roster;

/* Logging stuff */
extern void * fsinit(void *);
extern void logs(const char *, size_t);
extern char * logstr(char *);
#define logf(FMT,ARGS...) free(logstr(g_strdup_printf(FMT, ##ARGS)))
extern GArray *LogBuf;

extern gchar *get_resource(gchar *jid);
extern gchar *get_jid(gchar *jid);
extern void xmpp_send(const gchar *to, const gchar *body);
extern int joinmuc(const char *jid, const char *password, const char *nick);
extern int partmuc(const char *jid, const char *nick);

extern int addri(const char *jid, GPtrArray *resources, unsigned type);
extern int destroyri(rosteritem *RI);

