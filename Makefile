CFLAGS = -Wall `pkg-config fuse loudmouth-1.0 --cflags --libs`
LDFLAGS = `pkg-config fuse loudmouth-1.0 --libs`

all: hatexmpp 

hatexmpp: hatexmpp.o fuse.o xmpp.o

hatexmpp.o: hatexmpp.c common.h xmpp.h

fuse.o: fuse.c common.h

xmpp.o: xmpp.c xmpp.h

clean:
	rm *.o
	rm hatexmpp
