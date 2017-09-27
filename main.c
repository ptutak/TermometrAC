#include "my_types.h"
#include "my_queue.h"
#include "my_twi.h"
#include "my_usart.h"
#include "lcd_control.h"
#include "string.h"
#include "util/delay.h"
#include "termometr.h"
#include "stdlib.h"


static const uint16_t BAUD=9600;

LCD lcd;




void resendUsartMsg(OsPackage* package){
    static const char* received;
    static uint8_t size=0;

    if(!usartReceivedQueue()->isEmpty){
        received=usartGetText();
        while(*(received+size))
            size++;
        usartSendText(received,size,true);
        size=0;
    }
}

uint8_t myLen(const char* str){
    uint8_t len=0;
    while(*str++)
        len++;
    return len;
}


void lcdSetTemperature(OsPackage* lcdPackage){
	static char tempLow[5];
	static char tempAll[10];
    for (uint8_t i=0;i<5;++i)
        tempLow[i]=0;
    for (uint8_t i=0;i<10;++i)
        tempAll[i]=0;
    int temp=getTemperature(PC1,PC0,1970);
	itoa(temp/10,tempAll,10);
	itoa(temp%10,tempLow,10);
    strcat(tempAll,",");
	strcat(tempAll,tempLow);
   	strcat(tempAll,"\n");
    usartSafeTransmit(tempAll[0]);
    usartSafeTransmit('\n'); 
	usartSendText(tempAll,strlen(tempAll),false);
 /*   lcdClear(&lcd);
    lcdGoTo(&lcd,0,0);
    lcdSendText(&lcd,tempAll,strlen(tempAll),true);
    */
    twiManageQueue(twiMasterQueue());
}

void initSystem(OsPackage* package){


	usartInit(BAUD);
 //   addOsPriorFunc(osStaticPriorQueue(),resendUsartMsg,NULL,0,false,1000);

    twiInit(TWI_FREQ,true);
	addOsPriorFunc(osStaticPriorQueue(),twiInterrupt,NULL,0,false,1000);

    lcd.address=0x40;
    lcd.configInitArray=LCD_CONFIG_INIT_2X16S;
    lcd.configInitArraySize=LCD_CONFIG_INIT_2X16S_SIZE;
    lcd.sendSplit=splitDataPCF8574_DataHigh;
    lcd.receivedMerge=receivedDataPCF8574_DataHigh;
    lcd.backlight=BACKLIGHT_ON;

    lcdInit(&lcd);

    usartSendText(PSTR("System init\n"),sizeof("System init\n")-1,false);
    lcdSendText(&lcd,PSTR("Czesc Magda!"),sizeof("Czesc Magda!")-1,false);
    lcdGoTo(&lcd,0,1);
    lcdSendText(&lcd,PSTR("HURRA Dziala ;)"),sizeof("HURRA Dziala ;)")-1,false);
    twiManageQueue(twiMasterQueue());
    addOsPriorFunc(osStaticPriorQueue(),lcdSetTemperature,NULL,0,false,0xffff/4);
}



int main(void){

	addOsFunc(osInitQueue(),initSystem,NULL,0,false);

	manageOsDynamicQueue(osInitQueue());

	while(1){
    	manageOsDynamicQueue(osDynamicQueue());
    	manageOsQueue(osStaticQueue());
    	manageOsDynamicPriorQueue(osDynamicPriorQueue());
    	manageOsPriorQueue(osStaticPriorQueue());
    }

}
