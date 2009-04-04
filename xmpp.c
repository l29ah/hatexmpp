#include "common.h"
#include <loudmouth/loudmouth.h>

#define XMPP_MUC_XMLNS "http://jabber.org/protocol/muc"

void xmpp_send(const gchar *to, const gchar *body);

LmConnection *connection;

gchar *get_resource(gchar *jid)
{
	gchar *res;
	res = strchr(jid,'/');
	if (res)
		return g_strdup(res+1);
	return NULL;
}

gchar *get_jid(gchar *jid)
{
	gchar *ch;
	ch = strchr(jid, '/');
	if (ch) 
		return g_strndup(jid, ch - jid);
	else
		return g_strdup(jid);
}

void roster_add(rosteritem *ri)
{
	logf( "Adding %s to roster\n", ri->jid);
	ri->resources = g_ptr_array_new();
	ri->log = g_array_new(FALSE, FALSE, 1);
	g_hash_table_insert(roster, g_strdup(ri->jid), ri);
}

int partmuc(const char *jid, const char *nick) {
        LmMessage *m;
	gchar *to;

	if (!nick) nick = (gchar *) g_hash_table_lookup(config, "muc_default_nick");
	to = g_strdup_printf("%s/%s", jid, nick);
	m = lm_message_new_with_sub_type(to, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_AVAILABLE);
	lm_message_node_set_attribute(m->node, "type", "unavailable");
	/* TODO part message */
	//node = lm_message_node_add_child(m->node, "status", "bye! your hatexmpp.");
	lm_connection_send(connection, m, NULL);
	lm_message_unref(m);
	return 0;
}

int joinmuc(const char *jid, const char *password, const char *nick) {
	LmMessage *m;
	LmMessageNode *node;
	gchar *to;
	rosteritem *ri;

	if (!nick) nick = (gchar *) g_hash_table_lookup(config, "muc_default_nick");
	to = g_strdup_printf("%s/%s", jid, nick);
	m = lm_message_new_with_sub_type(to, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_AVAILABLE);
	node = lm_message_node_add_child(m->node, "x", NULL);
	lm_message_node_set_attribute(node, "xmlns", XMPP_MUC_XMLNS);
	lm_message_node_add_child (m->node, "show", "available");

	if (password) {
	        lm_message_node_add_child (node, "password", password);
	}

        lm_connection_send(connection, m, NULL);
	lm_message_unref(m);

	ri = g_new(rosteritem, 1);
	ri->jid = g_strdup(jid);
	ri->type = MUC;
	roster_add(ri);
	g_free(to);
	return 0;
}

static LmHandlerResult presence_rcvd_cb(LmMessageHandler *handler, LmConnection *connection, LmMessage *m, gpointer data)
{
	gchar *from, *jid, *res;
	rosteritem *ri;
	resourceitem *resource;

	from = (gchar *) lm_message_node_get_attribute(m->node, "from");
	jid = get_jid(from);
	res = get_resource(from);
	ri = g_hash_table_lookup(roster, jid);

	if (ri)	{
		// TODO: do something better with presence
		logf("Adding resource %s to %s\n", res, jid);
		if (res) {
			resource = g_new(resourceitem, 1);
			resource->name = res;
			resource->presence = PRESENCE_ONLINE;
			g_ptr_array_add(ri->resources, resource);
		}
		lm_message_unref(m);

	} else
		logf("Presence from unknown (%s), ignoring\n", from);
	return LM_HANDLER_RESULT_REMOVE_MESSAGE;
}


static LmHandlerResult message_rcvd_cb(LmMessageHandler *handler, LmConnection *connection, LmMessage *m, gpointer data)
{
	const gchar *from, *to, *body, *jid, *log_str;
	rosteritem *ri;

	from = lm_message_node_get_attribute(m->node, "from");
	to = lm_message_node_get_attribute(m->node, "to");
	body = lm_message_node_get_value(lm_message_node_get_child(m->node, "body"));
	if (from && body) {
		logf("Message from %s to %s: %s\n", from, to, body );
		jid = get_jid((gchar *) from);
		ri = g_hash_table_lookup(roster, jid);
		if(ri) {
			ri->lastmsgtime = time(NULL);
			if (ri->type == MUC)
				jid = get_resource(from);
			log_str = g_strdup_printf("%s: %s\n", jid, body);
			g_array_append_vals(ri->log, log_str, strlen(log_str));
		} else {
			logf("JID %s is not in TalkLog, ignoring message", jid);
		}
	} else logf("Message from noone or empty one!\n");
	lm_message_unref(m);
	return LM_HANDLER_RESULT_REMOVE_MESSAGE;
}

