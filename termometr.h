#ifndef _TERMOMETR_H_
#define _TERMOMETR_H_

#include "ac_adc.h"

uint16_t getTemperature(uint16_t referenceValue){
	uint16_t temp=adcGetStatisticalValue(128);
	temp=temp*referenceValue/1024;
}


#endif
