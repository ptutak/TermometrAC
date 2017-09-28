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


#ifndef _MY_USART_H_
#define _MY_USART_H_ 1

#include "my_types.h"
#include "my_queue.h"


CommQueue* usartToSendQueue(void);
CommQueue* usartReceivedQueue(void);


void usartSafeTransmit(uint8_t data);


void usartSendText(const __memx char* text, uint8_t size, bool dynamic);
void usartSendData(const __memx uint8_t* data, uint8_t size, bool dynamic);


const char* usartGetText();
UsartPackage usartGetData(void);


void usartInit(uint16_t baud);

void usartSendInterrupt(OsPackage* notUsed);
void usartManageToSendQueue(OsPackage* notUsed);


#endif
