#include "common.h"
#include "xmpp.h"


GMainLoop *main_loop;
GMainContext *context;
ClientConfig *config;
GHashTable *roster;

GArray *LogBuf;

inline void logs(const char *msg, size_t len) {
	g_array_append_vals(LogBuf, msg, len);
}

char * logstr(char *msg) {
	size_t len;
	
	g_printf("%s",msg);
	len = strlen(msg);
	logs(msg, len);

	return msg;
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
	
	/* TODO */
	//g_thread_init(NULL);
	LogBuf = g_array_sized_new(FALSE, FALSE, 1, 512);
	logstr("hi all\n");
	roster = g_hash_table_new(g_str_hash, g_str_equal);
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
	/* TODO free everything */

        return 0;
}
