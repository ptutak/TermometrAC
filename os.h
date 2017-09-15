#ifndef _OS_H_
#define _OS_H_
#include "my_types.h"
#include "my_queue.h"

CommQueue* osQueue(void);


void addOsFunc(void (*runFunc)(OsPackage* package),const __memx void* data, uint8_t size, bool dynamic);

void remOsFunc(uint8_t index);

void manageOsQueue(void);


#endif
