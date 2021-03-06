/*
Copyright 2017 Piotr Tutak

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/


#ifndef _OS_H_
#define _OS_H_

#include "types.h"
#include "queue.h"

CommQueue* osInitQueue(void);

CommQueue* osDynamicQueue(void);
CommQueue* osStaticQueue(void);
PriorityQueue* osDynamicPriorQueue(void);
PriorityQueue* osStaticPriorQueue(void);

void addOsFunc(CommQueue* osQueue,void (*runFunc)(OsPackage* package),const __memx void* data, uint8_t size, bool dynamic);
void addOsPriorFunc(PriorityQueue* osQueue,void (*runFunc)(OsPackage* package),const __memx void* data, uint8_t size, bool dynamic, uint16_t priority);

void remOsFunc(CommQueue* osQueue,uint8_t index);
void remOsPriorFunc(PriorityQueue* osQueue,uint16_t priority);

void manageOsDynamicQueue(CommQueue* osQueue);
void manageOsDynamicPriorQueue(PriorityQueue* osQueue);
void manageOsQueue(CommQueue* osQueue);
void manageOsPriorQueue(PriorityQueue* osQueue);

#endif
