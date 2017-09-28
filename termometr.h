#ifndef _TERMOMETR_H_
#define _TERMOMETR_H_

#include "ac_adc.h"

int getTemperature(int pinPlus, int pinMinus, uint16_t referenceVoltage);

uint16_t getReferenceVoltage(int pinPlus);
#endif
