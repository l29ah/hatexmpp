CFLAGS = -Wall `pkg-config fuse loudmouth-1.0 --cflags` -ggdb -DDEBUG -O0
LDFLAGS = `pkg-config fuse loudmouth-1.0 --libs`

all: hatexmpp 

hatexmpp: version.o hatexmpp.o fuse.o xmpp.o 

hatexmpp.o: hatexmpp.c common.h xmpp.h

fuse.o: fuse.c fuse.h common.h

xmpp.o: xmpp.c xmpp.h common.h

version.o: version.c

version.c: hatexmpp.c common.h xmpp.c xmpp.h fuse.c fuse.h
	echo "char HateXMPP_ver[] = "\"0.1.`git log --pretty=oneline | wc -l`\""; char * getversion(void) { return HateXMPP_ver; }" > version.c

test: hatexmpp
	mkdir -p test/fs
	mv hatexmpp test
	cd test;./hatexmpp fs -d

clean:
	rm *.o
	rm version.c
	rm hatexmpp
