#include "lcd_control.h"




LCDCommand LCD_CONFIG_INIT_2X16S[]={
		LCD_F_SET_4_BIT_1_LINE_8_FONT,
		LCD_SET_DISPLAY|DISPLAY_ON|BLINKING_ON,
		LCD_CLEAR_DISPLAY,
		LCD_SET_INCREMENT,
};

uint8_t LCD_CONFIG_INIT_2X16S_SIZE=sizeof(LCD_CONFIG_INIT_2X16S)/sizeof(LCDCommand);


uint16_t splitDataPCF8574_DataHigh(uint8_t instructionType, uint8_t data){
	return ((uint16_t)(instructionType|(data & 0xF0)))|(((uint16_t)(instructionType|(data<<4)))<<8);
}

uint8_t receivedDataPCF8574_DataHigh(uint8_t high, uint8_t low){
	return (high & 0xF0)|((low & 0xF0)>>4);
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
}

void lcdNextFunc(TwiPackage* package){
	addOsFunc(osDynamicQueue(),twiInterrupt,NULL,0,false);
	free((uint8_t*)package->data);
}

static inline uint32_t packageEnableSplit(uint8_t instruction,uint8_t data, uint16_t (*splitFunction)(uint8_t instruction, uint8_t data)){
	return (((uint32_t)(*splitFunction)(instruction,data))<<16) | ((*splitFunction)(instruction|ENABLE,data));
}

void lcdInit(LCD* lcd, uint16_t (*splitFunction)(uint8_t instruction,uint8_t data)){
	if (lcd->configInitArraySize==0)
		return;

	uint8_t* waitForBSFlagData=malloc(4);
	*((uint32_t*)waitForBSFlagData)=packageEnableSplit(LCD_READ_BS_FLAG_AND_ADDR | BACKLIGHT,LCD_FULL,splitFunction);


	uint8_t* data=malloc(2);
	data[0]=(uint8_t)(*splitFunction)(LCD_COMMAND | BACKLIGHT | ENABLE, LCD_F_SET_8_BIT_2_LINE_8_FONT);
	data[1]=(uint8_t)(*splitFunction)(LCD_COMMAND | BACKLIGHT, LCD_F_SET_8_BIT_2_LINE_8_FONT);

	twiSendMasterData(data,2,lcd->address,NULL);
	twiInterrupt(NULL);
	_delay_ms(5);
	twiSendMasterData(data,2,lcd->address,NULL);
	twiInterrupt(NULL);
	_delay_ms(5);
	twiSendMasterData(data,2,lcd->address,NULL);
	twiInterrupt(NULL);
	_delay_ms(5);

	data[0]=(uint8_t)(*splitFunction)(LCD_COMMAND | BACKLIGHT | ENABLE, lcd->configInitArray[0]);
	data[1]=(uint8_t)(*splitFunction)(LCD_COMMAND | BACKLIGHT, lcd->configInitArray[0]);
	twiSendMasterData(data,2,lcd->address,freePackageData);
	twiInterrupt(NULL);

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		for (int i=0;i<lcd->configInitArraySize;i++){
			data=malloc(4);
			*((uint32_t*)data)=packageEnableSplit(LCD_COMMAND | BACKLIGHT ,lcd->configInitArray[i],splitFunction);
			twiSendMasterData(data,4,lcd->address,freePackageData);
		}
	}
	twiInterrupt(NULL);

	for (uint8_t i='A';i<'z';++i){
		data=malloc(4);
		*((uint32_t*)data)=packageEnableSplit(LCD_WRITE_CG_OR_DDR |BACKLIGHT,(uint8_t)i,splitFunction);
		twiSendMasterData(data,4,lcd->address,freePackageData);
		twiInterrupt(NULL);
	}
/*
	twiSendMasterData(waitForBSFlagData,4,lcd->address,NULL);
	data=malloc(2);
	twiReadMasterData(data,2,lcd->address,waitForBSFlagFunc);
	usartSafeTransmit(receivedDataPCF8574_DataHigh(data[0],data[1]));
*/
}
