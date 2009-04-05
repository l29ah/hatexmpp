#include "common.h"

#define XMPP_MUC_XMLNS "http://jabber.org/protocol/muc"

void xmpp_send(const gchar *to, const gchar *body);

LmConnection *connection;

gchar *get_resource(const gchar *jid)
{
	gchar *res;
	res = strchr(jid,'/');
	if (res)
		return g_strdup(res+1);
	return NULL;
}

gchar *get_jid(const gchar *jid)
{
	gchar *ch;
	ch = strchr(jid, '/');
	if (ch) 
		return g_strndup(jid, ch - jid);
	else
		return g_strdup(jid);
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

int partmuc(const char *jid, const char *nick, const char *leave) {
        LmMessage *m;
	gchar *to;

	if (!nick) nick = (gchar *) g_hash_table_lookup(config, "muc_default_nick");
	to = g_strdup_printf("%s/%s", jid, nick);
	m = lm_message_new_with_sub_type(to, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_UNAVAILABLE);
	if (leave) lm_message_node_add_child(m->node, "status", leave);
	lm_connection_send(connection, m, NULL);
	lm_message_unref(m);
	return 0;
}

int joinmuc(const char *jid, const char *password, const char *nick) {
	LmMessage *m;
	LmMessageNode *node;
	gchar *to;

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

	addri(jid, NULL, MUC);
	g_free(to);
	return 0;
}

void add_resource(rosteritem *ri, gchar *res, unsigned presence) {
	resourceitem *r;
	r = g_hash_table_lookup(ri->resources, res);
	if (r) {
		if (r->name) g_free(r->name);
		r->name = g_strdup(res);
		r->presence = presence;
	}
	else {
		r = g_malloc(sizeof(resourceitem));
		r->name = g_strdup(res);
		r->presence = presence;
		g_hash_table_insert(ri->resources, res, r);
	}
}

static LmHandlerResult presence_rcvd_cb(LmMessageHandler *handler, LmConnection *connection, LmMessage *m, gpointer data)
{
	gchar *from, *jid, *res, *type;
	rosteritem *ri;

	from = (gchar *) lm_message_node_get_attribute(m->node, "from");
	jid = get_jid(from);
	res = get_resource(from);
	ri = g_hash_table_lookup(roster, jid);

	if (ri && res)	{
		// TODO: do something better with presence
		type = (gchar *) lm_message_node_get_attribute(m->node, "type");
		if (type && (strcmp(type, "unavailable") == 0)) {
			logf("Deleting resource %s from %s\n", res, jid);
			g_hash_table_remove(ri->resources, res);	
		}
		else {
			logf("Adding resource %s to %s\n", res, jid);
			add_resource(ri, res, PRESENCE_ONLINE);
		}
	} else
		logf("Presence from unknown (%s), ignoring\n", from);
	lm_message_unref(m);
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
			log_str = g_strdup_printf("%d %s: %s\n", (unsigned)ri->lastmsgtime, jid, body);
			g_array_append_vals(ri->log, log_str, strlen(log_str));
		} else {
			logf("%s isn't in roster, ignoring message", jid);
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
	if (!query) {
		lm_message_unref(m);
		return LM_HANDLER_RESULT_REMOVE_MESSAGE;
	}
	if (strcmp(lm_message_node_get_attribute(m->node, "type"), "get") == 0) {
		LmMessage *msg = lm_message_new_with_sub_type(lm_message_node_get_attribute(m->node, "from"), LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_RESULT);
		gchar *xmlns;
		xmlns = (gchar *) lm_message_node_get_attribute(query, "xmlns");
		lm_message_node_set_attribute(msg->node, "id", (gchar *) lm_message_node_get_attribute(m->node, "id"));
		query = lm_message_node_add_child(msg->node, "query", NULL);
		lm_message_node_set_attribute(query, "xmlns", xmlns);
		
		// Saying our version
		if (strcmp(xmlns, "jabber:iq:version") == 0) {
			lm_message_node_add_child(query, "name", PROGRAM_NAME);
			lm_message_node_add_child(query, "version", HateXMPP_ver);
			// TODO: Make uname work 
			//struct utsname *buf = g_malloc(sizeof utsname);
			//uname(buf);
			lm_message_node_add_child(query, "os", "My Awesome OS v2.0");
			//g_free(buf);
			lm_connection_send(connection, msg, NULL);
		}

		// Saying our time
		else if (strcmp(xmlns, "jabber:iq:time") == 0) {
			time_t now;
			char buf[50];
			time(&now);
			strftime(buf, 50, "%Y-%m-%dT%H:%M:%S", (struct tm *) gmtime(&now));
			struct tm *t = localtime(&now);
			lm_message_node_add_child(query, "utc", buf);
			lm_message_node_add_child(query, "tz", t->tm_zone);
			strftime(buf, 50, "%c", (struct tm *) localtime(&now));
			lm_message_node_add_child(query, "display", buf);
			lm_connection_send(connection, msg, NULL);
		}

		// Saying our discovery info
		else if (strcmp(xmlns, "http://jabber.org/protocol/disco#info") == 0) {		
			lm_message_node_set_attribute(lm_message_node_add_child(query, "feature", NULL), "var", "http://jabber.org/protocol/disco#info");		
			lm_message_node_set_attribute(lm_message_node_add_child(query, "feature", NULL), "var", "jabber:iq:version");
			lm_message_node_set_attribute(lm_message_node_add_child(query, "feature", NULL), "var", "jabber:iq:time");
			lm_connection_send(connection, msg, NULL);
		}
		
		// Otherwise, feature not supported
		else {	
			LmMessageNode *error = lm_message_node_add_child(msg->node, "error", NULL);
			lm_message_node_set_attribute(error, "type", "cancel");
			lm_message_node_set_attribute(error, "code", "501");
			lm_message_node_set_attribute(lm_message_node_add_child(error, "feature-not-implemented", NULL), "xmlns", "urn:ietf:params:xml:ns:xmpp-stanzas");
			lm_connection_send(connection, msg, NULL);
			lm_message_node_unref(error);
		}
		lm_message_node_unref(query);
		lm_message_unref(msg);
		lm_message_unref(m);
		return LM_HANDLER_RESULT_REMOVE_MESSAGE;
	}
	if ((strcmp(lm_message_node_get_attribute(m->node, "type"), "result") == 0) ||
	    (strcmp(lm_message_node_get_attribute(m->node, "type"), "set") == 0)) {
		if (strcmp(lm_message_node_get_attribute(query, "xmlns"), "jabber:iq:roster") == 0) {
			item  = lm_message_node_get_child (query, "item");
			while (item) {
				addri(lm_message_node_get_attribute(item, "jid"), NULL, GUY);
				item = item->next;
			}
		}
		lm_message_node_unref(query);
		lm_message_unref(m);
		return LM_HANDLER_RESULT_REMOVE_MESSAGE;
	}
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
		lm_message_node_set_attribute(lm_message_node_add_child(m->node, "query", NULL), "xmlns", "jabber:iq:roster");
        	result = lm_connection_send(connection, m, NULL);
        	lm_message_unref (m);
	} else
	{
		logstr("Authentication failed!\n");
	}
}

