CFLAGS+=-std=gnu99
ifdef DEBUG
	CFLAGS+=-ggdb -DDEBUG -O0 -Wall -pedantic
else
	CFLAGS+=-DNDEBUG
endif

CFLAGS+=$(shell pkg-config fuse loudmouth-1.0 --cflags)
LDFLAGS+=$(shell pkg-config fuse loudmouth-1.0 --libs)

# FIXME: the Makefile doesn't check for the changes in the header files

all: hatexmpp

hatexmpp: version.o hatexmpp.o fuse.o xmpp.o

hatexmpp.o: hatexmpp.c common.h
fuse.o: fuse.c common.h
xmpp.o: xmpp.c common.h

version.c:
	echo "char HateXMPP_ver[] = "\"0.2-`git log -n1 --pretty=format:%H`\""; char * getversion(void) { return HateXMPP_ver; }" > version.c

.PHONY:	clean

clean:
	rm *.o
	rm version.c
	rm hatexmpp

install: hatexmpp
	install hatexmpp /usr/bin

