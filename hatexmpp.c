#include "common.h"
#include "xmpp.h"


GMainLoop *main_loop;
GMainContext *context;
ClientConfig *config;
Roster *roster;

GArray *LogBuf;
GHashTable *RosterHT;

inline void logs(const char *msg, size_t len) {
	g_array_append_vals(LogBuf, msg, len);
}

char * logstr(char *msg) {
	size_t len;

	len = strlen(msg);
	logs(msg, len);

	return msg;
}

/* Nice code from 'man printf' */
char * make_message(const char *fmt, ...) {
	/* Guess we need no more than 100 bytes. */
        int n, size = 100;
        char *p, *np;
        va_list ap;

	if ((p = malloc(size)) == NULL)
        	return NULL;

        while (1) {
		/* Try to print in the allocated space. */
               	va_start(ap, fmt);
               	n = vsnprintf(p, size, fmt, ap);
               	va_end(ap);
               	/* If that worked, return the string. */
               	if (n > -1 && n < size)
                	return p;
               	/* Else try again with more space. */
               	if (n > -1)    /* glibc 2.1 */
                	size = n+1; /* precisely what is needed */
               	else           /* glibc 2.0 */
                	size *= 2;  /* twice the old size */
               	if ((np = realloc (p, size)) == NULL) {
                	free(p);
                	return NULL;
               	} else {
                	p = np;
                }
	}
}


static gchar *get_nick(const gchar *jid)
{
	const gchar *ch;
	g_return_val_if_fail(jid != NULL, NULL);
	ch = strchr(jid, '@');
        if (!ch) 
                return (gchar *) jid;
        return g_strndup (jid, ch-jid);
}

int main (int argc, char **argv)
{
	pthread_t fsthread;
	struct fuse_args par = FUSE_ARGS_INIT(argc, argv);
	
	LogBuf = g_array_sized_new(FALSE, FALSE, 1, 512);
	logstr("hi all\n");
	RosterHT = g_hash_table_new(g_str_hash, g_str_equal);
	pthread_create(&fsthread, NULL, fsinit, (void *)&par); 

	logstr("fuse is going up\n");
	config = g_new0(ClientConfig, 1);
	config->server = "jabber.ru";
	config->username = "lexszer0";
	config->password = "123123123";
	config->resource = "hatexmpp";
	context = g_main_context_new();
	xmpp_connect();
	logstr("server connected\n");
        main_loop = g_main_loop_new (context, FALSE);
	/* TODO: fix segfaults (valgrind is your friend?) */
        g_main_loop_run (main_loop);
	sleep(-1);	/* I don't want to die so soon */
	/* TODO fuse shutdown */
	g_array_free(LogBuf, TRUE);
	
        return 0;
}
