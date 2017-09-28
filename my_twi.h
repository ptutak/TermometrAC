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


#ifndef _MY_TWI_H_
#define _MY_TWI_H_

#include "my_types.h"
#include "my_queue.h"



extern const uint32_t TWI_FREQ;
extern const uint8_t TWI_STD_TTL;

CommQueue* twiMasterQueue(void);

bool twiReady(void);

void twiInit(uint32_t freq,bool twea);
void twiSendMasterData(const __memx uint8_t* data, uint8_t size,uint8_t address, void (*callFunc)(TwiPackage* self));
void twiReadMasterData(uint8_t* data, uint8_t size, uint8_t address, void (*callFunc)(TwiPackage* self));

void twiInterrupt(OsPackage* notUsed);
void twiManageQueue(CommQueue* commQueue);
void twiManageMasterQueue(OsPackage* notUsed);

void twiOff(void);

void freeTwiPackageData(TwiPackage* package);


#endif
