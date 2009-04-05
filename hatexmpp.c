#include "common.h"

GMainLoop *main_loop;
GMainContext *context;
GHashTable *config;
GHashTable *roster;

GArray *LogBuf;

inline void logs(const char *msg, size_t len) {
	g_array_append_vals(LogBuf, msg, len);
}

char * logstr(char *msg) {
	size_t len;
	
	g_printf("LOGF: %s",msg);
	len = strlen(msg);
	logs(msg, len);
	
	return msg;
}

void destroy_resource(resourceitem *res) {
	if (res->name) g_free (res->name);
}

int addri(const char *jid, GHashTable *resources, unsigned type) {
	rosteritem *ri;

	logf("Adding %s to roster\n", jid);
	ri = g_new(rosteritem, 1);
	ri->jid = g_strdup(jid);
	if (resources)
		ri->resources = resources;	/* TODO */
	else
		ri->resources = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify) destroy_resource);
	ri->log = g_array_new(FALSE, FALSE, 1);
	ri->type = type;
	g_hash_table_insert(roster, g_strdup(ri->jid), ri);
	return 0;
}

void destroy_ri(rosteritem *RI) {
	if(RI->jid) g_free(RI->jid);
	if(RI->resources) g_hash_table_destroy(RI->resources);
	if(RI->log) g_array_free(RI->log, TRUE);
}

void free_all()		// trying to make a general cleanup
{
	g_hash_table_destroy(roster);
	g_array_free(LogBuf, TRUE);
// It's wrong not to destroy this hash, but otherwise it segfaults
//	g_hash_table_destroy(config);
}

gchar *conf_read(GKeyFile *cf, gchar *section, gchar *key, gchar *def)
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
	return fuseinit(argc, argv);
}

