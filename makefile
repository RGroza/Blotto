CC = gcc
CFLAGS = -Wall -std=c99 -pedantic -g3

all: GMapUnit Blotto

Blotto: blotto.o gmap.o
	$(CC) -o $@ -g $^ $(CFLAGS) -lm

GMapUnit: gmap.o gmap_test_functions.o gmap_unit.o
	$(CC) -o $@ -g $^ $(CFLAGS) -lm

blotto.o: gmap.h
gmap.o: gmap.h
gmap_test_functions.o: gmap_test_functions.h
gmap_unit.o: gmap.h gmap_test_functions.h string_key.h