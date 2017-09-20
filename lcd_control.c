#include "lcd_control.h"




const __flash LCDCommandS LCD_CONFIG_INIT_2X16S[4]={
		{LCD_COMMAND,LCD_F_SET_4_BIT_2_LINE_8_FONT},
		{LCD_COMMAND,LCD_SET_DISPLAY_ON_CURSOR_ON_BLINKING_ON},
		{LCD_COMMAND,LCD_CLEAR_DISPLAY},
		{LCD_COMMAND,LCD_SET_INCREMENT},
};

const __flash uint8_t LCD_CONFIG_INIT_2X16S_SIZE=4;

uint16_t_split splitDataPCF8574_DataHigh(LCDCommandType commandType, uint8_t data){
	uint8_t high=(uint8_t)commandType;
	uint8_t low=(uint8_t)commandType;
	high|=data&(0xF0);
	low|=data<<4;
	return (uint16_t_split){low,high};
}




const __flash uint8_t waitForBSFlag[2]={0b11110010,0b11110010};

Package* waitForBSFlagPackage(uint8_t address){
	static Package package;
	package=(Package){.tPackage={waitForBSFlag,2,address,'R',TWI_STD_TTL,0,TWI_NULL,NULL}};
	return &package;
}

void waitForBSFlagFunc(TwiPackage* package){
	if (*(package->data)&0b10000000){
		insert(twiMasterQueue(),&(Package){.tPackage=*package},0);
		insert(twiMasterQueue(),waitForBSFlagPackage(package->address),0);
		return;
	}
	free((uint8_t*)package->data);
}




void lcdInit(LCD* lcd, uint16_t_split (*splitFunction)(LCDCommandType type,uint8_t data)){
	if (lcd->configInitArraySize==0)
		return;

	uint16_t_split configData=(*splitFunction)(LCD_COMMAND,LCD_F_SET_8_BIT_2_LINE_8_FONT);

	uint8_t* data=malloc(2);
	data[0]=configData.high;
	twiSendMasterDataNoInterrupt(data,1,lcd->address,NULL);
	runTwiInterruptFunc(NULL);
	_delay_ms(20);
	twiSendMasterDataNoInterrupt(data,1,lcd->address,NULL);
	runTwiInterruptFunc(NULL);
	_delay_ms(10);
	twiSendMasterDataNoInterrupt(data,1,lcd->address,freePackageData);
	runTwiInterruptFunc(NULL);
	_delay_ms(10);
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		for (int i=0;i<lcd->configInitArraySize;i++){
			uint8_t* data=malloc(2);
			uint8_t* receivedData=malloc(2);
			configData=(*splitFunction)((lcd->configInitArray[i]).commandType,(lcd->configInitArray[i]).command);
			data[0]=configData.high;
			data[1]=configData.low;
			twiSendMasterDataNoInterrupt(data,2,lcd->address,freePackageData);
			twiSendMasterDataNoInterrupt(waitForBSFlag,2,lcd->address,NULL);
			twiReadMasterDataNoInterrupt(receivedData,2,lcd->address,waitForBSFlagFunc);
		}
	}
	runTwiInterruptFunc(NULL);
}
