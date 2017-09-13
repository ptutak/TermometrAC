#include "my_types.h"
#include "my_queue.h"
#include "my_twi.h"
#include "my_usart.h"
#include "lcd_control.h"
#include "string.h"
#include "util/delay.h"


static const uint16_t BAUD=9600;


static inline void usartTransmit(uint8_t data) {
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}


int main(void){
	usartInit(BAUD);
	DDRB=1<<PB5;
    const char* received;
    uint8_t size=0;
    usartSendText(PSTR("Czesc\n"),sizeof("Czesc\n"),false);
/*
    twiInit(TWI_FREQ,true);
    LCD lcd;
    lcd.address=0x4E;
    lcd.configInitArray=LCD_CONFIG_INIT_2X16S;
    lcd.configInitArraySize=LCD_CONFIG_INIT_2X16S_SIZE;
    lcdInit(&lcd,splitDataPCF8574_DataHigh);
*/
    usartSendText(PSTR("InitDone\n"),sizeof("InitDone\n"),false);
    while(1){
        if(!usartReceivedQueue()->isEmpty){
            received=usartGetText();
            while(*(received+size))
                size++;
            size++;
            usartTransmit('h');
            usartTransmit('\n');
            usartSendText(received,size,true);
            size=0; 
        }
        PORTB^=1<<PB5;
        _delay_ms(500);
    }

}
