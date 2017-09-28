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


#include "termometr.h"


int getTemperature(int pinPlus, int pinMinus, uint16_t referenceVoltage){
	uint16_t tempPlus,tempMinus;

	adcInit(pinPlus,AREF);
	tempPlus=(int)adcGetStatisticalValue(20);
	adcInit(pinMinus,AREF);
	tempMinus=(int)adcGetStatisticalValue(20);
    return (int)(((long)(tempPlus-tempMinus)*referenceVoltage)/1024);
}

uint16_t getReferenceVoltage(int pinPlus){
	return 2000;
}
