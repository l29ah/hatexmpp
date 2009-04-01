#include "common.h"
#include "xmpp.h"

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

	pthread_create(&fsthread, NULL, fsinit, (void *)&par);

	config = g_new0(ClientConfig, 1);
	config->server = "jabber.ru";
	config->username = "lexszer0";
	config->password = "123123123";
	config->resource = "hatexmpp";
	context = g_main_context_new();
	xmpp_connect();
        main_loop = g_main_loop_new (context, FALSE);
	g_print("asdfasdf");
        //g_main_loop_run (main_loop);
	sleep(-1);
        return 0;
}
