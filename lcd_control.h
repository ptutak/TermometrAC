#ifndef _LCD_CONTROL_H_
#define _LCD_CONTROL_H_

#include "my_types.h"
#include "my_twi.h"


typedef enum {
	LCD_NULL=0,
	LCD_INIT,
}LCDStatus;

typedef enum{
	LCD_CLEAR_DISPLAY=1,
	LCD_RETURN_HOME=2,

	LCD_SET_DECREMENT=4,
	LCD_SET_DECREMENT_SHIFT=5,

	LCD_SET_INCREMENT=6,
	LCD_SET_INCREMENT_SHIFT=7,

	LCD_SET_DISPLAY_OFF=8,
	LCD_SET_DISPLAY_ON_CURSOR_OFF_BLINKING_OFF=12,
	LCD_SET_DISPLAY_ON_CURSOR_OFF_BLINKING_ON=13,
	LCD_SET_DISPLAY_ON_CURSOR_ON_BLINKING_OFF=14,
	LCD_SET_DISPLAY_ON_CURSOR_ON_BLINKING_ON=15,

	LCD_SHIFT_CURSOR_LEFT=16,
	LCD_SHIFT_CURSOR_RIGHT=20,
	LCD_SHIFT_DISPLAY_LEFT=24,
	LCD_SHIFT_DISPLAY_RIGHT=28,

	LCD_F_SET_4_BIT_1_LINE_8_FONT=32,
	LCD_F_SET_4_BIT_1_LINE_10_FONT=36,
	LCD_F_SET_4_BIT_2_LINE_8_FONT=40,
	LCD_F_SET_8_BIT_1_LINE_8_FONT=48,
	LCD_F_SET_8_BIT_1_LINE_10_FONT=52,
	LCD_F_SET_8_BIT_2_LINE_8_FONT=56,

	LCD_SET_CGRAM_ADDR=64,
	LCD_SET_DDRAM_ADDR=128

}LCDCommand;

typedef enum{
	LCD_WRITE_CG_OR_DDR=1,
	LCD_READ_BS_FLAG=2,
	LCD_READ_CG_OR_DDR=3,

	LCD_WRITE_CG_OR_DDR_E=5,
	LCD_READ_CG_OR_DDR_E=7,

	LCD_WRITE_CG_OR_DDR_BT=9,
	LCD_READ_CG_OR_DDR_BT=11,

	LCD_WRITE_CG_OR_DDR_E_BT=13,
	LCD_READ_CG_OR_DDR_E_BT=15,
}RSRWEBT;

typedef union {
	RSRWEBT command;
	struct {
		bool rs :1;
		bool rw :1;
		bool e  :1;
		bool bt :1;
		bool    :0;
	};
}LCDRSRWEBT;



typedef struct{
	uint8_t address;
	LCDStatus status;
	const __flash uint8_t* configInit;
}LCD;

const __flash uint8_t* LCD_CONFIG_INIT_2X16={0x20,
											0x20,0xC0,		//number of lines
											0x00,0x80,		//display OFF
											0x00,0x10,		//display CLEAR
											0x00,0x70 };	//entry mode set



uint16_t splitDataPCF8574_DataHigh(LCDRSRWEBT status, uint8_t data){
	uint8_t high=e<<2; //E enabled;
	uint8_t low=e<<2; //E enabled;
	high|=rs;
	low|=rs;
	high|=rw<<1;
	low|=rw<<1;
	high|=data&(0xF0);
	low|=data<<4;
	return (uint16_t)((high<<8)|low);
}


void initLCD(LCD* lcd, const __memx uint8_t* configInit, uint8_t address){
	lcd->address=address;
	lcd->config
	twiInit(100000,true);

}

#endif _LCD_CONTROL_H_
