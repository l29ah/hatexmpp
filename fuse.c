#include "common.h"

GHashTable *FDt;
unsigned FDn;
static void * mainloopthread(void *loop);
pthread_t thr;

gchar *filter_str(gchar *str) {
	gchar *ch = strchr(str, '\n');
	if (ch) 
		str[ch-str] = 0;
	g_strstrip(str);
	return str;
}

int fileexists(const char *path) {	/* TODO remove/rewrite */
	if (strcmp(path, "/log") == 0) return 1;
	if (strncmp(path, "/roster/", 8) == 0) return 1;
	return 0;
}

rosteritem * getri(const char *path) {
	char *p;
	rosteritem *ri;

	if (*path == '/')
		path ++;
	gchar *ch = strchr(path, '/');
	if (ch) {
		p = g_strndup(path, ch - path);
	} else {
		p = g_strdup(path);
	}
	ri = g_hash_table_lookup(roster, p);
	g_free(p);
	return ri;
}

FD * addfd(rosteritem *ri) {
	FD *fd;
	int *id;

	fd = g_malloc0(sizeof(fd));
	fd->id = ++FDn;
	fd->ri = ri;
	fd->writebuf = g_array_new(TRUE, FALSE, 1);
	id = g_malloc(sizeof(int));
	*id = fd->id;
	g_hash_table_insert(FDt, id, fd);
	return fd;
}

void destroyfd(FD *fd) {
//	g_array_free(fd->readbuf, TRUE);	/* TODO? */
	g_array_free(fd->writebuf, TRUE);
	g_free(fd);
}

/* FS calls */

static int fstruncate(const char *path, off_t size) {
	return 0;
}

static int fsrmdir(const char *path) {
	if (strcmp(path, "/config") == 0) {
		g_hash_table_destroy(config);
		config = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
		return 0;
	}
	if (strcmp(path, "/roster") == 0) {
		if (connection_state != OFFLINE) {
			xmpp_disconnect();
			return 0;
		}
	}
	if (strncmp(path, "/roster/", 8) == 0) {
		rosteritem *ri;

		path += 8;
		ri = getri(path);
		if(ri) {
			if(ri->type == MUC) {
			  partmuc(path, NULL, "TODO here must be default leave message :-)");
			  g_hash_table_remove(roster, path);
			}
			else logstr("Roster items removal isn't implemented\n");; /* TODO */
			return 0;
		} else return -ENOENT;
	}
	return -EPERM;
}

static int fscreate(const char *path, mode_t mode, struct fuse_file_info *fi) {
	/* TODO add roster items */
	if (strncmp(path, "/roster/", 8) == 0) {
		path += 8;
		xmpp_add_to_roster(path);
	}
	return 0;
}

static int fsmknod(const char *path, mode_t mode, dev_t type) {
	/* TODO */
	return 0;
}

static int fsmkdir(const char *path, mode_t mode) {
	if (strcmp(path, "/roster") == 0) {
		logstr("Make roster!\n");
		if(connection && lm_connection_is_open(connection)) {
			return -EPERM;
		} else {
			pthread_create(&thr, NULL, mainloopthread, NULL);
			usleep(100);
			while (connection_state == CONNECTING) usleep(100);	// FIXME
			return 0;
		}
	}
	if (strncmp(path, "/roster/", 8) == 0) {
		path += 8;
		logf("join conference %s!\n", path);
		joinmuc(path, NULL, NULL);
		return 0;
	}
	return -EPERM;
}

