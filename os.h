#ifndef _OS_H_
#define _OS_H_
#include "my_types.h"
#include "my_queue.h"

CommQueue* osDynamicQueue(void);
CommQueue* osStaticQueue(void);
CommQueue* osInitQueue(void);

void addOsFunc(CommQueue* queue,void (*runFunc)(OsPackage* package),const __memx void* data, uint8_t size, bool dynamic);

void remOsFunc(CommQueue* queue,uint8_t index);

void manageOsQueue(CommQueue* queue, bool dynamic);


#endif
