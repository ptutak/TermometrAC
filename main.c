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
    PORTB^=1<<PB5;
    _delay_ms(500);
}

void initSystem(OsPackage* package){
	usartInit(BAUD);

    addOsPriorFunc(osStaticPriorQueue(),resendUsartMsg,NULL,0,false,500);


    DDRB=1<<PB5;

	twiInit(TWI_FREQ,true);
	addOsPriorFunc(osStaticPriorQueue(),twiInterrupt,NULL,0,false,100);
    LCD lcd;
    lcd.address=0x40;
    lcd.configInitArray=LCD_CONFIG_INIT_2X16S;
    lcd.configInitArraySize=LCD_CONFIG_INIT_2X16S_SIZE;

    _delay_ms(1000);

    lcdInit(&lcd,splitDataPCF8574_DataHigh);

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
