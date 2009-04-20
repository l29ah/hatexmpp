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
	} else p = g_strdup(path);
	ri = g_hash_table_lookup(roster, p);
	g_free(p);
	return ri;
}

FD * addfd(rosteritem *ri) {
	FD *fd;
	int *id;

	fd = calloc(sizeof(fd), 1);
	fd->id = ++FDn;
	fd->ri = ri;
	fd->writebuf = g_array_new(TRUE, FALSE, 1);
	id = malloc(sizeof(int));
	*id = fd->id;
	g_hash_table_insert(FDt, id, fd);
	return fd;
}

void destroyfd(FD *fd) {
//	g_array_free(fd->readbuf, TRUE);	/* TODO? */
	g_array_free(fd->writebuf, TRUE);
	free(fd);
}

/* FS calls */

static int fstruncate(const char *path, off_t size) {
	return 0;
}

static int fsrmdir(const char *path) {
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
		logf("Make roster!\n");
		if(connection && lm_connection_is_open(connection)) {
			return -EPERM;
		} else {
			pthread_create(&thr, NULL, mainloopthread, NULL);
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
	if (strcmp(path, "/roster") == 0) {
		if (connection_state != OFFLINE) {
			stbuf->st_mode = S_IFDIR | 0755;
			stbuf->st_nlink = 2;
			return 0;
		} else return -ENOENT;
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
			if ((ri->type == MUC) && (strlen(path) == strlen(get_jid(path)))) {
				stbuf->st_mode = S_IFDIR | 0755;
				stbuf->st_nlink = 2;
				return 0;
			}
			stbuf->st_mode = S_IFREG | 0666;	/* TODO fixable perms */
			stbuf->st_nlink = 1;
			//logf("jid %s log len is %u\n", path, ri->log->len);
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
		if (roster && lm_connection_is_open(connection)) filler(buf, "roster",NULL, 0);
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
		if (ri && (ri->type == MUC)) {
			filler(buf, ".", NULL, 0);
			filler(buf, "..", NULL, 0);
			filler(buf, "__chat", NULL, 0);
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
		gchar *val = g_hash_table_lookup(config, path);
		if(val) {
			len = strlen(val);
			if(offset + size < len) {
				memcpy(buf, val + offset, size);
				return size;
			} else {
				memcpy(buf, val + offset, len - offset);
				return len - offset;
			}
		} else return -ENOENT;
	}
	if (strncmp(path, "/roster/", 8) == 0) {
		GArray *log;
		rosteritem *ri;

		path += 8;
		ri = getri(path);
		if(ri && ((strcmp(path, "__nick") == 0) || (strcmp(get_resource(path), "__nick") == 0))) {
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

		if (strrchr(buf,'\n'))
			msg_len--;

		msg = malloc(msg_len+1);
		memcpy(msg, buf, msg_len);
		msg[msg_len] = 0;
		if (res && (strncmp(res, "__nick", 6) == 0 )) 
			xmpp_muc_change_nick(get_jid(path), msg);
		else
			xmpp_send(path, msg);
		free(msg);
		return size;
	}
	if (strncmp(path, "/config/", 8) == 0) {
		path += 8;
		gchar *option = g_strdup(path);
		gchar *val;
		// bool options
		if ((strcmp(option, "events") == 0) ||
			(strcmp(option, "raw_logs")== 0) ||
			(strcmp(option, "auto_reconnect") == 0)) {
			val = g_strdup("1"); // just something :)
		}
		else {
			val = filter_str(g_strndup(buf, size));
		}
		logf("Setting %s = %s\n", option, val);
		g_hash_table_replace(config, option, val);	
		return size;
	}
	return 0;
}

static int fssetxattr(const char *path, const char *a, const char *aa, size_t size, int aaa) {
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
	main_loop = g_main_loop_new(context, FALSE);
	xmpp_connect();
	g_main_loop_run(main_loop);
	return NULL;
}

static void * fsinit(struct fuse_conn_info *conn) {
	context = g_main_context_new();
	FDt = g_hash_table_new_full(g_int_hash, g_int_equal, free, (GDestroyNotify)destroyfd);
	return NULL;
}

int fuseinit(int argc, char **argv) {
	int ret;
//	fifo = open("fs/log", O_WRONLY | O_NONBLOCK);
	logstr("fuse is going up\n");
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

