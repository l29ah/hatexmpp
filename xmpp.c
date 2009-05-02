#include "common.h"

#define XMPP_MUC_XMLNS "http://jabber.org/protocol/muc"
#define XMPP_DISCO_XMLNS "http://jabber.org/protocol/disco#info"

void xmpp_send(const gchar *to, const gchar *body);

LmConnection *connection;

#ifdef PROXY
LmProxy *proxy;
#endif

void filterjunk(gchar *str) {
	size_t f, t, len = strlen(str);
	for(f = 0, t = 0; f < len; f++) {
		if ( (unsigned) str[f] >= 0xc0 && f+1 < len && (unsigned) str[f+1] >= 0x80) {
			str[t++] = str[f];
			str[t++] = str[f+1];
			f++;
		}
		if (str[f] >= ' ' || str[f] == '\n' || str[f] == '\r' || str[f] == '\t' ||
			((str[f] >= 192) && (f+1 < len) && (str[f+1] >= 128))) {
			str[t++] = str[f];
		}
	}
	str[t] = 0;
}

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

int banmuc(const char *mucjid, const char *who) {
	if (connection_state != ONLINE) return -1;
	LmMessage *msg = lm_message_new_with_sub_type(mucjid, LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_SET);
	LmMessageNode *query = lm_message_node_add_child(msg->node, "query", NULL);
	lm_message_node_set_attribute(query, "xmlns", "http://jabber.org/protocol/muc#admin");
	LmMessageNode *child = lm_message_node_add_child(query, "item", NULL);
	lm_message_node_set_attributes(child, "affiliation", "outcast", "jid", who);
	lm_connection_send(connection, msg, NULL);
}

/* TODO
int devoice(const char *mucjid; const char *who) {
	LmMessage *msg = lm_message_new_with_sub_type(mucjid, LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_SET);
	LmMessageNode *query = lm_message_node_add_child(msg->node, "query", NULL);
        lm_message_node_set_attribute(query, "xmlns", "http://jabber.org/protocol/muc#admin");
	LmMessageNode *child = lm_message_node_add_child(query, "name", str);
}
*/

int partmuc(const char *jid, const char *nick, const char *leave) {
        LmMessage *m;
	if (connection_state != ONLINE) return -1;
	gchar *to;

	if (!nick) nick = (gchar *) g_hash_table_lookup(config, "muc_default_nick");
	to = g_strdup_printf("%s/%s", jid, nick);
	m = lm_message_new_with_sub_type(to, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_UNAVAILABLE);
	if (leave) lm_message_node_add_child(m->node, "status", leave);
	lm_connection_send(connection, m, NULL);
	lm_message_unref(m);
	g_free(to);
	return 0;
}

int joinmuc(const gchar *jid, const gchar *password, const gchar *nick) {
	if (connection_state != ONLINE) return -1;
	LmMessage *m;
	LmMessageNode *node;
	gchar *to;

	if (!nick) nick = (gchar *) g_hash_table_lookup(config, "muc_default_nick");
	to = g_strdup_printf("%s/%s", jid, nick);
	m = lm_message_new_with_sub_type(to, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_AVAILABLE);
	node = lm_message_node_add_child(m->node, "x", NULL);
	lm_message_node_set_attribute(node, "xmlns", XMPP_MUC_XMLNS);

	if (password) {
	        lm_message_node_add_child (node, "password", password);
	}

	lm_connection_send(connection, m, NULL);
	lm_message_unref(m);

	rosteritem *ri = addri(jid, NULL, MUC);
	if (ri) ri->self_resource->name = g_strdup(nick);
	g_free(to);
	return 0;
}

void add_resource(rosteritem *ri, const gchar *res, const unsigned presence) {
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
		g_hash_table_insert(ri->resources, g_strdup(res), r);
	}
}

