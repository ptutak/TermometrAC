#include "termometr.h"


int getTemperature(int pinPlus, int pinMinus, uint16_t referenceVoltage){
	uint16_t tempPlus,tempMinus;

	adcInit(pinPlus,AREF);
	tempPlus=(int)adcGetStatisticalValue(128);
	adcInit(pinMinus,AREF);
	tempMinus=(int)adcGetStatisticalValue(128);
    return (int)(((long)(tempPlus-tempMinus)*referenceVoltage)/1024);
}

