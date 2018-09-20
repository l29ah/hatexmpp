CFLAGS+=-std=gnu99
ifdef DEBUG
	CFLAGS+=-ggdb -DDEBUG -O0 -Wall -pedantic
else
	CFLAGS+=-DNDEBUG
endif

EXE=hatexmpp
CFLAGS+=$(shell pkg-config fuse loudmouth-1.0 --cflags)
LDLIBS=$(shell pkg-config fuse loudmouth-1.0 --libs)

all: $(EXE)

$(EXE): version.o hatexmpp.o fuse.o xmpp.o

hatexmpp.o: hatexmpp.c common.h
fuse.o: fuse.c common.h
xmpp.o: xmpp.c common.h

version.c:
	echo "char HateXMPP_ver[] = "\"0.2-`git log --no-show-signature -n1 --pretty=format:%H`\""; char * getversion(void) { return HateXMPP_ver; }" > version.c

.PHONY:	clean

clean:
	rm -rf *.o version.c $(EXE)

install: $(EXE)
	install $(EXE) $(DESTDIR)/usr/bin

