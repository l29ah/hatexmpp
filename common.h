#include <glib.h>
#include <pthread.h>

typedef struct {
	int c;
	char **v;
} fsinit_arg;

extern int fsinit(void *);

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
