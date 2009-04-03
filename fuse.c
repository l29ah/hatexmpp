#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "common.h"
#include "parser.h"

int isMUC(const char *path) {
	return 0;
}

int isJID(const char *path) {
	return 0;
}

int fileexists(const char *path) {
	if (strcmp(path, "/log") == 0) return 1;
	if (strncmp(path, "/roster/", 8) == 0) return 1;
	return 0;
}
/*
void breakparse(char **linestart, const char *buf, size_t len, const char chr, void callb(const char *)) {
	int i = 0;

	while (1) {
		while(i < len && (buf[i] != chr)) {
			i++;
		}
		if(i == len) return;
		else {
			callb(*linestart);
			*linestart = buf + (++i); 
		}
	}
	return;
}
*/
//void postparse(char **linestart, 


/* FS calls */

static int fscreate(const char *path, mode_t mode, struct fuse_file_info *fi) {
	/* TODO add roster items */
	return 0;
}

static int fsmknod(const char *path, mode_t mode, dev_t type) {
	/* TODO */
	return 0;
}

static int fsmkdir(const char *path, mode_t mode) {
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
	if (strcmp(path, "/ctl") == 0) {
		stbuf->st_mode = S_IFREG | 0200;
		stbuf->st_nlink = 1;
		return 0;
	}
        if (strcmp(path, "/log") == 0) {
                stbuf->st_mode = S_IFREG | 0444;
                stbuf->st_nlink = 1;
 	        stbuf->st_size = LogBuf->len;
                return 0;
	}
	if ((strcmp(path, "/roster") == 0) || 
	    (strcmp(path, "/config") == 0) ||
	    (strcmp(path, "/") == 0)) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return 0;
	}
	if (strncmp(path, "/config/", 8) == 0) {
		path += 8;

		/* TODO */
	}
	if (strncmp(path, "/roster/", 8) == 0) {
		rosteritem *ri;
		GArray *log;

		path += 8;
		stbuf->st_mode = S_IFREG | 0666;
		stbuf->st_nlink = 1;
		ri = g_hash_table_lookup(roster, path);
		if(ri) {
			log = ri->log;
			logf("jid %s log len is %u\n", path, log->len);
			stbuf->st_size = log->len;

		} else return -ENOENT;
		return 0;
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
		filler(buf, "ctl", NULL, 0);
		filler(buf, "log", NULL, 0);
		filler(buf, "config", NULL, 0);
		if (roster) filler(buf, "roster",NULL, 0);
		return 0;
	}
	/* obsolete? */
	/*
	if (isMUC(path)) {
		//make participiant list
		filler(buf, "chat", NULL, 0);
		return 0;
	}
	*/
	if (strcmp(path, "/roster") == 0)
	{
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
// Testing new roster!!!
	GHashTableIter *iter;
	iter=g_malloc(sizeof(GHashTableIter));
	gpointer jid;
	g_hash_table_iter_init (iter, roster);
	while (g_hash_table_iter_next (iter, &jid, NULL))
	{
		filler(buf, jid, NULL, 0);
	}
	

		/* TODO updated roster */
		filler(buf, "anime@conference.jabber.ru", NULL, 0);
		filler(buf, "hatexmpp@conference.jabber.ru", NULL, 0);
		filler(buf, "megabreds@conference.jabber.ru", NULL, 0);
		return 0;
	}
	//return -ENOENT;
	return 0;
}

static int fsopen(const char *path, struct fuse_file_info *fi)
{
	if (fileexists(path))
		return 0;
	/* TODO */
	//return -ENOENT;
	return 0;
}

static int fsread(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
//	size_t i;
	if (isJID(path)) {
		/* TODO: read some messages */
		return 0;
	}
	if (strcmp(path, "/log") == 0) {
		printf("reading log\n");
		memcpy(buf, LogBuf->data + offset, size);
		/* TODO: checks, lock */
		return size;
	}
	if (strncmp(path, "/roster/", 8) == 0) {
		GArray *log;
		rosteritem *ri;

		path += 8;
		ri = g_hash_table_lookup(roster, path);
		log = ri->log;
		if(log) {
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
	if (strcmp(path, "/ctl") == 0) {
		/* TODO: call mum */
		return 0;
	}
	if (strncmp(path, "/roster/", 8) == 0) {
		char *msg;

		path += 8;
		msg = malloc(size + 1);
		memcpy(msg, buf, size);
		msg[size] = 0;
		xmpp_send(path, msg);
		return size;
	}
	return 0;
}

static int fssetxattr(const char *path, const char *a, const char *aa, size_t size, int aaa) {
	return 0;
}


static struct fuse_operations oper = {
	.getattr	= fsgetattr,
	.readdir	= fsreaddir,
	.open		= fsopen,
	.read		= fsread,
	.write		= fswrite,
	.setxattr	= fssetxattr,
	.mkdir		= fsmkdir,
	.mknod		= fsmknod,
	.create		= fscreate,
};

void * fsinit(void *arg) {
	int argc;
	char **argv;

	argc = ((struct fuse_args *)arg)->argc;
	argv = ((struct fuse_args *)arg)->argv;
	fuse_main(argc, argv, &oper, NULL);
	perror("fuse_main terminated");
	exit(1);
	return 0;
}
