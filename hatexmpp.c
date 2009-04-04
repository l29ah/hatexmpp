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

	return sg;
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
}

gchar *conf_read(GKeyFile *cf, gchar *section, gchar *key, gchar *def)
{
	gchar *val = g_key_file_get_string(cf, section, key, NULL);
	if (!val)
		return g_strdup(def);
	return val;
}

int main (int argc, char **argv)
{
	pthread_t fsthread;
	struct fuse_args par = FUSE_ARGS_INIT(argc, argv);
	
	LogBuf = g_array_sized_new(FALSE, FALSE, 1, 512);
	logf("hatexmpp v%s is going up\n", HateXMPP_ver);
	roster = g_hash_table_new(g_str_hash, g_str_equal);
	pthread_create(&fsthread, NULL, fsinit, (void *)&par); 
	
	GKeyFile *cf = g_key_file_new();
	if (!g_key_file_load_from_file(cf, DEFAULT_CONFIG, G_KEY_FILE_KEEP_COMMENTS, NULL)) {
		g_error("Couldn't read config file %s\n", DEFAULT_CONFIG);		
		return -1;
	}
	config = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(config, "server", conf_read(cf, "login", "server", ""));
	g_hash_table_insert(config, "username", conf_read(cf, "login", "username", ""));
	g_hash_table_insert(config, "password", conf_read(cf, "login", "password", ""));
	g_hash_table_insert(config, "resource", conf_read(cf, "login", "resource", ""));
	g_hash_table_insert(config, "muc_default_nick", conf_read(cf, "login", "muc_default_nick", ""));
	g_key_file_free(cf);

	logstr("fuse is going up\n");
	context = g_main_context_new();
	xmpp_connect();
	logstr("server connected\n");
        main_loop = g_main_loop_new (context, FALSE);
        g_main_loop_run (main_loop);
	//sleep(-1);	/* I don't want to die so soon */
	free_all();
        return 0;
}
