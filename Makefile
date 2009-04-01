CFLAGS = -Wall `pkg-config fuse loudmouth-1.0 --cflags --libs` -DDEBUG
LDFLAGS = `pkg-config fuse loudmouth-1.0 --libs`

all: hatexmpp 

hatexmpp: hatexmpp.o parser.o fuse.o xmpp.o

hatexmpp.o: hatexmpp.c common.h xmpp.h

fuse.o: fuse.c common.h parser.h

xmpp.o: xmpp.c xmpp.h

parser.o: parser.c parser.h

clean:
	rm *.o
	rm hatexmpp
