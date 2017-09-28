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

#include "ac_adc.h"



void adcInit(int input,ReferenceType type){
	switch (input){
	case PC0:
		ADMUX=0;
		DIDR0=~(1<<PC0);
		break;
	case PC1:
		ADMUX=1<<MUX0;
		DIDR0=~(1<<PC1);
		break;
	case PC2:
		ADMUX=1<<MUX1;
		DIDR0=~(1<<PC2);
		break;
	case PC3:
		ADMUX=1<<MUX1|1<<MUX0;
		DIDR0=~(1<<PC3);
		break;
	}
	switch(type){
	case AREF:
		break;
	case INTERNAL_AVCC:
		ADMUX|=1<<REFS0;
		break;
	case INTERNAL_VREF_1_1V:
		ADMUX|=1<<REFS1 | 1<<REFS0;
		break;
	}
	ADCSRA=1<<ADEN | 1<<ADIE | 1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0;
}

ISR(ADC_vect){};

uint16_t adcGetValue(void) {
	if (!(ADCSRA&(1<<ADEN)))
		return 0;
	set_sleep_mode(SLEEP_MODE_ADC);
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        sleep_enable();
    };
    sleep_cpu();
    sleep_disable();
	return ADC;
}

void adcOff(void){
	ADMUX=0x00;
	ADCSRA=0x00;
}

uint16_t adcGetStatisticalValue(uint8_t sampleNumber){
	if (!(ADCSRA&(1<<ADEN)))
		return 0;
	if (sampleNumber==0)
		return 0;
	uint32_t sum=0;
	for(uint8_t i=0;i<sampleNumber;++i){
    	sum+=adcGetValue();
    }
	return sum/sampleNumber;
}
