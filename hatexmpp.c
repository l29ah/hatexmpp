#include <stdlib.h>
#include <string.h>
#include <loudmouth/loudmouth.h>
#include "common.h"

//extern int fsinit(void);

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

static LmHandlerResult message_rcvd_cb(LmMessageHandler *handler, LmConnection *connection, LmMessage *m, gpointer data)
{
	gchar *from, *to, *body;
	from = lm_message_node_get_attribute(m->node, "from");
	to = lm_message_node_get_attribute(m->node, "to");
	body = lm_message_node_get_value(lm_message_node_get_child(m->node, "body"));
	g_print("Message from %s to %s: %s\n", from, to, body );
	return LM_HANDLER_RESULT_REMOVE_MESSAGE;
}

static LmHandlerResult roster_rcvd_cb(LmMessageHandler *handler, LmConnection *connection, LmMessage *m, gpointer data)
{
	LmMessageNode *query, *item;
	GError *error = NULL;
	Roster *buddy;
	query = lm_message_node_get_child(m->node, "query");
	item  = lm_message_node_get_child (query, "item");
	while (item)
	{
		const gchar *jid, *nick;
		jid = lm_message_node_get_attribute (item, "jid");
		nick = get_nick(jid);
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
	if (success)
	{
		m = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_AVAILABLE);
		lm_connection_send(connection, m, NULL);
		lm_message_unref(m);

		m = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_GET);
		handler = lm_message_handler_new(roster_rcvd_cb, NULL, NULL );
		lm_message_node_set_attribute(lm_message_node_add_child(m->node, "query", NULL), "xmlns", "jabber:iq:roster");
        	result = lm_connection_send_with_reply(connection, m, handler, NULL);
		lm_message_handler_unref(handler);
        	lm_message_unref (m);
	} else
	{
		g_error ("Authentication failed");
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
	LmMessageHandler *handler;

	fsinit(argc, argv);

	config = g_new0(ClientConfig, 1);

	config->server = "jabber.ru";
	config->username = "lexszer0";
	config->password = "123123123";
	config->resource = "hatexmpp";
	context = g_main_context_new();
        connection = lm_connection_new_with_context (config->server, context);

        if (!lm_connection_open (connection, (LmResultFunction) connection_open_cb, NULL, g_free, &error))
		g_error ("lm_connection_open failed");
	
	lm_connection_register_message_handler(connection, lm_message_handler_new(message_rcvd_cb, NULL, NULL), LM_MESSAGE_TYPE_MESSAGE, LM_HANDLER_PRIORITY_NORMAL);

        main_loop = g_main_loop_new (context, FALSE);
        g_main_loop_run (main_loop);

        return 0;
}
