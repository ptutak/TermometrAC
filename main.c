
#include "my_types.h"
#include "my_twi.h"
#include "my_usart.h"
#include "my_queue.h"
#include "string.h"


static const uint16_t BAUD=9600;


int main(void){
	usartInit(BAUD);
    DDRB|=1<<PB5;
    PORTB^=1<<PB5;
    const char __flash * text=PSTR("Czesc\n");
    const char __flash * text2=PSTR("Czesc2\n");

    const char* received;
    uint8_t size=0;
    usartSendText(text,sizeof("Czesc\n"),false);
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
