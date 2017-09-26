#include "my_types.h"
#include "my_queue.h"
#include "my_twi.h"
#include "my_usart.h"
#include "lcd_control.h"
#include "string.h"
#include "util/delay.h"


static const uint16_t BAUD=9600;

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

LCD lcd;

void lcdBlink(OsPackage* notUsed){
	lcdBacklightToggle(&lcd);
}



void initSystem(OsPackage* package){

	usartInit(BAUD);
    addOsPriorFunc(osStaticPriorQueue(),resendUsartMsg,NULL,0,false,1000);

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

    addOsPriorFunc(osStaticPriorQueue(),lcdBlink,NULL,0,false,0xffff/2);

}



int main(void){
	addOsFunc(osInitQueue(),initSystem,NULL,0,false);

	manageOsDynamicQueue(osInitQueue());

	while(1){

//    	manageOsDynamicQueue(osDynamicQueue());
//    	manageOsQueue(osStaticQueue());
//    	manageOsDynamicPriorQueue(osDynamicPriorQueue());
    	manageOsPriorQueue(osStaticPriorQueue());

    }

}
