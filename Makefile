CFLAGS = -Wall `pkg-config loudmouth-1.0 --cflags --libs`
LDFLAGS = `pkg-config loudmouth-1.0 --libs`

all: hatexmpp 

hatexmpp: hatexmpp.o fs.o

clean:
	rm *.o
	rm hatexmpp
