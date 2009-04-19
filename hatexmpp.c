#include "common.h"
#define EVENTS

GMainLoop *main_loop;
GMainContext *context;
GHashTable *config;
GHashTable *roster;

GArray *LogBuf;
int fd_events;
gchar *events_file;
time_t last_activity_time;
enum connection_state_e connection_state;

gchar *eventstr(gchar *str) {	/* TODO const */
#ifdef EVENTS
	if (fd_events <= 0) {
		fd_events = open(events_file, O_WRONLY | O_NONBLOCK);
		#ifdef DEBUG
		logf("open fd_events = %d errno = %d", fd_events, errno);
		#endif
	}
	#ifdef DEBUG
	logf("Event: fd_events = %d, str = %s", fd_events, str);
	#endif
	if (fd_events != -1) {
		write(fd_events, str, strlen(str)+1);
		#ifdef DEBUG
		logf("write to fd_events = %d errno = %d", fd_events, errno);
		#endif
	}
#endif
	return str;
}

inline void logs(const char *msg, size_t len) {
	g_array_append_vals(LogBuf, msg, len);
}

gchar * logstr(gchar *msg) {	/* TODO const */
	size_t len;
	
	g_printf("LOGF: %s",msg);
	len = strlen(msg);
	logs(msg, len);

	return msg;
}

void destroy_resource(resourceitem *resi) {
	if (resi) {
		if (resi->name) g_free(resi->name);
		g_free(resi);
	}
}

rosteritem *addri(const gchar *jid, GHashTable *resources, unsigned type) {
	rosteritem *ri;

	logf("Adding %s to roster\n", jid);
	eventf("add_ri %s %s", jid, (type == MUC) ? "MUC" : "BUDDY" );
	ri = g_new(rosteritem, 1);
	if (ri) {
		ri->jid = g_strdup(jid);
		if (resources)
			ri->resources = resources;	/* TODO */
		else
			ri->resources = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify) destroy_resource);
		ri->log = g_array_new(FALSE, FALSE, 1);
		ri->type = type;
		g_hash_table_insert(roster, g_strdup(jid), ri);
		ri->self_resource = g_new(resourceitem, 1);
	}
	return ri;
}

void destroy_ri(rosteritem *RI) {
	if (!RI) return;
	eventf("del_ri %s", RI->jid);
	if(RI->jid) g_free(RI->jid);
	if(RI->resources) g_hash_table_destroy(RI->resources);
	if(RI->log) g_array_free(RI->log, TRUE);
	if(RI->self_resource) g_free(RI->self_resource);
	g_free(RI);
}

void free_all()		// trying to make a general cleanup
{
	g_hash_table_destroy(roster);
	g_array_free(LogBuf, TRUE);
	g_hash_table_destroy(config);
}

gchar *conf_read(GKeyFile *cf, const gchar *section, const gchar *key, const gchar *def)
{
	gchar *val = g_key_file_get_string(cf, section, key, NULL);

	if (!val)
		return g_strdup(def);
	return val;
}

int main(int argc, char **argv) {
	LogBuf = g_array_sized_new(FALSE, FALSE, 1, 512);
	logf("hatexmpp v%s is going up\n", HateXMPP_ver);
	roster = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify) destroy_ri);
	config = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

	if (argc) {
		events_file = g_strdup_printf("%sevents", argv[1]);
	}

	logf("Events FIFO: %s\n", events_file);
	return fuseinit(argc, argv);
}

