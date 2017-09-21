#include "lcd_control.h"




LCDCommand LCD_CONFIG_INIT_2X16S[]={
		LCD_F_SET_4_BIT_2_LINE_8_FONT,
		LCD_SET_DISPLAY,
		LCD_CLEAR_DISPLAY,
		LCD_SET_INCREMENT,
		LCD_SET_DISPLAY|DISPLAY_ON|CURSOR_ON,
		LCD_RETURN_HOME
};

const uint8_t LCD_CONFIG_INIT_2X16S_SIZE=6;


uint16_t_split splitDataPCF8574_DataHigh(LCDInstructionType instructionType, uint8_t data){
	uint8_t high=(uint8_t)instructionType;
	uint8_t low=(uint8_t)instructionType;
	high|=data&(0xF0);
	low|=data<<4;
	return (uint16_t_split){low,high};
}

uint8_t* waitForBSFlagData;

Package* waitForBSFlagPackage(uint8_t address){
	static Package package;
	package=(Package){.tPackage={waitForBSFlagData,2,address,'R',TWI_STD_TTL,0,TWI_NULL,NULL}};
	return &package;
}

void waitForBSFlagFunc(TwiPackage* package){
	if (*(package->data)&0b10000000){
		insert(twiMasterQueue(),&(Package){.tPackage=*package},0);
		insert(twiMasterQueue(),waitForBSFlagPackage(package->address),0);
		usartSafeTransmit('b');
		usartSafeTransmit('\n');
		return;
	}
	free((uint8_t*)package->data);
}


void lcdInit(LCD* lcd, uint16_t_split (*splitFunction)(LCDInstructionType type,uint8_t data)){
	if (lcd->configInitArraySize==0)
		return;

	uint16_t_split configData;
	uint16_t_split configDataE;
	uint8_t* data=malloc(6);
	waitForBSFlagData=malloc(6);
	configData=(*splitFunction)(LCD_READ_BS_FLAG_AND_ADDR | BACKLIGHT,LCD_FULL);
	configDataE=(*splitFunction)(LCD_READ_BS_FLAG_AND_ADDR | BACKLIGHT | COMMAND_ENABLE,LCD_FULL);
	waitForBSFlagData[0]=configDataE.high;
	waitForBSFlagData[1]=configDataE.low;
	waitForBSFlagData[2]=configData.high;
	waitForBSFlagData[3]=configData.low;

	configData=(*splitFunction)(LCD_COMMAND | BACKLIGHT,LCD_F_SET_8_BIT_2_LINE_8_FONT);
	configDataE=(*splitFunction)(LCD_COMMAND| BACKLIGHT |COMMAND_ENABLE,LCD_F_SET_8_BIT_2_LINE_8_FONT);
	data[0]=configDataE.high;
	data[1]=configData.high;

	twiSendMasterData(data,2,lcd->address,NULL);
	_delay_ms(5);
	twiSendMasterData(data,2,lcd->address,NULL);
	_delay_ms(5);

	twiSendMasterData(data,2,lcd->address,freePackageData);
	_delay_ms(5);

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		for (int i=0;i<lcd->configInitArraySize;i++){
			data=malloc(6);
			configData=(*splitFunction)(LCD_COMMAND|BACKLIGHT,lcd->configInitArray[i]);
			configDataE=(*splitFunction)(LCD_COMMAND|BACKLIGHT|COMMAND_ENABLE,lcd->configInitArray[i]);
			data[0]=configDataE.high;
			data[1]=configDataE.low;
			data[2]=configData.high;
			data[3]=configData.low;
			twiSendMasterDataNoInterrupt(data,4,lcd->address,NULL);
			twiSendMasterDataNoInterrupt(waitForBSFlagData,4,lcd->address,NULL);
			data=malloc(2);
			twiReadMasterDataNoInterrupt(data,2,lcd->address,NULL);
		}
	}
	runTwiInterruptFunc(NULL);
}