static int fsgetattr(const char *path, struct stat *stbuf)
{
	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/events") == 0) {
		stbuf->st_mode = S_IFIFO | 0444;
		stbuf->st_nlink = 1;
		return 0;
	}
	if (strcmp(path, "/log") == 0) {
    	stbuf->st_mode = S_IFREG | 0666;
		stbuf->st_nlink = 1;
		stbuf->st_size = LogBuf->len;
		return 0;
	}
	if (connection_state != OFFLINE) {
		if (strcmp(path, "/roster") == 0) {
			stbuf->st_mode = S_IFDIR | 0755;
			stbuf->st_nlink = 2;
			return 0;
		}
		if (strcmp(path, "/rawxmpp") == 0) {
    		stbuf->st_mode = S_IFREG | 0222;
			stbuf->st_nlink = 1;
			return 0;
		} 
	}
	if ((strcmp(path, "/config") == 0) ||
	    (strcmp(path, "/") == 0)) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return 0;
	}
	if (strncmp(path, "/config/", 8) == 0) {
		char *conf;
		path += 8;
		stbuf->st_mode = S_IFREG | 0644;
		stbuf->st_nlink = 1;
		conf = g_hash_table_lookup(config, path);
		if(conf) stbuf->st_size = strlen(conf);
		return 0;
	}
	if (strncmp(path, "/roster/", 8) == 0) {
		rosteritem *ri;
		path += 8;
		ri = getri(path);
		if (ri) {
			if (strlen(path) == strlen(get_jid(path))) {
				stbuf->st_mode = S_IFDIR | 0755;
				stbuf->st_nlink = 2;
				return 0;
			}
			stbuf->st_mode = S_IFREG | 0666;	/* TODO fixable perms */
			stbuf->st_nlink = 1;
			stbuf->st_size = ri->log->len;
			return 0;
		}
	}
	return -ENOENT;
}

static int fsreaddir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;

	if (strcmp(path, "/") == 0) {
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
		filler(buf, "events", NULL, 0);
		filler(buf, "log", NULL, 0);
		filler(buf, "config", NULL, 0);
		if (roster && (connection_state == ONLINE)) {
			filler(buf, "roster",NULL, 0);
			filler(buf, "rawxmpp", NULL, 0);
		}
		return 0;
	}
	if (strcmp(path, "/config") == 0) {
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
		GHashTableIter iter;
		gchar *key;
		g_hash_table_iter_init (&iter, config);
		while (g_hash_table_iter_next(&iter, (gpointer *)&key, NULL)) {
			filler(buf, key, NULL, 0);
		}
		filler(buf, "server", NULL, 0);
		filler(buf, "port", NULL, 0);
		filler(buf, "ssl", NULL, 0);

	}
	if (strcmp(path, "/roster") == 0)
	{
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
		
		GHashTableIter iter;
		rosteritem *ri;
		g_hash_table_iter_init (&iter, roster);
		while (g_hash_table_iter_next(&iter, NULL, (gpointer) &ri))
			filler(buf, ri->jid, NULL, 0);
		return 0;
	}
	if (strncmp(path, "/roster/", 8) == 0) {
		path += 8;
		rosteritem *ri;
		
		ri = getri(path);
		if (ri) {
			filler(buf, ".", NULL, 0);
			filler(buf, "..", NULL, 0);
			filler(buf, "__chat", NULL, 0);
			if (ri->type == MUC)
				filler(buf, "__nick", NULL, 0);
			GHashTableIter iter;
			gchar *res;
			g_hash_table_iter_init (&iter, ri->resources);
			while (g_hash_table_iter_next(&iter, (gpointer) &res, NULL)) 
				filler(buf, res, NULL, 0);
		}
	}

	return 0;
}

static int fsopen(const char *path, struct fuse_file_info *fi)
{
	/* FDs are introduced only for writing ops */
	if ((fi->flags & O_WRONLY) || (fi->flags & O_RDWR)) {
	        if (strncmp(path, "/roster/", 8) == 0) {
			rosteritem *ri;

	                path += 8;
                	ri = getri(path);
			if (ri) {
				/* TODO */
                	} 
		}
	}
	/* TODO */
	return 0;
}

static char *get_option(const char *option) {
	if (strcmp(option, "server") == 0) {
		const char *r = lm_connection_get_server(connection);
		if (r) 
			return strdup(r);
	} else if (strcmp(option, "ssl") == 0) {
		if (ssl) {
			if (lm_ssl_get_use_starttls(ssl)) {
				if (lm_ssl_get_require_starttls(ssl)) {
					return strdup("required_starttls");
				} else {
					return strdup("starttls");
				}
			} else {
				return strdup("enabled");
			}
		} else {
			return strdup("disabled");
		}
	} else if (strcmp(option, "port") == 0) {
		int p = lm_connection_get_port(connection);
		return g_strdup_printf("%d", p);
	} else {
		char *r = g_hash_table_lookup(config, option);
		if (r)
			return strdup(r);
	}
	return NULL;
}