static LmHandlerResult presence_rcvd_cb(LmMessageHandler *handler, LmConnection *connection, LmMessage *m, gpointer data)
{
	gchar *from, *jid, *res, *type;
	rosteritem *ri;

	from = (gchar *)lm_message_node_get_attribute(m->node, "from");
	jid = get_jid(from);
	res = get_resource(from);
	ri = g_hash_table_lookup(roster, jid);
	// TODO: do something with this awful if's logic
	if (ri)	{
		// TODO: do something better with presence
		type = (gchar *) lm_message_node_get_attribute(m->node, "type");
		if (type && (strcmp(type, "subscribe") == 0)) {
			// always agree with subscription requests, myabe do something better in future
			eventf("subscr_request %s", from);
			lm_connection_send(connection, lm_message_new_with_sub_type(from, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_SUBSCRIBED), NULL);
		}
		if (type && (strcmp(type, "unavailable") == 0) && res) {
			logf("Deleting resource %s from %s\n", res, jid);
			eventf("del_resource %s/%s", jid, res);
			if (ri->type == MUC && !g_hash_table_lookup(config, "raw_logs")) {
				gchar *log_str = g_strdup_printf("%d * %s has left the room\n", (unsigned) time(NULL), res);
				g_array_append_vals(ri->log, log_str, strlen(log_str));
				g_hash_table_remove(ri->resources, res);
			}
		} else {
			resourceitem *rr;
			if (!res) 
				rr = ri->self_resource;
			else
				rr = g_hash_table_lookup(ri->resources, res);
			if (rr) {
				logf("Changing status of %s/%s\n", jid, res);
				eventf("ch_presence %s", from);
				// maybe this will do smth in future
			}
			else {
				logf("Adding resource %s to %s\n", res, jid);
				eventf("add_resource %s/%s", jid, res);
				add_resource(ri, res, PRESENCE_ONLINE);
				if (ri->type == MUC && !g_hash_table_lookup(config, "raw_logs")) {
					LmMessageNode *child = lm_message_node_find_child(m->node, "item");
					gchar *log_str;
					if (child) {
						log_str = g_strdup_printf("%d * %s (%s) has entered the room\n", (unsigned) time(NULL), res, lm_message_node_get_attribute(child, "jid"));
					} else {
						log_str = g_strdup_printf("%d * %s has entered the room\n", (unsigned) time(NULL), res);
					}
					g_array_append_vals(ri->log, log_str, strlen(log_str));
					g_free(log_str);
				}
			}
		}
	} else
		logf("Presence from unknown (%s), ignoring\n", jid);
	lm_message_unref(m);
	return LM_HANDLER_RESULT_REMOVE_MESSAGE;
}


