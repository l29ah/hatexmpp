#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/un.h>

typedef struct sintlist {
  int v;
  struct sintlist *n;
} intlist;

typedef struct {
  int mucid;
  intlist *jids;
} muc;

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

/* makemuc should get list of participiants to makejid everybody */
muc makemuc(char *Name) {
	snprintf(tmppath, _POSIX_PATH_MAX, "%s/%s", path, striplower(Name));
	mkdir(tmppath, S_IRWXU);
}

int makejid(char *Name) {

}

//void updateroster(roster) {}
