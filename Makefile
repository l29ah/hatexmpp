CFLAGS = -Wall `pkg-config fuse loudmouth-1.0 --cflags` -ggdb -DDEBUG -O0
LDFLAGS = `pkg-config fuse loudmouth-1.0 --libs`

all: hatexmpp 

hatexmpp: hatexmpp.o parser.o fuse.o xmpp.o

hatexmpp.o: hatexmpp.c common.h xmpp.h

fuse.o: fuse.c common.h parser.h

xmpp.o: xmpp.c xmpp.h common.h

parser.o: parser.c parser.h

clean:
	rm *.o
	rm hatexmpp
