#ifndef _LCD_CONTROL_H_
#define _LCD_CONTROL_H_

#include "my_types.h"
#include "my_twi.h"


typedef enum {
	LCD_NULL=0,
	LCD_INIT,
}LCDStatus;

typedef enum{
	LCD_NULL=0x00,

	LCD_CLEAR_DISPLAY=0x01,
	LCD_RETURN_HOME=0x02,

	LCD_SET_DECREMENT=0x04,
	LCD_SET_DECREMENT_SHIFT=0x05,

	LCD_SET_INCREMENT=0x06,
	LCD_SET_INCREMENT_SHIFT=0x07,

	LCD_SET_DISPLAY_OFF=0x08,
	LCD_SET_DISPLAY_ON_CURSOR_OFF_BLINKING_OFF=0x0C,
	LCD_SET_DISPLAY_ON_CURSOR_OFF_BLINKING_ON=0x0D,
	LCD_SET_DISPLAY_ON_CURSOR_ON_BLINKING_OFF=0x0E,
	LCD_SET_DISPLAY_ON_CURSOR_ON_BLINKING_ON=0x0F,

	LCD_SHIFT_CURSOR_LEFT=0x10,
	LCD_SHIFT_CURSOR_RIGHT=0x14,
	LCD_SHIFT_DISPLAY_LEFT=0x18,
	LCD_SHIFT_DISPLAY_RIGHT=0x1C,

	LCD_F_SET_4_BIT_1_LINE_8_FONT=0x20,
	LCD_F_SET_4_BIT_1_LINE_10_FONT=0x24,
	LCD_F_SET_4_BIT_2_LINE_8_FONT=0x28,
	LCD_F_SET_8_BIT_1_LINE_8_FONT=0x30,
	LCD_F_SET_8_BIT_1_LINE_10_FONT=0x34,
	LCD_F_SET_8_BIT_2_LINE_8_FONT=0x38,

	LCD_SET_CGRAM_ADDR=0x40,
	LCD_SET_DDRAM_ADDR=0x80,

	LCD_FULL=0xFF

}LCDCommand;


//byte structure: 0b0,0,0,0,bt,e,rw,rs
typedef enum{
	LCD_COMMAND=0x00,

	LCD_WRITE_CG_OR_DDR=0x01,

	LCD_READ_BS_FLAG_AND_ADDR=0x02,

	LCD_READ_CG_OR_DDR=0x03,

	LCD_WRITE_CG_OR_DDR_E=0x05,

	LCD_READ_BS_FLAG_AND_ADDR_E=0x06,

	LCD_READ_CG_OR_DDR_E=0x07,

	LCD_COMMAND_BT=0x08,

	LCD_WRITE_CG_OR_DDR_BT=0x09,
	LCD_READ_CG_OR_DDR_BT=0x0B,

	LCD_WRITE_CG_OR_DDR_E_BT=0x0D,
	LCD_READ_CG_OR_DDR_E_BT=0x0F,
}LCDCommandType;

typedef struct{
	LCDCommandType commandType;
	LCDCommand command;
}LCDCommadS;

typedef struct{
	uint8_t low;
	uint8_t high;
}uint16_t_split;

typedef struct{
	uint8_t address;
	LCDStatus status;
	const __flash LCDCommandS* configInit;
	uint8_t configInitArraySize;
}LCD;

const __flash LCDCommandS LCD_CONFIG_INIT_2X16S[]={
		{LCD_COMMAND,LCD_F_SET_4_BIT_2_LINE_8_FONT},
		{LCD_COMMAND,LCD_SET_DISPLAY_OFF},
		{LCD_COMMAND,LCD_CLEAR_DISPLAY},
		{LCD_COMMAND,LCD_SET_INCREMENT},
		{LCD_COMMAND,LCD_SET_DISPLAY_ON_CURSOR_ON_BLINKING_ON}
};



uint16_t_split splitDataPCF8574_DataHigh(LCDCommandType commandType, uint8_t data){
	uint8_t high=(uint8_t)commandType;
	uint8_t low=(uint8_t)commandType;
	high|=data&(0xF0);
	low|=data<<4;
	return (uint16_t_split){low,high};
}


void initLCD(LCD* lcd, uint16_t_split (*splitFunction)(LCDCommandType type,uint8_t data)){
	if (lcd->configInitArraySize==0)
		return;

	uint16_t_split waitForBSFlag=(*splitFunction)(LCD_READ_BS_FLAG_AND_ADDR,LCD_NULL);
	uint16_t_split firstData=(*splitFunction)(lcd->configInit[0].commandType,lcd->configInit[0].command);

	uint8_t initDataSize=lcd->configInitArraySize*sizeof(LCDCommandS)*2+1;
	uint8_t* initData=malloc(initDataSize);

	initData[0]=firstData.high;

	for (int i=0;i<lcd->configInitArraySize;i++){
		uint16_t_split tmpData=(*splitFunction)(lcd->configInit[i].commandType,lcd->configInit[i].command);
		initData[1+i*sizeof(LCDCommandS)*2]=tmpData.high;
		initData[1+i*sizeof(LCDCommandS)*2+1]=tmpData.low;
		initData[1+i*sizeof(LCDCommandS)*2+2]=waitForBSFlag.high;
		initData[1+i*sizeof(LCDCommandS)*2+3]=waitForBSFlag.low;
	}

	twiInit(100000,true);

	twiSendData(initData,initDataSize,true);

}

#endif _LCD_CONTROL_H_
