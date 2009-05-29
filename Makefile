ifdef DEBUG
	CFLAGS+=-ggdb -DDEBUG -O0 -Wall
endif

CFLAGS+=$(shell pkg-config fuse loudmouth-1.0 --cflags)
LDFLAGS+=$(shell pkg-config fuse loudmouth-1.0 --libs)

all: hatexmpp

hatexmpp: version.o hatexmpp.o fuse.o xmpp.o

version.c:
	echo "char HateXMPP_ver[] = "\"0.2-`git log -n1 --pretty=format:%H`\""; char * getversion(void) { return HateXMPP_ver; }" > version.c

.PHONY:	clean

clean:
	rm *.o
	rm version.c
	rm hatexmpp

install: hatexmpp
	install hatexmpp /usr/bin

