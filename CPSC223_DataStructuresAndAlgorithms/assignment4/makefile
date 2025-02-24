CC = gcc
CFLAGS = -std=c99 -Wall -g

Blotto: blotto.o gmap_test_functions.o gmap.o string_key.o string_helper.o
	${CC} -o $@ $^ ${CFLAGS} -lm

GmapUnit: gmap_unit.o gmap_test_functions.o gmap.o string_key.o string_helper.o
	${CC} -o $@ $^ ${CFLAGS} -lm

Test: test.o gmap_test_functions.o gmap.o string_key.o string_helper.o
	${CC} -o $@ $^ ${CFLAGS} -lm

blotto.o: blotto.c
	${CC} -c $^ ${CFLAGS}

gmap_unit.o: gmap_unit.c 
	${CC} -c $^ ${CFLAGS}

gmap_test_functions.o: gmap_test_functions.c
	${CC} -c $^ ${CFLAGS}

gmap.o: gmap.c
	${CC} -c $^ ${CFLAGS}

string_helper.o: string_helper.c
	${CC} -c $^ ${CFLAGS}

string_key.o: string_key.c
	${CC} -c $^ ${CFLAGS}

test.o: test.c
	${CC} -c $^ ${CFLAGS}