static LmHandlerResult message_rcvd_cb(LmMessageHandler *handler, LmConnection *connection, LmMessage *m, gpointer data)
{
	const gchar *from, *to, *body, *jid;
	gchar *log_str;
	rosteritem *ri;

	from = lm_message_node_get_attribute(m->node, "from");
	to = lm_message_node_get_attribute(m->node, "to");
	body = lm_message_node_get_value(lm_message_node_get_child(m->node, "body"));
	if (from && body) {
		logf("Message from %s to %s: %s\n", from, to, body );
		eventf("msg %s %s %s", from, to, body);
		jid = get_jid((gchar *) from);
		ri = g_hash_table_lookup(roster, jid);
		if(ri) {
			ri->lastmsgtime = time(NULL);
			if (ri->type == MUC)
				jid = get_resource(from);
			if (g_hash_table_lookup(config, "raw_logs"))
				g_array_append_vals(ri->log, body, strlen(body) + 1);		// +1 for delimiting messages by \0
			else {
				log_str = g_strdup_printf("%d %s: %s\n", (unsigned)ri->lastmsgtime, jid, body);
				g_array_append_vals(ri->log, log_str, strlen(log_str));
				g_free(log_str);
			}
		} else {
			logf("%s isn't in roster, ignoring message", jid);
		}
	} else logf("Message from nobody or empty one!\n");
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
	gchar *xmlns = (gchar *) lm_message_node_get_attribute(query, "xmlns");
	gchar *type = (gchar *) lm_message_node_get_attribute(m->node, "type");
	
	// working with roster
	if (strcmp(xmlns, "jabber:iq:roster") == 0) {
		item  = lm_message_node_get_child (query, "item");
		rosteritem *ri;
		gchar *jid;
		while (item) {
			jid = (gchar *) lm_message_node_get_attribute(item, "jid");
			if (strcmp(lm_message_node_get_attribute(item, "subscription"), "remove") == 0) {
				g_hash_table_remove(roster, jid);
			}
			else {
				ri = g_hash_table_lookup(roster, jid);
				if (!ri) addri(jid, NULL, GUY);
			}
			item = item->next;
		}
	
		// say our presence only after receiving roster, not adding contact or smth
		if (strcmp(lm_message_node_get_attribute(m->node, "type"), "result") == 0) {
			LmMessage *msg = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_AVAILABLE);
			lm_connection_send(connection, msg, NULL);
			lm_message_unref(msg);
		}
		lm_message_node_unref(query);
	} else if (strcmp(type, "get") == 0) {
		LmMessage *msg = lm_message_new_with_sub_type(lm_message_node_get_attribute(m->node, "from"), LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_RESULT);
		lm_message_node_set_attribute(msg->node, "id", (gchar *)lm_message_node_get_attribute(m->node, "id"));
		query = lm_message_node_add_child(msg->node, "query", NULL);
		lm_message_node_set_attribute(query, "xmlns", xmlns);
		
		// Saying our version
		if (strcmp(xmlns, "jabber:iq:version") == 0) {
			char *str;

			str = g_hash_table_lookup(config, "jiv_name");
			if (!str) str = PROGRAM_NAME;
			lm_message_node_add_child(query, "name", str);

			str = g_hash_table_lookup(config, "jiv_version");
			if (!str) str = HateXMPP_ver;
			lm_message_node_add_child(query, "version", str);

			str = g_hash_table_lookup(config, "jiv_os");
			if (!str) str = "My Awesome OS v2.0";
			lm_message_node_add_child(query, "os", str);
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
		
		// Saying last activity time
		else if (strcmp(xmlns, "jabber:iq:last") == 0) {
			lm_message_node_set_attribute(query, "seconds", g_strdup_printf("%d", (int)(time(NULL) - last_activity_time)));
			lm_connection_send(connection, msg, NULL);
		}

		// Saying our discovery info
		else if (strcmp(xmlns, XMPP_DISCO_XMLNS) == 0) {		
			lm_message_node_set_attribute(lm_message_node_add_child(query, "feature", NULL), "var", XMPP_DISCO_XMLNS);		
			lm_message_node_set_attribute(lm_message_node_add_child(query, "feature", NULL), "var", "jabber:iq:version");
			lm_message_node_set_attribute(lm_message_node_add_child(query, "feature", NULL), "var", "jabber:iq:time");
			lm_message_node_set_attribute(lm_message_node_add_child(query, "feature", NULL), "var", "jabber:iq:last");
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
	} else if (strcmp(type, "result") == 0) {
		/* <iq type='result' id='reg2'/>
		 *
		 * and what!?
		 */
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
		m = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_GET);
		lm_message_node_set_attribute(lm_message_node_add_child(m->node, "query", NULL), "xmlns", "jabber:iq:roster");
        	result = lm_connection_send(connection, m, NULL);
        	lm_message_unref (m);
		time(&last_activity_time);
		eventstr("auth_ok");
		connection_state = ONLINE;
	} else
	{
		logstr("Authentication failed!\n");
		eventstr("auth_fail");
		connection_state = OFFLINE;
	}
}

