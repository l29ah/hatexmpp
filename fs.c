#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/un.h>


#define UNIX_PATH_MAX    108
/*
#undef _POSIX_PATH_MAX
#define _POSIX_PATH_MAX UNIX_PATH_MAX
*/

typedef struct sintlist {
  int v;
  struct sintlist *n;
} intlist;

typedef struct {
  int mucid;
  intlist *jids;
} muc;

static char basepath[_POSIX_PATH_MAX] = "hatexmpp";

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
	static char tmppath[_POSIX_PATH_MAX];

	snprintf(tmppath, _POSIX_PATH_MAX, "%s/%s", basepath, striplower(Name));
	mkdir(tmppath, S_IRWXU);
}

int makejid(char *Name, char *BasePath) {
	int sock;
	static char tmppath[_POSIX_PATH_MAX];
		
	snprintf(tmppath, _POSIX_PATH_MAX, "%s/%s", BasePath, Name);
	sock = makesocket(tmppath);

	return sock;
}

int makesocket(char *Path) {
        int sock;
        struct sockaddr_un addr;

        sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("omg can't create socket");
		exit(1);
	}
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, Path, UNIX_PATH_MAX);

	return sock;
}

void killsocket(int Sock) {
	shutdown(Sock, 2);
}

//void updateroster(roster) {}
