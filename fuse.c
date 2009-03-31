#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "common.h"

int isMUC(const char *path) {
	return 1;
}

int isJID(const char *path) {
	return 1;
}

static int fsgetattr(const char *path, struct stat *stbuf)
{
	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return 0;
	} 
	if (strcmp(path, "/ctl") == 0) {
		stbuf->st_mode = S_IFREG | 0200;
		stbuf->st_nlink = 1;
		stbuf->st_size = 0;
		return 0;
	}
        if (strcmp(path, "/log") == 0) {
                stbuf->st_mode = S_IFREG | 0400;
                stbuf->st_nlink = 1;
                stbuf->st_size = 0;
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
		/* Get roster, mucs and stuff */
		return 0;
	}
	if (isMUC(path)) {
		/*
		make participiant list
		*/
		filler(buf, "chat", NULL, 0);
		return 0;
	}
	return -ENOENT;
}

static int fsopen(const char *path, struct fuse_file_info *fi)
{
	if (isJID(path))
		return 0;

	return -ENOENT;
}

static int fsread(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	size_t i;
	if (isJID(path)) {
		/* read some messages */
		return 0;
	}
	if (strcmp(path, "/log") == 0) {
		for (i = 0; i < size; i++) {
			buf[i] = 'a';
		}
		return size;
	}
	return -ENOENT;
}

static int fswrite(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	if (strcmp(path, "/ctl") == 0) {
		/* call mum */
		return 0;
	} else return 0;
}

static struct fuse_operations oper = {
	.getattr	= fsgetattr,
	.readdir	= fsreaddir,
	.open		= fsopen,
	.read		= fsread,
	.write		= fswrite,
};

int fsinit(void *arg) {
	int argc;
	char **argv;

	argc = ((fsinit_arg*)arg)->c;
	argv = ((fsinit_arg*)arg)->v;
	fuse_main(argc, argv, &oper, NULL);
	return 0;
}