static void connection_open_cb (LmConnection *connection, gboolean success, void *data)
{
	if (!success)
		logstr("Cannot open connection\n");
	if (!lm_connection_authenticate (connection, 
					 (gchar *) g_hash_table_lookup(config, "username"),
					 (gchar *) g_hash_table_lookup(config, "password"),
					 (gchar *) g_hash_table_lookup(config, "resource"),
					 (LmResultFunction) connection_auth_cb, NULL, g_free, NULL))
		logstr("lm_connection_authenticate failed\n");
}

void connection_close_cb (LmConnection *connection, LmDisconnectReason reason, gpointer data)
{
	gchar *str;
	switch (reason) {
	case LM_DISCONNECT_REASON_OK:
		str = "User requested disconnect.";
		break;
	case LM_DISCONNECT_REASON_PING_TIME_OUT:
		str = "Connection to the server timed out";
		break;
	case LM_DISCONNECT_REASON_HUP:
		str = "The socket emitted that the connection was hung up.";
		break;
	case LM_DISCONNECT_REASON_ERROR:
		str = "A generic error somewhere in the transport layer.";
		break;
	case LM_DISCONNECT_REASON_RESOURCE_CONFLICT:
		str = "Another connection was made to the server with the same resource.";
		break;
	case LM_DISCONNECT_REASON_INVALID_XML:
		str = "Invalid XML was sent from the client.";
		break;
	case LM_DISCONNECT_REASON_UNKNOWN:
		str = "An unknown error.";
	}

	logf("Disconnected. Reason: %s\n", str);
	g_free(str);
}

