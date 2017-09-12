#include "my_types.h"
#include "my_queue.h"
#include "my_twi.h"
#include "my_usart.h"
#include "lcd_control.h"
#include "string.h"
#include "util/delay.h"


static const uint16_t BAUD=9600;


int main(void){
	usartInit(BAUD);
	DDRB=1<<PB5;
    const char __flash * text=PSTR("Czesc\n");
    const char __flash* msg=PSTR("Msg1\n");
    const char* received;
    uint8_t size=0;
    usartSendText(text,7,false);

    _delay_ms(5000);
    twiInit(TWI_FREQ,true);
    LCD lcd;
    lcd.address=0x4E;
    lcd.configInitArray=LCD_CONFIG_INIT_2X16S;
    lcd.configInitArraySize=LCD_CONFIG_INIT_2X16S_SIZE;
    lcdInit(&lcd,splitDataPCF8574_DataHigh);

    PORTB=1<<PB5;
    while(1){
        if(!usartReceivedQueue()->isEmpty){
            received=usartGetText();
            while(*(received+size))
                size++;
            size++;
            usartSendText(received,size,true);
            size=0; 
        }
    }

}
