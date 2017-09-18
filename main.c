#include "my_types.h"
#include "my_queue.h"
#include "my_twi.h"
#include "my_usart.h"
#include "lcd_control.h"
#include "string.h"
#include "util/delay.h"


static const uint16_t BAUD=9600;


/*


static inline void twiStart(bool twea){
	TWCR=1<<TWEN|1<<TWINT|1<<TWSTA|1<<TWEA;
}

static inline void twiAddress(uint8_t address, char mode, bool twea){
	switch (mode){
	case 'w':
	case 'W':
		usartSafeTransmit('W');
		usartSafeTransmit('\n');
		address&= 0b11111110;
		break;
	case 'r':
	case 'R':
		usartSafeTransmit('R');
		usartSafeTransmit('\n');
		address|=1;
		break;
	}
	usartSafeTransmit(address);
	usartSafeTransmit('\n');
	TWDR=address;
	TWCR=1<<TWEN|1<<TWINT|twea<<TWEA;
	usartSafeTransmit('a');
	usartSafeTransmit('a');
	usartSafeTransmit('a');
	usartSafeTransmit('a');
	usartSafeTransmit('\n');

}

static inline void twiDataSend(uint8_t data, bool twea){
	TWDR=data;
	TWCR=1<<TWEN|1<<TWINT|twea<<TWEA;
}

static inline uint8_t twiDataReceive(bool twea){
	uint8_t data=TWDR;
	TWCR=1<<TWEN|1<<TWINT|twea<<TWEA;
	return data;
}

static inline void twiStop(bool twea){
	TWCR=1<<TWEN|1<<TWINT|1<<TWSTO|twea<<TWEA;
}
static inline void twiClearInt(bool twea){
	TWCR=1<<TWEN|1<<TWINT|twea<<TWEA;
}


static inline void twiWaitForComplete() {while (!(TWCR & 1<<TWINT));};
*/

int main(void){
	usartInit(BAUD);
	DDRB=1<<PB5;
    const char* received;
    uint8_t size=0;



	usartSafeTransmit('C');
	usartSafeTransmit('C');
	usartSafeTransmit('C');
	usartSafeTransmit('C');
	usartSafeTransmit('C');
	usartSafeTransmit('C');
	usartSafeTransmit('C');
	usartSafeTransmit('C');
	usartSafeTransmit('C');
	usartSafeTransmit('C');
	usartSafeTransmit('C');
	usartSafeTransmit('C');
	usartSafeTransmit('\n');

	twiInit(TWI_FREQ,true);
/*   twiStart(false);
    twiWaitForComplete();
    twiAddress(0x40,'W',false);
    twiWaitForComplete();

    usartSafeTransmit(TWSR);
    usartSafeTransmit('\n');

    usartSafeTransmit('H');
   	usartSafeTransmit('H');
   	usartSafeTransmit('H');
   	usartSafeTransmit('H');
   	usartSafeTransmit('\n');



/*/
    LCD lcd;
    lcd.address=0x40;
    lcd.configInitArray=LCD_CONFIG_INIT_2X16S;
    lcd.configInitArraySize=LCD_CONFIG_INIT_2X16S_SIZE;
    lcdInit(&lcd,splitDataPCF8574_DataHigh);
//*/


    while(1){
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

}