static void connection_open_cb (LmConnection *connection, gboolean success, void *data)
{
	if (!success) {
		connection_state = OFFLINE;
		logstr("Cannot open connection\n");
		eventstr("connect_fail");
		g_main_loop_quit(main_loop);
		return;
	}

	/* TODO: something better */
	if (g_hash_table_lookup(config, "noauth")) {
		logstr("config/noauth exists, not logging in\n");
	} else if (!lm_connection_authenticate (connection, 
					 (gchar *) g_hash_table_lookup(config, "username"),
					 (gchar *) g_hash_table_lookup(config, "password"),
					 (gchar *) g_hash_table_lookup(config, "resource"),
					 (LmResultFunction) connection_auth_cb, NULL, g_free, NULL)) {
		logstr("lm_connection_authenticate failed\n");
		g_main_loop_quit(main_loop);
		return;
	}
	eventstr("connect_ok");
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
	eventf("disconnected %s", str);
	connection_state = OFFLINE;
	g_main_loop_quit(main_loop);
	g_hash_table_remove_all(roster);

	// very stupid auto reconnect
	if (g_hash_table_lookup(config,"auto_reconnect") && (reason != LM_DISCONNECT_REASON_OK)) {
		logstr("Auto reconnecting");
		xmpp_connect();
	}
}

void xmpp_connect() {
	if (connection_state != OFFLINE) return;
	connection_state = CONNECTING;
	connection = lm_connection_new_with_context ((gchar *) g_hash_table_lookup(config, "server"), context);
	// TODO: moar c011b4ckz!
	lm_connection_register_message_handler(connection, lm_message_handler_new(iq_rcvd_cb, NULL, NULL ), LM_MESSAGE_TYPE_IQ, LM_HANDLER_PRIORITY_NORMAL);
	lm_connection_register_message_handler(connection, lm_message_handler_new(presence_rcvd_cb, NULL, NULL), LM_MESSAGE_TYPE_PRESENCE, LM_HANDLER_PRIORITY_NORMAL);
	lm_connection_register_message_handler(connection, lm_message_handler_new(message_rcvd_cb, NULL, NULL), LM_MESSAGE_TYPE_MESSAGE, LM_HANDLER_PRIORITY_NORMAL);
	lm_connection_set_disconnect_function(connection, connection_close_cb, NULL, g_free);

#ifdef PROXY
	gchar *p_serv = g_hash_table_lookup(config, "proxy_server");
	gchar *p_port = g_hash_table_lookup(config, "proxy_port");
	gchar *p_user = g_hash_table_lookup(config, "proxy_username");
	gchar *p_pasw = g_hash_table_lookup(config, "proxy_password");
	if (p_serv && p_port) {
		proxy = lm_proxy_new_with_server(LM_PROXY_TYPE_HTTP, p_serv, atoi(p_port));
		if (p_user && p_pasw) {
			lm_proxy_set_username(proxy, p_user);
			lm_proxy_set_password(proxy, p_pasw);
		}
		lm_connection_set_proxy (connection, proxy);
	}
#endif

	if (!lm_connection_open (connection, (LmResultFunction) connection_open_cb, NULL, g_free, NULL)) {
		logstr("lm_connection_open failed\n");
		g_main_loop_quit(main_loop);
		connection_state = OFFLINE;
	}
} 

void xmpp_send(const gchar *to, const gchar *body) {
	if (connection_state != ONLINE) return;
	LmMessage *m;
	rosteritem *ri;
	ri = g_hash_table_lookup(roster, get_jid(to));
	gchar *b;
	if(ri) {
		/* TODO: NO WAI!
		if (strncmp(body, "/clear", 6) == 0) {
			logf("Clearing log of %s", ri->jid);
			g_array_remove_range(ri->log, 0, ri->log->len-1);
			return;
		}
		*/
		if(ri->type == MUC) {
			if (strchr(to, '/')) 
				m = lm_message_new_with_sub_type(to, LM_MESSAGE_TYPE_MESSAGE, LM_MESSAGE_SUB_TYPE_CHAT);
			else
				m = lm_message_new_with_sub_type(to, LM_MESSAGE_TYPE_MESSAGE, LM_MESSAGE_SUB_TYPE_GROUPCHAT);
		}
		else
			m = lm_message_new(to, LM_MESSAGE_TYPE_MESSAGE);
		
		if (m) {
			time(&last_activity_time);
			b = g_strdup(body);
			filterjunk(b);
			lm_message_node_add_child(m->node, "body", b);
			lm_connection_send(connection, m, NULL);
			lm_message_unref(m);
			g_free(b);
		}
	}
}