static LmHandlerResult iq_rcvd_cb(LmMessageHandler *handler, LmConnection *connection, LmMessage *m, gpointer data)
{
	LmMessageNode *query, *item;
	
	// TODO: parse other IQs too
	query = lm_message_node_get_child(m->node, "query");
	if (query) {
		item  = lm_message_node_get_child (query, "item");
		while (item) {
			rosteritem *ri = g_new(rosteritem,1);
			ri->jid = g_strdup(lm_message_node_get_attribute(item, "jid"));
			ri->type = GUY;
			roster_add(ri);
			item = item->next;
		}
		lm_message_node_unref(query);
	}
	lm_message_unref(m);
	return LM_HANDLER_RESULT_REMOVE_MESSAGE;
}

static void connection_auth_cb(LmConnection *connection, gboolean success, void *data)
{
	LmMessage *m;
	gboolean result;

	if (success)
	{
		m = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_AVAILABLE);
		lm_connection_send(connection, m, NULL);
		lm_message_unref(m);

		m = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_GET);
		// TODO: moar c011b4ckz!
		lm_connection_register_message_handler(connection, lm_message_handler_new(iq_rcvd_cb, NULL, NULL ), LM_MESSAGE_TYPE_IQ, LM_HANDLER_PRIORITY_NORMAL);
		lm_connection_register_message_handler(connection, lm_message_handler_new(presence_rcvd_cb, NULL, NULL), LM_MESSAGE_TYPE_PRESENCE, LM_HANDLER_PRIORITY_NORMAL);
		lm_connection_register_message_handler(connection, lm_message_handler_new(message_rcvd_cb, NULL, NULL), LM_MESSAGE_TYPE_MESSAGE, LM_HANDLER_PRIORITY_NORMAL);
		
		lm_message_node_set_attribute(lm_message_node_add_child(m->node, "query", NULL), "xmlns", "jabber:iq:roster");
        	result = lm_connection_send(connection, m, NULL);
        	lm_message_unref (m);
	} else
	{
		logstr("Authentication failed");
	}
}

static void connection_open_cb (LmConnection *connection, gboolean success, void *data)
{
	if (!success)
		logstr("Cannot open connection");
	if (!lm_connection_authenticate (connection, 
					 (gchar *) g_hash_table_lookup(config, "username"),
					 (gchar *) g_hash_table_lookup(config, "password"),
					 (gchar *) g_hash_table_lookup(config, "resource"),
					 (LmResultFunction) connection_auth_cb, NULL, g_free, NULL))
		logstr("lm_connection_authenticate failed");
}

extern void xmpp_connect()
{
	connection = lm_connection_new_with_context ((gchar *) g_hash_table_lookup(config, "server"), context);
        if (!lm_connection_open (connection, (LmResultFunction) connection_open_cb, NULL, g_free, NULL))
		logstr("lm_connection_open failed");
} 

void xmpp_send(const gchar *to, const gchar *body)
{
	LmMessage *m;
	rosteritem *ri;
	gchar *log_str;

	m = lm_message_new(to, LM_MESSAGE_TYPE_MESSAGE);
	ri = g_hash_table_lookup(roster, to);
	if(ri) {
		if(ri->type == MUC) lm_message_node_set_attribute(m->node, "type", "groupchat");
		log_str = g_strdup_printf("%s: %s\n", (gchar *) g_hash_table_lookup(config, "muc_default_nick"), body);
		g_array_append_vals(ri->log, log_str, strlen(log_str));
		g_array_append_vals(ri->log, "\n", 2);
	}
	lm_message_node_add_child(m->node, "body", body);
	lm_connection_send(connection, m, NULL);
	lm_message_unref(m);
}
