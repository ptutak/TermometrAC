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
/*
Package* bsFlagPackage(uint8_t address){
	static Package package;
	package=(Package){.tPackage={waitForBSFlagData,2,address,'R',TWI_STD_TTL,0,TWI_NULL,NULL}};
	return &package;
}

void readBSFlagAndAddr(TwiPackage* package){
	if (*(package->data)&0b10000000){
		insert(twiMasterQueue(),&(Package){.tPackage=*package},0);
		insert(twiMasterQueue(),bsFlagPackage(package->address),0);
		return;
	}
}
*/

void lcdNextFunc(TwiPackage* package){
	addOsFunc(osDynamicQueue(),twiInterrupt,NULL,0,false);
	free((uint8_t*)package->data);
}

static inline uint32_t packageEnableSplit(uint8_t instruction,uint8_t data, uint16_t (*splitFunction)(uint8_t instruction, uint8_t data)){
	uint16_t enData=(*splitFunction)(instruction|ENABLE,data);
	uint16_t origData=(*splitFunction)(instruction,data);
	return ((uint32_t)((uint8_t)enData)) | (((uint32_t)((uint8_t)origData))<<8) | (((uint32_t)(enData & 0xFF00))<<8) | (((uint32_t)(origData & 0xFF00))<<16);
}

uint8_t* readBSFlagAndAddrFunc(LCD* lcd){
	uint8_t* waitForBSFlagData=malloc(4);
	*((uint32_t*)waitForBSFlagData)=packageEnableSplit(LCD_READ_BS_FLAG_AND_ADDR | lcd->backlight,LCD_FULL,lcd->splitFunction);
	twiSendMasterData(waitForBSFlagData,4,lcd->address,freeTwiPackageData);

	uint8_t* addr=malloc(2);
	twiReadMasterData(addr,2,lcd->address,NULL);
	return addr;
}

void lcdInit(LCD* lcd){
	if (lcd->configInitArraySize==0)
		return;

	uint8_t* data=malloc(2);
	data[0]=(uint8_t)(*lcd->splitFunction)(LCD_COMMAND | lcd->backlight | ENABLE, LCD_F_SET_8_BIT_2_LINE_8_FONT);
	data[1]=(uint8_t)(*lcd->splitFunction)(LCD_COMMAND | lcd->backlight, LCD_F_SET_8_BIT_2_LINE_8_FONT);

	twiSendMasterData(data,2,lcd->address,NULL);
	twiInterrupt(NULL);
	_delay_ms(5);
	twiSendMasterData(data,2,lcd->address,NULL);
	twiInterrupt(NULL);
	_delay_ms(5);
	twiSendMasterData(data,2,lcd->address,NULL);
	twiInterrupt(NULL);
	_delay_ms(5);

	data[0]=(uint8_t)(*lcd->splitFunction)(LCD_COMMAND | lcd->backlight | ENABLE, lcd->configInitArray[0]);
	data[1]=(uint8_t)(*lcd->splitFunction)(LCD_COMMAND | lcd->backlight, lcd->configInitArray[0]);
	twiSendMasterData(data,2,lcd->address,freeTwiPackageData);
	twiInterrupt(NULL);

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		for (int i=0;i<lcd->configInitArraySize;i++){
			data=malloc(4);
			*((uint32_t*)data)=packageEnableSplit(LCD_COMMAND | lcd->backlight ,lcd->configInitArray[i],lcd->splitFunction);
			twiSendMasterData(data,4,lcd->address,freeTwiPackageData);
		}
	}
	twiInterrupt(NULL);
}

void lcdGoTo(LCD* lcd, uint8_t x, uint8_t y){
	uint8_t* data=malloc(4);
	*((uint32_t*)data)=packageEnableSplit(LCD_COMMAND | lcd->backlight,(x+40*y)|LCD_GO_TO_DDR_ADDR,lcd->splitFunction);
	twiSendMasterData(data,4,lcd->address,freeTwiPackageData);
	twiInterrupt(NULL);
}

void lcdSendText(LCD* lcd,const __memx char* tekst,uint8_t size,bool dynamic){
	uint32_t* data=malloc(sizeof(uint32_t)*size);
	for (uint8_t i=0;i<size;++i){
		data[i]=packageEnableSplit(LCD_WRITE_CG_OR_DDR | lcd->backlight, tekst[i], lcd->splitFunction);
	}
	if(dynamic)
		twiSendMasterData((uint8_t*)data,sizeof(uint32_t)*size,lcd->address,freeTwiPackageData);
	else
		twiSendMasterData((uint8_t*)data,sizeof(uint32_t)*size,lcd->address,NULL);
}

void lcdBacklightToggle(LCD* lcd){
	if (lcd->backlight!=BACKLIGHT_ON && lcd->backlight!=BACKLIGHT_OFF)
		lcd->backlight=BACKLIGHT_OFF;
	lcd->backlight^=lcd->backlight;
}

