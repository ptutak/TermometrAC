#include "lcd_control.h"




LCDCommand LCD_CONFIG_INIT_2X16S[]={
		LCD_F_SET_4_BIT_2_LINE_8_FONT,
		LCD_SET_DISPLAY|DISPLAY_ON|BLINKING_ON,
		LCD_CLEAR_DISPLAY,
		LCD_SET_INCREMENT
};

uint8_t LCD_CONFIG_INIT_2X16S_SIZE=sizeof(LCD_CONFIG_INIT_2X16S)/sizeof(LCDCommand);





uint16_t splitDataPCF8574_DataHigh(uint8_t instructionType, uint8_t data){
	return ((uint16_t)(instructionType|(data & 0xF0)))|(((uint16_t)(instructionType|(data<<4)))<<8);
}

uint8_t receivedDataPCF8574_DataHigh(uint8_t high, uint8_t low){
	return (high & 0xF0)|((low & 0xF0)>>4);
}





static inline uint32_t packageEnableSplit(uint8_t instruction,uint8_t data, uint16_t (*splitFunction)(uint8_t instruction, uint8_t data)){
	uint16_t enData=(*splitFunction)(instruction|ENABLE,data);
	uint16_t origData=(*splitFunction)(instruction,data);
	return ((uint32_t)((uint8_t)enData)) | (((uint32_t)((uint8_t)origData))<<8) | (((uint32_t)(enData & 0xFF00))<<8) | (((uint32_t)(origData & 0xFF00))<<16);
}




void freeTwi2msDelay(TwiPackage* package){
	free((uint8_t*)package->data);
	_delay_ms(2);
}

void freeTwi50usDelay(TwiPackage* package){
	free((uint8_t*)package->data);
	_delay_us(50);
}

void freeTwi37usDelay(TwiPackage* package){
	free((uint8_t*)package->data);
	_delay_us(37);
}




void lcdInit(LCD* lcd){
	if (lcd->configInitArraySize==0)
		return;

	uint8_t* data=malloc(2);
	data[0]=(uint8_t)(*lcd->sendSplit)(LCD_COMMAND | lcd->backlight | ENABLE, LCD_F_SET_8_BIT_2_LINE_8_FONT);
	data[1]=(uint8_t)(*lcd->sendSplit)(LCD_COMMAND | lcd->backlight, LCD_F_SET_8_BIT_2_LINE_8_FONT);

	twiSendMasterData(data,2,lcd->address,NULL);
    twiManageQueue(twiMasterQueue());
    _delay_ms(5);

	twiSendMasterData(data,2,lcd->address,NULL);
    twiManageQueue(twiMasterQueue());
    _delay_ms(5);

	twiSendMasterData(data,2,lcd->address,NULL);
    twiManageQueue(twiMasterQueue());
    _delay_ms(5);

	data[0]=(uint8_t)(*lcd->sendSplit)(LCD_COMMAND | lcd->backlight | ENABLE, lcd->configInitArray[0]);
	data[1]=(uint8_t)(*lcd->sendSplit)(LCD_COMMAND | lcd->backlight, lcd->configInitArray[0]);
	twiSendMasterData(data,2,lcd->address,freeTwiPackageData);
    twiManageQueue(twiMasterQueue());
    _delay_ms(5);

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		for (int i=0;i<lcd->configInitArraySize;i++){
			data=malloc(4);
			*((uint32_t*)data)=packageEnableSplit(LCD_COMMAND | lcd->backlight ,lcd->configInitArray[i],lcd->sendSplit);
			twiSendMasterData(data,4,lcd->address,freeTwi2msDelay);
		}
	}
	twiManageQueue(twiMasterQueue());
}




void lcdGoTo(LCD* lcd, uint8_t x, uint8_t y){
	uint8_t* data=malloc(4);
	*((uint32_t*)data)=packageEnableSplit(LCD_COMMAND | lcd->backlight,(x+0x40*y)|LCD_GO_TO_DDR_ADDR,lcd->sendSplit);
	twiSendMasterData(data,4,lcd->address,freeTwi50usDelay);
}

void lcdSendText(LCD* lcd,const __memx char* tekst,uint8_t size,bool dynamic){
	uint32_t* data=malloc(sizeof(uint32_t)*size);
	for (uint8_t i=0;i<size;++i){
		data[i]=packageEnableSplit(LCD_WRITE_CG_OR_DDR | lcd->backlight, tekst[i], lcd->sendSplit);
	}
	twiSendMasterData((uint8_t*)data,sizeof(uint32_t)*size,lcd->address,freeTwiPackageData);
	if (dynamic)
		free((uint8_t*)tekst);
}

void lcdBacklightToggle(LCD* lcd){
	if (lcd->backlight!=BACKLIGHT_ON && lcd->backlight!=BACKLIGHT_OFF)
		lcd->backlight=BACKLIGHT_OFF;
	lcd->backlight^=lcd->backlight;
	lcdReadBsFlagAndAddr(lcd);
}

uint8_t lcdReadBsFlagAndAddr(LCD* lcd){
	uint8_t* waitForBSFlagData=malloc(4);
	*((uint32_t*)waitForBSFlagData)=packageEnableSplit(LCD_READ_BS_FLAG_AND_ADDR | lcd->backlight,LCD_FULL,lcd->sendSplit);
	twiSendMasterData(waitForBSFlagData,4,lcd->address,freeTwiPackageData);
	uint8_t* tmpData=malloc(2);
	twiReadMasterData(tmpData,2,lcd->address,NULL);
	twiManageQueue(twiMasterQueue());
	return (*lcd->receivedMerge)(tmpData[0],tmpData[1]);
}

void lcdClear(LCD* lcd){
	uint8_t* data=malloc(4);
	*((uint32_t*)data)=packageEnableSplit(LCD_COMMAND | lcd->backlight,LCD_CLEAR_DISPLAY,lcd->sendSplit);
	twiSendMasterData(data,4,lcd->address,freeTwi50usDelay);
}

void lcdHome(LCD* lcd){
	uint8_t* data=malloc(4);
	*((uint32_t*)data)=packageEnableSplit(LCD_COMMAND | lcd->backlight,LCD_RETURN_HOME,lcd->sendSplit);
	twiSendMasterData(data,4,lcd->address,freeTwi2msDelay);
}

