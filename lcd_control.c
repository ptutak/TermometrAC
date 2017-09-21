#include "lcd_control.h"




LCDCommand LCD_CONFIG_INIT_2X16S[]={
		LCD_F_SET_4_BIT_1_LINE_8_FONT,
		LCD_SET_DISPLAY|DISPLAY_ON|BLINKING_ON,
		LCD_CLEAR_DISPLAY
//		LCD_SET_INCREMENT
};

uint8_t LCD_CONFIG_INIT_2X16S_SIZE=sizeof(LCD_CONFIG_INIT_2X16S)/sizeof(LCDCommand);


uint16_t_split splitDataPCF8574_DataHigh(LCDInstructionType instructionType, uint8_t data){
	return (uint16_t_split){(uint8_t)instructionType|(data<<4),(uint8_t)instructionType|(data&(0xF0))};
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

void lcdInit(LCD* lcd, uint16_t_split (*splitFunction)(LCDInstructionType type,uint8_t data)){
	if (lcd->configInitArraySize==0)
		return;

	uint16_t_split configData;
	uint16_t_split configDataE;
	uint8_t* data=malloc(4);
	waitForBSFlagData=malloc(4);
	configDataE=(*splitFunction)(LCD_READ_BS_FLAG_AND_ADDR | BACKLIGHT | COMMAND_ENABLE,LCD_FULL);
	configData=(*splitFunction)(LCD_READ_BS_FLAG_AND_ADDR | BACKLIGHT,LCD_FULL);
	waitForBSFlagData[0]=configDataE.high;
	waitForBSFlagData[1]=configDataE.low;
	waitForBSFlagData[2]=configData.high;
	waitForBSFlagData[3]=configData.low;

	configDataE=(*splitFunction)(LCD_COMMAND | BACKLIGHT |COMMAND_ENABLE,LCD_F_SET_8_BIT_2_LINE_8_FONT);
	configData=(*splitFunction)(LCD_COMMAND | BACKLIGHT ,LCD_F_SET_8_BIT_2_LINE_8_FONT);
	data[0]=configDataE.high;
	data[1]=configData.high;

	twiSendMasterData(data,2,lcd->address,NULL);
	twiInterrupt(NULL);
	_delay_ms(5);
	twiSendMasterData(data,2,lcd->address,NULL);
	twiInterrupt(NULL);
	_delay_ms(5);
	twiSendMasterData(data,2,lcd->address,NULL);
	twiInterrupt(NULL);
	_delay_ms(5);

	configDataE=(*splitFunction)(LCD_COMMAND| BACKLIGHT |COMMAND_ENABLE,lcd->configInitArray[0]);
	configData=(*splitFunction)(LCD_COMMAND | BACKLIGHT,lcd->configInitArray[0]);
	data[0]=configDataE.high;
	data[1]=configData.high;
	twiSendMasterData(data,2,lcd->address,freePackageData);

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		for (int i=0;i<lcd->configInitArraySize;i++){
			data=malloc(4);
			configDataE=(*splitFunction)(LCD_COMMAND| BACKLIGHT |COMMAND_ENABLE,lcd->configInitArray[i]);
			configData=(*splitFunction)(LCD_COMMAND | BACKLIGHT,lcd->configInitArray[i]);
			data[0]=configDataE.high;
			data[1]=configDataE.low;
			data[2]=configData.high;
			data[3]=configData.low;
			twiSendMasterData(data,4,lcd->address,freePackageData);
		}
	}


	configDataE=(*splitFunction)(LCD_COMMAND |BACKLIGHT|COMMAND_ENABLE,LCD_SET_DDR_ADDR|0x03);
	configData=(*splitFunction)(LCD_COMMAND|BACKLIGHT,LCD_SET_DDR_ADDR|0x03);
	data=malloc(4);
	data[0]=configDataE.high;
	data[1]=configDataE.low;
	data[2]=configData.high;
	data[3]=configData.low;
	twiSendMasterData(data,4,lcd->address,freePackageData);

	twiSendMasterData(waitForBSFlagData,4,lcd->address,NULL);
	data=malloc(2);
	twiReadMasterData(data,2,lcd->address,waitForBSFlagFunc);
	usartSafeTransmit(receivedDataPCF8574_DataHigh(data[0],data[1]));

	configDataE=(*splitFunction)(LCD_WRITE_CG_OR_DDR |BACKLIGHT|COMMAND_ENABLE,'A');
	configData=(*splitFunction)(LCD_WRITE_CG_OR_DDR|BACKLIGHT,'A');
	data=malloc(4);
	data[0]=configDataE.high;
	data[1]=configDataE.low;
	data[2]=configData.high;
	data[3]=configData.low;
	twiSendMasterData(data,4,lcd->address,freePackageData);
	twiSendMasterData(waitForBSFlagData,4,lcd->address,NULL);
	data=malloc(2);
	twiReadMasterData(data,2,lcd->address,waitForBSFlagFunc);
	usartSafeTransmit(receivedDataPCF8574_DataHigh(data[0],data[1]));

}
