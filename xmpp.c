#include "common.h"
#include <loudmouth/loudmouth.h>

LmConnection *connection;


Roster *roster_add(Roster **p, RosterItem item)
{
	Roster *n = malloc(sizeof(Roster));
	size_t s;

	if (!n) {
		logstr("Can't allocate memory for roster item!");
		return NULL;
	}
	logf( "Adding JID: %s; Nick: %s\n", item.jid, item.nick );
	n->next = *p;
	*p = n;
	n->item.jid = malloc(s = (strlen(item.jid) + 1));
	memcpy(n->item.jid, item.jid, s);
	n->item.nick = malloc(s = (strlen(item.nick) + 1));
	memcpy(n->item.nick, item.nick, s);
	return *p;
}

void roster_print(Roster *n)
{
	if (n == NULL)
	{
		g_print("list is empty\n");
	}
	while (n != NULL)
	{
		g_print ( "%p %p JID: %s; Nick: %s\n",n, n->next, n->item.jid, n->item.nick );
		n = n->next;
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

static LmHandlerResult message_rcvd_cb(LmMessageHandler *handler, LmConnection *connection, LmMessage *m, gpointer data)
{
	gchar *from, *to, *body;
	from = lm_message_node_get_attribute(m->node, "from");
	to = lm_message_node_get_attribute(m->node, "to");
	body = lm_message_node_get_value(lm_message_node_get_child(m->node, "body"));
	g_print("Message from %s to %s: %s\n", from, to, body );
	xmpp_send(from,body);
	return LM_HANDLER_RESULT_REMOVE_MESSAGE;
}

static LmHandlerResult roster_rcvd_cb(LmMessageHandler *handler, LmConnection *connection, LmMessage *m, gpointer data)
{
	LmMessageNode *query, *item;
	Roster *buddy;

	query = lm_message_node_get_child(m->node, "query");
	item  = lm_message_node_get_child (query, "item");
	while (item)
	{
		RosterItem roster_item;
		roster_item.jid = lm_message_node_get_attribute (item, "jid");
		roster_item.nick = get_nick(roster_item.jid);
		roster_add(&roster,roster_item);
		item = item->next;
	}
	lm_message_node_unref(query);
	roster_print(roster);
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
	//g_print("desudesu\n");
	if (!success)
		g_error("Cannot open connection");
	if (!lm_connection_authenticate (connection, config->username, config->password, config->resource, (LmResultFunction) connection_auth_cb, NULL, g_free, NULL))
		g_error ("lm_connection_authenticate failed");
}

extern void xmpp_connect()
{
	
	connection = lm_connection_new_with_context (config->server, context);
        if (!lm_connection_open (connection, (LmResultFunction) connection_open_cb, NULL, g_free, NULL))
		g_error ("lm_connection_open failed");
	lm_connection_register_message_handler(connection, lm_message_handler_new(message_rcvd_cb, NULL, NULL), LM_MESSAGE_TYPE_MESSAGE, LM_HANDLER_PRIORITY_NORMAL);
}

extern void xmpp_send(gchar *to, gchar *body)
{
	LmMessage *m;
	m = lm_message_new(to, LM_MESSAGE_TYPE_MESSAGE);
	lm_message_node_add_child(m->node, "body", body);
	lm_connection_send(connection, m, NULL);
	lm_message_unref(m);
}
