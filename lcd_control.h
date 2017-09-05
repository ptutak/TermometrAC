#ifndef _LCD_CONTROL_H_
#define _LCD_CONTROL_H_

#include "my_types.h"
#include "my_twi.h"


typedef enum {
	LCD_NULL=0,
	LCD_INIT,
}LCDStatus;

typedef struct{
	uint8_t address;
	LCDStatus status;
	const __flash uint8_t* configInit;
}LCD;

const __flash uint8_t* LCD_INIT_2X16={0xFF,0x20,0x40};


void initLCD(LCD* lcd, const __memx uint8_t* configInit, uint8_t address){
	lcd->address=address;
	lcd->config
	twiInit(100000,true);

	twiSendData()
}

#endif _LCD_CONTROL_H_
