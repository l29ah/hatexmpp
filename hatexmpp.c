#include <stdlib.h>
#include <string.h>
#include <loudmouth/loudmouth.h>

typedef struct {
	const gchar *server;
	const gchar *username;
	const gchar *password;
	const gchar *resource;
} ClientConfig;

typedef struct {
	const gchar *jid;
	const gchar *nick;
	struct Roster *next;
} Roster;

static GMainLoop *main_loop;
ClientConfig *config;
LmConnection *connection;
Roster *roster;
/*
struct Roster *roster_add(struct Roster *p, gchar *jid, gchar *nick)
{
	struct Roster *item = malloc(sizeof(struct Roster));
	if (item == NULL)
		return NULL;
	item->jid = jid;
	item->nick = nick;
	p->next = item;
	return *item;
}
*/
static gchar *get_nick(const gchar *jid)
{
	const gchar *ch;
	g_return_val_if_fail(jid != NULL, NULL);
	ch = strchr(jid, '@');
        if (!ch) 
                return (gchar *) jid;
        return g_strndup (jid, ch-jid);
}

static LmHandlerResult roster_rcvd_cb(LmMessageHandler *handler, LmConnection *connection, LmMessage *m, gpointer data)
{
	LmMessage *msg, *roster;
	LmMessageNode *query, *item;
	GError *error = NULL;
	Roster *buddy;
	g_print("\n--------------------------------------------------------------\nenter callback\n");
	query = lm_message_node_get_child(m->node, "query");
	item  = lm_message_node_get_child (query, "item");
//	roster = malloc(sizeof(struct Roster));
//	buddy=roster;
	while (item)
	{
		const gchar *jid, *nick;
		jid = lm_message_node_get_attribute (item, "jid");
		nick = get_nick(jid);
//		roster_add(roster, jid, nick);
		g_print ( "JID: %s; Nick: %s\n", jid, nick );
		item = item->next;
	}
	lm_message_node_unref(query);
	return LM_HANDLER_RESULT_REMOVE_MESSAGE;
}

static void connection_auth_cb(LmConnection *connection, gboolean success, void *data)
{
	LmMessage *m;
	LmMessageHandler *handler;
	gboolean result;
	GError *error = NULL;
	if (!success)
		g_error ("Authentication failed");
	m = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_GET);
	handler = lm_message_handler_new(roster_rcvd_cb, NULL, NULL );
	lm_message_node_set_attribute(lm_message_node_add_child(m->node, "query", NULL), "xmlns", "jabber:iq:roster");
        result = lm_connection_send_with_reply(connection, m, handler, &error);
	lm_message_handler_unref(handler);
        lm_message_unref (m);
        if (!result)
	{
		g_error ("lm_connection_send failed");
		lm_connection_close (connection, NULL);
	}
}

static void connection_open_cb (LmConnection *connection, gboolean success, void *data)
{
        GError *error = NULL;
	if (!success)
		g_error("Cannot open connection");
	if (!lm_connection_authenticate (connection, config->username, config->password, config->resource, (LmResultFunction) connection_auth_cb, NULL, g_free, &error))
		g_error ("lm_connection_authenticate failed");
}

int main (int argc, char **argv)
{
	GMainContext	*context;
	GError		*error = NULL;

	config = g_new0(ClientConfig, 1);

	config->server = "jabber.ru";
	config->username = "lexszer0";
	config->password = "123123123";
	config->resource = "hatexmpp";
	context = g_main_context_new();
        connection = lm_connection_new_with_context (config->server, context);

        if (!lm_connection_open (connection, (LmResultFunction) connection_open_cb, NULL, g_free, &error))
		g_error ("lm_connection_open failed");

        main_loop = g_main_loop_new (context, FALSE);
        g_main_loop_run (main_loop);

        return 0;
}
