#include "lcd_control.h"




const __flash LCDCommandS LCD_CONFIG_INIT_2X16S[5]={
		{LCD_COMMAND,LCD_F_SET_4_BIT_2_LINE_8_FONT},
		{LCD_COMMAND,LCD_SET_DISPLAY_OFF},
		{LCD_COMMAND,LCD_CLEAR_DISPLAY},
		{LCD_COMMAND,LCD_SET_INCREMENT},
		{LCD_COMMAND,LCD_SET_DISPLAY_ON_CURSOR_ON_BLINKING_ON}
};

const __flash uint8_t LCD_CONFIG_INIT_2X16S_SIZE=5;

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
	package=(Package){.tPackage={waitForBSFlag,2,address,'R',TWI_STD_TTL,0,TWI_NULL,waitForBSFlagFunc}};
	return &package;
}

void waitForBSFlagFunc(TwiPackage* package){
	if (*(package->data)&0b00001000)
		insert(twiMasterQueue(),waitForBSFlagPackage(package->address),0);
}




void lcdInit(LCD* lcd, uint16_t_split (*splitFunction)(LCDCommandType type,uint8_t data)){
//	usartSendText(PSTR("lcdInit\n"),sizeof("lcdInit\n"),false);

	if (lcd->configInitArraySize==0)
		return;

	uint16_t_split configData=(*splitFunction)(lcd->configInitArray[0].commandType,lcd->configInitArray[0].command);

	uint8_t data[2];
	data[0]=configData.high;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		twiSendMasterDataNoInterrupt(data,1,lcd->address,NULL);
		twiSendMasterDataNoInterrupt(waitForBSFlag,2,lcd->address,waitForBSFlagFunc);
		for (int i=0;i<lcd->configInitArraySize;i++){
			configData=(*splitFunction)(lcd->configInitArray[i].commandType,lcd->configInitArray[i].command);
			data[0]=configData.high;
			data[1]=configData.low;
			twiSendMasterDataNoInterrupt(data,2,lcd->address,NULL);
			twiSendMasterDataNoInterrupt(waitForBSFlag,2,lcd->address,waitForBSFlagFunc);
		}
	}

	twiManageOrders();

}
