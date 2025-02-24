#ifndef _QUEUE_H
#define _QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct queue queue;

/* create a new empty queue */
struct queue *
queueCreate(void);

/* add a new value to back of queue */
void
enq(struct queue *q, int value);

int
queueEmpty(const struct queue *q);

/* remove and return value from front of queue */
int
deq(struct queue *q);

/* print contents of queue on a single line, head first */
void
queuePrint(struct queue *q);

/* free a queue and all of its elements */
void
queueDestroy(struct queue *q);

#endif
