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


void waitForBSFlagFunc(TwiPackage* package);

const uint8_t waitForBSFlag[]={0b11110010,0b11110010};

TwiPackage* waitForBSFlagPackage(uint8_t address){
	static TwiPackage package={waitForBSFlag,2,0,'R',TWI_NULL,waitForBSFlagFunc};
	package.address=address;
	return &package;
}

void waitForBSFlagFunc(TwiPackage* package){
	if (*(package->data)&0b00001000)
		insert(twiMasterQueue(),(void*)waitForBSFlagPackage(package->address),'t',0);
}



void initLCD(LCD* lcd, uint16_t_split (*splitFunction)(LCDCommandType type,uint8_t data)){
	if (lcd->configInitArraySize==0)
		return;

	uint16_t_split configData=(*splitFunction)(lcd->configInit[0].commandType,lcd->configInit[0].command);

	uint8_t data[2];
	data[0]=configData.high;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		twiSendMasterDataNoInterrupt(data,1,lcd->address,NULL);
		twiSendMasterDataNoInterrupt(waitForBSFlag,2,lcd->address,waitForBSFlagFunc);
		for (int i=0;i<lcd->configInitArraySize;i++){
			configData=(*splitFunction)(lcd->configInit[i].commandType,lcd->configInit[i].command);
			data[0]=configData.high;
			data[1]=configData.low;
			twiSendMasterDataNoInterrupt(data,2,lcd->address,NULL);
			twiSendMasterDataNoInterrupt(waitForBSFlag,2,lcd->address,waitForBSFlagFunc);
		}
	}
	twiManageOrders();
}

#endif _LCD_CONTROL_H_
