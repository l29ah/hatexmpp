#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

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
		stbuf->st_mode = S_IFREG | 0444;
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
		/* Get roster, mucs and stuff */
	}
	/*
	else if (we're in the muc) {
		make participiant list
		make chat
	} */
	else return -ENOENT;
	return 0;
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
//	size_t len;
//	(void) fi;
//	if(strcmp(path, hello_path) != 0)
	if (isJID(path)) {
		/* read some messages */
		return 0;
	}
	return -ENOENT;
	/*
	len = strlen(hello_str);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, hello_str + offset, size);
	} else
		size = 0;

	return size;
	*/
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

int init(void) {
	fuse_main(0, NULL, &oper, NULL);
	return 0;
}
