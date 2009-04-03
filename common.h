
#define FUSE_USE_VERSION 26

#include <glib.h>
#include <glib/gprintf.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fuse.h>

typedef struct {
        const char *server;
        const char *username;
        const char *password;
        const char *resource;
} ClientConfig;

#define MUC 2
#define GUY 1

#define PRESENCE_OFFLINE 0
#define PRESENCE_ONLINE 1

typedef struct recourceitem_s {
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

extern GMainLoop *main_loop;
extern GMainContext *context;
extern ClientConfig *config;
extern GHashTable *roster;

/* Logging stuff */
extern void * fsinit(void *);
extern void logs(const char *, size_t);
extern char * logstr(char *);
#define logf(FMT,ARGS...) free(logstr(g_strdup_printf(FMT, ##ARGS)))
extern GArray *LogBuf;

extern void xmpp_send(const gchar *to, const gchar *body);
extern int joinmuc(const char *jid, const char *password, const char *nick);
