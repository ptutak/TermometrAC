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


#ifndef _MY_QUEUE_H_
#define _MY_QUEUE_H_ 1

#include "my_types.h"

void queue(CommQueue* queue, Package* package);

Package dequeue(CommQueue* queue);

void queuePrior(PriorityQueue* queue, Package* package, uint16_t priority);

Package dequeuePrior(PriorityQueue* queue);

Package removePrior(PriorityQueue* queue, uint16_t priority);

void insert(CommQueue* queue, Package* package, uint8_t index);

Package remove(CommQueue* queue, uint8_t index);


#endif
