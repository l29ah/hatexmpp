CFLAGS = -Wall `pkg-config fuse loudmouth-1.0 --cflags --libs`
LDFLAGS = `pkg-config fuse loudmouth-1.0 --libs`

all: hatexmpp 

hatexmpp: hatexmpp.o fuse.o

clean:
	rm *.o
	rm hatexmpp
