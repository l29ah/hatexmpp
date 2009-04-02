
#define FUSE_USE_VERSION 26

#include <glib.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fuse.h>

extern void * fsinit(void *);
extern void logs(const char *, size_t);
extern const char * logstr(const char *); 
extern char * make_message(const char *fmt, ...);

#define logf(FMT,ARGS...) free(logstr(make_message(FMT, ##ARGS)))

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

typedef struct {
	RosterItem item;
        struct Roster *next;
} Roster;

static GMainLoop *main_loop;
GMainContext *context;
ClientConfig *config;
Roster *roster;

extern char LogBuf[];
extern int LogBufEnd;
