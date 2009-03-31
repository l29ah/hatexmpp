#include <stdlib.h>
#include <string.h>
#include <loudmouth/loudmouth.h>

typedef struct {
	const gchar *server;
	const gchar *username;
	const gchar *password;
	const gchar *resource;
} ClientConfig;

static GMainLoop *main_loop;
ClientConfig *config;
LmConnection *connection;

static void connection_auth_cb(LmConnection *connection, gboolean success, MessageData *data)
{
	LmMessage	*m;
	gboolean	result;
	GError		*error = NULL;

	if (!success)
		g_error ("Authentication failed");
        
	m = lm_message_new_with_subtype("", LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_GET);
	lm_message_node_add_child(m->node, "query", "jabber:iq:roster");
        result = lm_connection_send(connection, m, &error);
        lm_message_unref (m);
        if (!result)
	{
		g_error ("lm_connection_send failed");
		lm_connection_close (connection, NULL);
	}
}

static void connection_open_cb (LmConnection *connection, gboolean success, ClientConfig *config)
{
        GError *error = NULL;
	if (!success)
		g_error("Cannot open connection");
	if (!lm_connection_authenticate (connection, config->username, config->password, config->resource, (LmResultFunction) connection_auth_cb, data->msg_data, g_free, &error))
		g_error ("lm_connection_authenticate failed");
}

int main (int argc, char **argv)
{
	GMainContext	*context;
	GError		*error = NULL;

	config = g_new0(ClientConfig, 1);

	config->server = "server.ru";
	config->username = "user";
	config->password = "passw";

	context = g_main_context_new();
        connection = lm_connection_new_with_context (config->server, context);

        if (!lm_connection_open (connection, (LmResultFunction) connection_open_cb, NULL, g_free, &error))
		g_error ("lm_connection_open failed");

        main_loop = g_main_loop_new (context, FALSE);
        g_main_loop_run (main_loop);

        return 0;
}
