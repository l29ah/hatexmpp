CFLAGS+=-std=gnu99
ifdef DEBUG
	CFLAGS+=-ggdb3 -DDEBUG -Og -Wall -pedantic
else
	CFLAGS+=-DNDEBUG
endif

EXE=hatexmpp
CFLAGS+=$(shell pkg-config fuse loudmouth-1.0 --cflags)
LDLIBS:=$(shell pkg-config fuse loudmouth-1.0 --libs)

all: main-build

main-build: astyle
	$(MAKE) --no-print-directory $(EXE)

SRCS := $(wildcard *.c) version.c
OBJS := $(SRCS:%.c=%.o)

$(EXE): $(OBJS)

.PHONY:	clean astyle

CPPFLAGS += -MMD
-include $(SRCS:.c=.d)

version.c:
	echo "char HateXMPP_ver[] = "\"0.2-`git log --no-show-signature -n1 --pretty=format:%H`\""; char * getversion(void) { return HateXMPP_ver; }" > version.c

.PHONY:	clean astyle

clean:
	rm -rf *.o *.d version.c $(EXE)

install: $(EXE)
	install $(EXE) $(DESTDIR)/usr/bin

astyle:
	astyle --style=linux --indent=tab --unpad-paren --pad-header --pad-oper *.c *.h
