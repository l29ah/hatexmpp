CFLAGS = `pkg-config fuse loudmouth-1.0 --cflags`
LDFLAGS = `pkg-config fuse loudmouth-1.0 --libs`

all: hatexmpp lovexmpp

lovexmpp: lovexmpp.o

lovexmpp.o: lovexmpp.c

hatexmpp: version.o hatexmpp.o fuse.o xmpp.o 

hatexmpp.o: hatexmpp.c common.h

fuse.o: fuse.c common.h

xmpp.o: xmpp.c common.h

version.o: version.c

version.c: hatexmpp.c common.h xmpp.c fuse.c
	echo "char HateXMPP_ver[] = "\"0.1.`git log --pretty=oneline | wc -l`\""; char * getversion(void) { return HateXMPP_ver; }" > version.c

.PHONY:	test mtest clean debug

clean:
	rm *.o
	rm version.c
	rm hatexmpp
	rm lovexmpp

debug:
	make all CFLAGS='${CFLAGS} -ggdb -DDEBUG -O0 -Wall'

