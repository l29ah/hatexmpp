#!/bin/bash
# mk_gtkface.sh

gcc -c gtkface.c -Os -g `pkg-config --cflags --libs gtk+-2.0` -o libgtkface.a
gcc gmain.c -L. -lgtkface -Os -g `pkg-config --cflags --libs gtk+-2.0` -o gmain 