static int fsread(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	if (strcmp(path, "/log") == 0) {
//		write(fi->fh, LogBuf->data+offset, size);
		memcpy(buf, LogBuf->data + offset, size);
		/* TODO: checks, lock */
		return size;
	}
	if (strncmp(path, "/config/", 8) == 0) {
		size_t len;

		path += 8;
		char *val = get_option(path);
		if (val) {
			len = strlen(val);
			int retval;
			if (offset + size < len) {
				memcpy(buf, val + offset, size);
				retval = size;
			} else {
				memcpy(buf, val + offset, len - offset);
				retval = len - offset;
			}
			free(val);
			return retval;
		} else return -ENOENT;
	}
	if (strncmp(path, "/roster/", 8) == 0) {
		GArray *log;
		rosteritem *ri;

		path += 8;
		ri = getri(path);
		gchar *resource = get_resource(path);
		if(ri && ((strcmp(path, "__nick") == 0) || (resource && strcmp(resource, "__nick") == 0)) && ri->type == MUC) {
			size_t len = strlen(ri->self_resource->name);
			if(offset + size < len) {
				memcpy(buf, ri->self_resource->name + offset, size);
				return size;
			} else {
				memcpy(buf, ri->self_resource->name + offset, len - offset);
				return len - offset;
			}
		}
		if(ri) {
//			if ((ri->type == MUC) && (strcmp(path + strlen(ri->jid)+1)));
			log = ri->log;
			if(offset + size < log->len) {
				memcpy(buf, log->data + offset, size);
				return size;
			} else {
				memcpy(buf, log->data + offset, log->len - offset);
				return log->len - offset;
			}

		} else return -ENOENT;
	}
	return -ENOENT;
}

static char *prepare_option(const char *option, const char *buf, size_t size) {
	char *val;

	// bool options
	if ((strcmp(option, "events") == 0) ||
		(strcmp(option, "raw_logs")== 0) ||
		(strcmp(option, "auto_reconnect") == 0)) {
		val = strdup("1"); // just something :)
	} else {
		val = filter_str(strndup(buf, size));
	
		bool pr;
		if (strcmp(option, "show") == 0 && 
				strcmp(val, "") &&
				strcmp(val, "away") &&
				strcmp(val, "chat") &&
				strcmp(val, "dnd") &&
				strcmp(val, "xa")) {
			logf("Failed to set %s to incorrect value %s\n", option, val);
			return 0;
		} else if (strcmp(option, "ssl") == 0) {
			/* A kludge to avoid double string checking */
			if (strcmp(val, "disabled") == 0) return (char *)1;
			else if (strcmp(val, "enabled") == 0) return (char *)2;
			else if (strcmp(val, "starttls") == 0) return (char *)3;
			else if (strcmp(val, "required_starttls") == 0) return (char *)4;
			else {
				logf("Failed to set %s to incorrect value %s\n", option, val);
				return 0;
			}
		} else if ((pr = (strcmp(option, "priority") == 0)) ||
				((strcmp(option, "port") == 0))) {
			/* Suggest a better way to validate the integer */
			char *not_ok;
			long long i = strtoll(val, &not_ok, 10);

			if (*not_ok) {
				logf("The %s option must be set to a numeric value, %s received instead!\n", option, val);
				return 0;
			}
			if (pr) {
				if (i != (int8_t)i) {
					logf("Failed to set priority to way too big value %s\n", val);
					return 0;
				}
			} else {
				if (i != (uint16_t)i) {
					logf("Value %s is an incorrect port number!\n", val);
					return 0;
				}
			}
		}
	}
	return val;
}

