CFLAGS = `pkg-config --cflags gtk+-2.0`
LDFLAGS = `pkg-config --libs gtk+-2.0`

all: gtkface

gtkface: gtkface.o gmain.o
gmain.o: gmain.c gtkface.h
gtkface.o: gtkface.c

clean:
	rm *.o
	rm gtkface