void xmpp_connect() {
	connection = lm_connection_new_with_context ((gchar *) g_hash_table_lookup(config, "server"), context);
	// TODO: moar c011b4ckz!
	lm_connection_register_message_handler(connection, lm_message_handler_new(iq_rcvd_cb, NULL, NULL ), LM_MESSAGE_TYPE_IQ, LM_HANDLER_PRIORITY_NORMAL);
	lm_connection_register_message_handler(connection, lm_message_handler_new(presence_rcvd_cb, NULL, NULL), LM_MESSAGE_TYPE_PRESENCE, LM_HANDLER_PRIORITY_NORMAL);
	lm_connection_register_message_handler(connection, lm_message_handler_new(message_rcvd_cb, NULL, NULL), LM_MESSAGE_TYPE_MESSAGE, LM_HANDLER_PRIORITY_NORMAL);
	lm_connection_set_disconnect_function(connection, connection_close_cb, NULL, g_free);		
        if (!lm_connection_open (connection, (LmResultFunction) connection_open_cb, NULL, g_free, NULL))
		logstr("lm_connection_open failed\n");
} 

void xmpp_send(const gchar *to, const gchar *body)
{
	LmMessage *m;
	rosteritem *ri;

	m = lm_message_new(to, LM_MESSAGE_TYPE_MESSAGE);
	ri = g_hash_table_lookup(roster, to);
	if(ri) {
		if(ri->type == MUC) {
			lm_message_node_set_attribute(m->node, "type", "groupchat");
			if (strncmp(body, "/leave", 6) == 0) {
				partmuc(to, NULL, body+7);
				g_hash_table_remove(roster, to);
				lm_message_unref(m);
				return;
			}
		}
	}
	lm_message_node_add_child(m->node, "body", body);
	lm_connection_send(connection, m, NULL);
	lm_message_unref(m);
}

void xmpp_add_to_roster(const gchar *jid) {
// TODO: this may work, but better to follow protocol. XMPP is so XMPP!
	if(!g_hash_table_lookup(roster, jid)) {
		logf("Adding contact %s to roster\n", jid);
		LmMessage *msg = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_SET);
		LmMessageNode *query = lm_message_node_add_child(msg->node, "query", NULL);
		lm_message_node_set_attribute(query, "xmlns", "jabber:iq:roster");
		LmMessageNode *item = lm_message_node_add_child(query, "item", NULL);
		lm_message_node_set_attribute(item, "jid", jid);
		lm_message_node_set_attribute(item, "name", get_nick(jid));
		lm_connection_send(connection, msg, NULL);
		lm_message_node_unref(item);
		lm_message_node_unref(query);
		lm_message_unref(msg);
		// ask subcription
		msg = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_SUBSCRIBE);
		lm_connection_send(connection, msg, NULL);
	}

	return;
}

void xmpp_disconnect() {
	if (lm_connection_is_open(connection)) {
		lm_connection_close(connection, NULL);
		g_hash_table_remove_all(roster);
	}
}