static void set_option(char *option, char *val) {
	bool used = false;

	// FIXME: in the case of ssl, the argument is not a string but a number
	if (strcmp(option, "ssl")) {
		logf("Setting %s = %s\n", option, val);
	} else {
		logf("Setting %s = %" PRIdPTR "\n", option, (intptr_t)val);
	}

	if (strcmp(option, "server") == 0) {
		lm_connection_set_server(connection, val);
	} else if (strcmp(option, "port") == 0) {
		lm_connection_set_port(connection, atoi(val));
	} else if (strcmp(option, "ssl") == 0) {
		if (val) {
			bool st = (long)val & 2, str = ((long)val & 3) == 3;

			while (!ssl) {
				ssl = lm_ssl_new(NULL, NULL, NULL, NULL);
				assert(ssl);
				lm_connection_set_ssl(connection, ssl);
			}
			lm_ssl_use_starttls(ssl, st, str);
		} else {
			lm_connection_set_ssl(connection, NULL);
			lm_ssl_unref(ssl);
			ssl = NULL;
		}
	} else {
		g_hash_table_replace(config, option, val);
		used = true;
	}
	if (connection_state == ONLINE && (
				strcmp(option, "priority") == 0 ||
				strcmp(option, "show") == 0 ||
				strcmp(option, "status") == 0))
		xmpp_send_presence();
	if (!used) {
		if (strcmp(option, "ssl")) {
			free(val);
		}
		free(option);
	}
}

static int fswrite(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	if (strncmp(path, "/roster/", 8) == 0) {
		char *msg;
		size_t msg_len = size;

		path += 8;
		gchar *res = get_resource(path);
		if (res && (strncmp(res, "__chat", 2) == 0 )) {
			path = get_jid(path);
		}

		msg = g_malloc(msg_len + 1);
		memcpy(msg, buf, msg_len);
		msg[msg_len] = 0;
		if (res && (strncmp(res, "__nick", 6) == 0 )) 
			xmpp_muc_change_nick(get_jid(path), msg);
		else
			xmpp_send(path, msg);
		g_free(msg);
		return size;
	}
	if (strncmp(path, "/config/", 8) == 0) {
		path += 8;
		char *option = g_strdup(path);
		char *val = prepare_option(option, buf, size);
		if (val) {
			set_option(option, val);
			return size;
		} else return 0;
	}
	if ((strcmp(path, "/rawxmpp") == 0) && connection_state == ONLINE) {
		gchar *str = g_strndup(buf, size);
		lm_connection_send_raw(connection, str, NULL);
		g_free(str);
		return size;
	}
	return 0;
}

static int fssetxattr(const char *path, const char *a, const char *aa, size_t size, int aaa) {
	/* Stub. Do we need this? */
	return 0;
}

static int fsunlink(const char *path) {
	if (strncmp(path, "/roster/", 8) == 0) {
		path += 8;
		char *sl = strchr(path, '/');
		if (sl) { /* We're in the conference and wanna ban! */
			char *who = g_strdup(sl + 1);
			char *where = g_strndup(path, who - path);
			where[who - path] = 0;
			// TODO!
			banmuc(where, who);
			g_free(where);
			g_free(who);
		} else {
			xmpp_del_from_roster(path);
		}
	}
	// delete options from /config/, maybe need some checks before actual deleting
	if (strncmp(path, "/config/",8) == 0) {
		path += 8;
		g_hash_table_remove(config, path);
	}
	return 0;
}

static void fsdestroy(void *privdata) {
	if (connection) 
		xmpp_disconnect();
	free_all();
	if(main_loop) {
		g_main_loop_quit(main_loop);
		g_main_destroy(main_loop);
	}
	return;
}

static void * mainloopthread(void *loop) {
	xmpp_connect();
	g_main_loop_run(main_loop);
	return NULL;
}

static void * fsinit(struct fuse_conn_info *conn) {
	FDt = g_hash_table_new_full(g_int_hash, g_int_equal, g_free, (GDestroyNotify)destroyfd);	//TODO check the cast
	return NULL;
}

int fuseinit(int argc, char **argv) {
	int ret;
//	fifo = open("fs/log", O_WRONLY | O_NONBLOCK);
	ret = fuse_main(argc, argv, &fuseoper, NULL);
	return ret;
}

struct fuse_operations fuseoper = {
	.getattr	= fsgetattr,
	.readdir	= fsreaddir,
	.open		= fsopen,
	.read		= fsread,
	.write		= fswrite,
	.setxattr	= fssetxattr,
	.mkdir		= fsmkdir,
	.rmdir		= fsrmdir,
	.mknod		= fsmknod,
	.create		= fscreate,
	.unlink		= fsunlink,
	.init		= fsinit,
	.destroy	= fsdestroy,
	.truncate	= fstruncate,
};

