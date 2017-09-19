#ifndef _MY_QUEUE_H_
#define _MY_QUEUE_H_ 1

#include "my_types.h"

void queue(CommQueue* queue, Package* package);

Package dequeue(CommQueue* queue);

void queuePrior(PriorityQueue* queue, Package* package, uint8_t priority);

Package dequeuePrior(PriorityQueue* queue);

Package removePrior(PriorityQueue* queue, uint8_t priority);

void insert(CommQueue* queue, Package* package, uint8_t index);

Package remove(CommQueue* queue, uint8_t index);


#endif
