#ifndef _OS_H_
#define _OS_H_
#include "my_types.h"
#include "my_queue.h"

CommQueue* osDynamicQueue(void);
CommQueue* osStaticQueue(void);
CommQueue* osInitQueue(void);

PriorityQueue* osDynamicPriorQueue(void);
PriorityQueue* osStaticPriorQueue(void);

void addOsFunc(CommQueue* osQueue,void (*runFunc)(OsPackage* package),const __memx void* data, uint8_t size, bool dynamic);
void addOsPriorFunc(PriorityQueue* osQueue,void (*runFunc)(OsPackage* package),const __memx void* data, uint8_t size, bool dynamic, uint8_t priority);

void remOsFunc(CommQueue* osQueue,uint8_t index);
void remOsPriorFunc(PriorityQueue* osQueue,uint8_t priority);

void manageOsDynamicQueue(CommQueue* osQueue);
void manageOsDynamicPriorQueue(PriorityQueue* osQueue);
void manageOsQueue(CommQueue* osQueue);
void manageOsPriorQueue(PriorityQueue* osQueue);

#endif
