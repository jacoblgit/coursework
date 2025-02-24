CC = gcc
CFLAGS = -std=c99 -Wall -g

Unit: kdtree_unit.o kdtree.o location.o list.o
	${CC} -o $@ $^ ${CFLAGS} -lm

Test: test.o kdtree.o location.o list.o
	${CC} -o $@ $^ ${CFLAGS} -lm

kdtree_unit.o: kdtree_unit.c
	${CC} -c $^ ${CFLAGS}

kdtree.o: kdtree.c
	${CC} -c $^ ${CFLAGS}

list.o: list.c
	${CC} -c $^ ${CFLAGS}

location.o: location.c
	${CC} -c $^ ${CFLAGS}

test.o: test.c
	${CC} -c $^ ${CFLAGS}

clean:
	rm -f Test *.o