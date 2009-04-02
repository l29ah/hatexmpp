
#define FUSE_USE_VERSION 26

#include <glib.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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

extern GMainLoop *main_loop;
extern GMainContext *context;
extern ClientConfig *config;
extern Roster *roster;

/* Logging stuff */
extern void * fsinit(void *);
extern void logs(const char *, size_t);
extern char * logstr(const char *);
extern char * make_message(const char *fmt, ...);
#define logf(FMT,ARGS...) free(logstr(make_message(FMT, ##ARGS)))
extern char LogBuf[];
extern int LogBufEnd;

extern void xmpp_send(const gchar *to, const gchar *body);