void xmpp_add_to_roster(const gchar *jid) {
	if (connection_state != ONLINE) return;
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
		lm_message_unref(msg);
	}

	return;
}

void xmpp_del_from_roster(const gchar *jid) {
	if (connection_state != ONLINE) return;
	rosteritem *ri = g_hash_table_lookup(roster, jid);
	if (ri) {
		logf("Deleting contact %s from roster\n", jid);
		LmMessage *msg = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_SET);
		LmMessageNode *query = lm_message_node_add_child(msg->node, "query", NULL);
		lm_message_node_set_attribute(query, "xmlns", "jabber:iq:roster");
		LmMessageNode *item = lm_message_node_add_child(query, "item", NULL);
		lm_message_node_set_attribute(item, "jid", jid);
		lm_message_node_set_attribute(item, "subscription", "remove");
		lm_connection_send(connection, msg, NULL);
		lm_message_node_unref(item);
		lm_message_node_unref(query);
		lm_message_unref(msg);
		// unsubscribing
		msg = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_UNSUBSCRIBE);
		lm_connection_send(connection, msg, NULL);
		lm_message_unref(msg);
	}
}

void xmpp_disconnect() {
	if (lm_connection_is_open(connection) || (connection_state == ONLINE)) {
		// disconect gracefully
		LmMessage *msg = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_UNAVAILABLE);
		lm_connection_send(connection, msg, NULL);
		lm_message_unref(msg);

		lm_connection_close(connection, NULL);
	}
}

void xmpp_muc_change_nick(const gchar *mucjid, const gchar *nick) {
	if (connection_state != ONLINE) return;
	logf("Change nick in %s to %s\n", mucjid, nick);
	rosteritem *ri = g_hash_table_lookup(roster, mucjid);
	if (ri && nick && (ri->type == MUC)) {
		logf("Change nick in %s to %s\n", mucjid, nick);
		g_free(ri->self_resource->name);
		ri->self_resource->name = g_strdup(nick);
		LmMessage *m = lm_message_new(g_strdup_printf("%s/%s", mucjid, nick), LM_MESSAGE_TYPE_PRESENCE);
//		lm_message_node_set_attr(m->node, "from", g_strdup_printf("%s/%s"));
		lm_connection_send(connection, m, NULL);
		lm_message_unref(m);
	}	
}

void xmpp_register_request_fields() {
	LmMessage *msg = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_GET);
	LmMessageNode *query = lm_message_node_add_child(msg->node, "query", NULL);
	lm_message_node_set_attribute(query, "xmlns", "jabber:iq:register");
	lm_connection_send(connection, msg, NULL);
	lm_message_node_unref(query);
	lm_message_unref(msg);
}

void xmpp_register_request(const char *Username, const char *Password, const char *Email) {
        LmMessage *msg = lm_message_new_with_sub_type(NULL, LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_SET);
	LmMessageNode *query = lm_message_node_add_child(msg->node, "query", NULL);
	LmMessageNode *un = lm_message_node_add_child(query, "username", Username);
	LmMessageNode *p = lm_message_node_add_child(query, "password", Password);
	LmMessageNode *em;
	if (Email) em = lm_message_node_add_child(query, "email", Email);
	lm_message_node_set_attribute(query, "xmlns", "jabber:iq:register");
	lm_connection_send(connection, msg, NULL);
	lm_message_node_unref(query);
	lm_message_node_unref(un);
	lm_message_node_unref(p);
	if (Email) lm_message_node_unref(em);
	lm_message_unref(msg);

}
