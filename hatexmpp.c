#include "common.h"
#include "xmpp.h"

#define LOGBUFSIZE 1000000

char LogBuf[LOGBUFSIZE];	/* TODO dynamic array */
int LogBufEnd = 0;

void logs(const char *msg, size_t len) {
	memcpy(LogBuf + LogBufEnd, msg, len);
	LogBufEnd += len;
}

void logstr(const char *msg) {
	size_t len;

	len = strlen(msg);
	logs(msg, len);
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
	
	logstr("hi all\n");
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
	g_print("asdfasdf");
	/* TODO: fix segfaults (valgrind is your friend?) */
        g_main_loop_run (main_loop);
	sleep(-1);	/* I don't want to die so soon */
	/* TODO fuse shutdown */
        return 0;
}
