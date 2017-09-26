#ifndef _ADC_H_
#define _ADC_H_


#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/atomic.h>


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
