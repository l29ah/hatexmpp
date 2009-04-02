
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

typedef struct {
        char *jid;
        char *nick;
} RosterItem;

typedef struct Roster_s {
	RosterItem item;
        struct Roster_s *next;
} Roster;

typedef struct ptrlist_s {
	void *v;
	struct ptrlist_s *n;
} ptrlist;

#define MUC 2
#define GUY 1

typedef struct rosteritem_s {
	gchar *jid;
	gchar *resource;
	GArray *log;
	time_t lastmsgtime;
	unsigned type;
} rosteritem;

extern GMainLoop *main_loop;
extern GMainContext *context;
extern ClientConfig *config;
extern Roster *roster;

/* Logging stuff */
extern void * fsinit(void *);
extern void logs(const char *, size_t);
extern char * logstr(char *);
#define logf(FMT,ARGS...) free(logstr(g_strdup_printf(FMT, ##ARGS)))
extern GArray *LogBuf;
extern GHashTable *RosterHT;

extern void xmpp_send(const gchar *to, const gchar *body);
