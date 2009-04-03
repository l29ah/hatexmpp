#include "common.h"
#include <loudmouth/loudmouth.h>

#define XMPP_MUC_XMLNS "http://jabber.org/protocol/muc"

void xmpp_send(const gchar *to, const gchar *body);

LmConnection *connection;

int partmuc(const char *jid, const char *nick) {
        LmMessage *m;
//        LmMessageNode *node;
	gchar *to;
	
	to = g_strdup_printf("%s/%s", jid, nick);
	m = lm_message_new_with_sub_type(to, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_AVAILABLE);
	lm_message_node_set_attribute(m->node, "type", "unavailable");
	/* TODO part message */
	//node = lm_message_node_add_child(m->node, "status", "bye! your hatexmpp.");
	lm_connection_send(connection, m, NULL);
	lm_message_unref(m);
}

int joinmuc(const char *jid, const char *password, const char *nick) {
	LmMessage *m;
	LmMessageNode *node;
	gchar *id_str, *to;
	rosteritem *ri;
	static int id;	/* TODO what's this!? */

	if(!nick) nick = "hatexmpp";
	to = g_strdup_printf("%s/%s", jid, nick);
	m = lm_message_new_with_sub_type(to, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_AVAILABLE);
	node = lm_message_node_add_child(m->node, "x", NULL);
	lm_message_node_set_attribute(node, "xmlns", XMPP_MUC_XMLNS);
	lm_message_node_add_child (m->node, "show", "available");

	if (password) {
	        lm_message_node_add_child (node, "password", password);
	}

	id_str = g_strdup_printf ("muc_join_%d", id);
	lm_message_node_set_attribute (m->node, "id", id_str);
        lm_connection_send(connection, m, NULL);
	lm_message_unref(m);

	ri = malloc(sizeof(rosteritem));
	ri->jid = g_strdup(jid);
	ri->log = g_array_new(FALSE, FALSE, 1);
	ri->type = MUC;
	g_hash_table_insert(RosterHT, g_strdup(jid), ri);

	g_free(id_str);
	g_free(to);
	++id;
	return 0;
}

Roster *roster_add(Roster **p, RosterItem item)
{
	Roster *n = malloc(sizeof(Roster));
	size_t s;
	rosteritem *ri;

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

	ri = malloc(sizeof(rosteritem));
	ri->jid = g_strdup(item.jid);
	ri->resource = NULL;
	ri->log = g_array_new(FALSE, FALSE, 1);
	ri->type = GUY;
	g_hash_table_insert(RosterHT, g_strdup(item.jid), ri);
	return *p;
}

void roster_print(Roster *n)
{
	if (n == NULL)
	{
		logstr("list is empty\n");
	}
	while (n != NULL)
	{
		logf( "%p %p JID: %s; Nick: %s\n",n, n->next, n->item.jid, n->item.nick );
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
	const gchar *from, *to, *body;
	gchar *jid;
	gpointer log;
	rosteritem *ri;

	from = lm_message_node_get_attribute(m->node, "from");
	to = lm_message_node_get_attribute(m->node, "to");
	body = lm_message_node_get_value(lm_message_node_get_child(m->node, "body"));
	/* TODO fix awful malloc */
	/*
	if (from && body) {
		logf("Message from %s to %s: %s\n", from, to, body );
		jid = g_strndup(from, strrchr(from, '/') - from);
		ri = g_hash_table_lookup(RosterHT, jid);
		if(ri) {
			ri->lastmsgtime = time(NULL);
			log = ri->log;
			logf("appending to %s\n", jid);
			g_array_append_vals(log, body, strlen(body));
			g_array_append_vals(log, "\n", 2);
		} else {
			logf("JID %s is not in TalkLog, ignoring message", jid);
		}
	} else logf("Message from noone or empty one!\n");
	*/
	return LM_HANDLER_RESULT_REMOVE_MESSAGE;
}

static LmHandlerResult roster_rcvd_cb(LmMessageHandler *handler, LmConnection *connection, LmMessage *m, gpointer data)
{
	LmMessageNode *query, *item;
//	Roster *buddy;

	logstr("Roster callback!\n");
	query = lm_message_node_get_child(m->node, "query");
	item  = lm_message_node_get_child (query, "item");
	while (item)
	{
		RosterItem roster_item;
		roster_item.jid = (gchar *)lm_message_node_get_attribute(item, "jid");
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

	logstr("connection_auth_cb\n");
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
	logstr("connection_open_cb\n");
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

void xmpp_send(const gchar *to, const gchar *body)
{
	LmMessage *m;
	rosteritem *ri;

	m = lm_message_new(to, LM_MESSAGE_TYPE_MESSAGE);
	ri = g_hash_table_lookup(RosterHT, to);
	if(ri) {
		if(ri->type == MUC) lm_message_node_set_attribute(m->node, "type", "groupchat");
	}
	lm_message_node_add_child(m->node, "body", body);
	lm_connection_send(connection, m, NULL);
	lm_message_unref(m);
}
