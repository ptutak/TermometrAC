#ifndef _MY_QUEUE_H_
#define _MY_QUEUE_H_ 1

#include "my_types.h"

void queue(CommQueue* queue, void* package, char type);

Package dequeue(CommQueue* queue, char type);

void insert(CommQueue* queue, void* package, char type, uint16_t index);

#endif
