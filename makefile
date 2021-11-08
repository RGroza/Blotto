CC = gcc
CFLAGS = -std=c99 -pedantic -g3

all: GmapUnit Blotto

GmapUnit: gmap_unit.o gmap.o gmap_test_functions.o string_key.o
	$(CC) $(CFLAGS) -o $@ -g $^ -lm

Blotto: blotto.o gmap.o
	$(CC) -o $@ -g $^ $(CFLAGS) -lm

blotto.o: blotto.c
gmap.o: gmap.c gmap.h
gmap_test_functions.o: gmap_test_functions.c gmap_test_functions.h
string_key.o: string_key.c string_key.h
gmap_unit.o: gmap_unit.c gmap_test_functions.h string_key.h