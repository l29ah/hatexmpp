#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

static char path[_POSIX_PATH_MAX] = "hatexmpp";
static char tmppath[_POSIX_PATH_MAX];

static char *striplower(char *s) {
        char *p = NULL;
        for(p = s; p && *p; p++) {
                if(*p == '/') *p = '_';
                *p = tolower(*p);
        }
        return s;
}

void makemuc(char *Name) {
	snprintf(tmppath, _POSIX_PATH_MAX, "%s/%s", path, striplower(Name));
	mkdir(tmppath, S_IRWXU);
}

void makejid(char *Name) {
}
