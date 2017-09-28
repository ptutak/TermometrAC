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


#ifndef _AC_ADC_H_
#define _AC_ADC_H_


#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "my_usart.h"


typedef enum{
	AREF,
	INTERNAL_AVCC,
	INTERNAL_VREF_1_1V
}ReferenceType;

void adcInit(int input,ReferenceType type);
void adcOff(void);

uint16_t adcGetValue(void);
uint16_t adcGetStatisticalValue(uint8_t sampleNumber);

#endif
