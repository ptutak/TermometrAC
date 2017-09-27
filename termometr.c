#include "termometr.h"


int getTemperature(int pinPlus, int pinMinus, uint16_t referenceVoltage){
	int tempPlus,tempMinus;

	adcInit(pinPlus,AREF);
	tempPlus=(int)adcGetStatisticalValue(128);
	tempPlus=(tempPlus*referenceVoltage)/1024;

	adcInit(pinMinus,AREF);
	tempMinus=(int)adcGetStatisticalValue(128);
	tempMinus=(tempMinus*referenceVoltage)/1024;

	return tempPlus-tempMinus;
}

