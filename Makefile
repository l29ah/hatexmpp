ifdef FEATURES
	ifneq ($(findstring debug, $(FEATURES)),)
		CFLAGS+=-ggdb -DDEBUG -O0 -Wall
	endif
	ifneq ($(findstring proxy, $(FEATURES)),)
		CFLAGS+=-DPROXY
	endif
endif

CFLAGS+=$(shell pkg-config loudmouth-1.0 --cflags) 
LDFLAGS+=$(shell pkg-config loudmouth-1.0 --libs) -lixp

all: hatexmpp

hatexmpp: version.o hatexmpp.o filesystem.o xmpp.o 

hatexmpp.o: hatexmpp.c common.h

#fuse.o: fuse.c common.h
filesystem.o: filesystem.c common.h

xmpp.o: xmpp.c common.h

version.o: version.c

version.c: hatexmpp.c common.h xmpp.c
	echo "char HateXMPP_ver[] = "\"0.3-`git log|head -n1|sed 's/commit //'`\""; char * getversion(void) { return HateXMPP_ver; }" > version.c

.PHONY:	clean

clean:
	rm *.o
	rm version.c
	rm hatexmpp

install: hatexmpp
	install hatexmpp /usr/bin

