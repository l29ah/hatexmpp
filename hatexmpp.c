#include "common.h"
#include "xmpp.h"

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

int addri(const char *jid, GPtrArray *resources, unsigned type) {
	rosteritem *ri;

	logf("Adding %s to roster\n", jid);
	ri = g_new(rosteritem, 1);
	ri->jid = g_strdup(jid);
	if (resources)
		ri->resources = resources;	/* TODO */
	else
		ri->resources = g_ptr_array_new();
	ri->log = g_array_new(FALSE, FALSE, 1);
	ri->type = type;
	g_hash_table_insert(roster, g_strdup(ri->jid), ri);

	return 0;
}

int destroyresource(resourceitem *res) {
	if (res->name) g_free (res->name);
	return 0;
}

int destroyri(rosteritem *RI) {
	if(RI->jid) g_free(RI->jid);
	if(RI->resources) {
		int i;
//		resourceitem *res;
		for (i=0; i < RI->resources->len; i++)
			destroyresource(g_ptr_array_index(RI->resources, i));
		g_ptr_array_free(RI->resources, TRUE);
	}
	if(RI->log) g_array_free(RI->log, TRUE);

	return 0;
}

void free_all()		// trying to make a general cleanup
{
	GHashTableIter iter;
	rosteritem *ri;
	g_hash_table_iter_init (&iter, roster);
	while (g_hash_table_iter_next(&iter, NULL, (gpointer) &ri)) 
		destroyri(ri);
	g_hash_table_unref(roster);
	g_array_free(LogBuf, TRUE);
	g_hash_table_unref(config);
	return;
}

gchar *conf_read(GKeyFile *cf, gchar *section, gchar *key, gchar *def)
{
//	GError *error;
	gchar *val = g_key_file_get_string(cf, section, key, NULL);
	if (!val) return g_strdup(def);
	else return val;
}

int main(int argc, char **argv) {
	return fuse_main(argc, argv, &fuseoper, NULL);
}

void * mainloopthread(void *loop) {
	g_main_loop_run(loop);
	return NULL;
}

