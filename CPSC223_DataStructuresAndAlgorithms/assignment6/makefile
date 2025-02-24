CC=gcc
CFLAGS=-Wall -pedantic -std=c99 -g3

Paths: paths.o ldigraph.o queue.o
	${CC} -o $@ ${CFLAGS} $^

ldigraph.o: ldigraph.h
paths.o: ldigraph.h
queue.o: queue.h
