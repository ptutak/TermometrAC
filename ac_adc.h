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
