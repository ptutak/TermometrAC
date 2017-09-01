#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <util/twi.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include <stdlib.h>
#include "my_twi.h"
#include "my_usart.h"


static const uint16_t BAUD=9600;







inline void usartTransmit(uint8_t data) {
/* Wait for empty transmit buffer */
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}

inline uint8_t usartReceive(void){
/* Wait for data to be received */
	while (!(UCSR0A & (1<<RXC0))){};
	return UDR0;

}




ISR(USART_TX_vect){
	static bool completedTransmission=true;
	static uint16_t marker;
	if (!usartToSendQueue()->isEmpty){
	    UsartPackage toSend=usartToSendQueue()->head->uPackage;
		if (!completedTransmission){
            marker++;
            if (marker==toSend.size){
                 completedTransmission=true;
                 toSend=(dequeue(usartToSendQueue(),'u')).uPackage;
                 if (toSend.dynamic)
                    free((uint8_t*)toSend.data);
             }
             else
		       usartTransmit(*(toSend.data+marker));            
		}
		else{
            completedTransmission=false;
    		marker=0;
            if (toSend.size>0)
                usartTransmit(*toSend.data);
            else
                completedTransmission=true;
		}
    }
}

ISR(USART_RX_vect){
	static bool completedTransmission=true;
	static uint16_t marker;
	static UsartPackage received;
    received.dynamic=true;
	if (!completedTransmission){
    	*((uint8_t*)received.data+marker)=usartReceive();
        marker++;
		if(marker>=received.size){
			completedTransmission=true;
            queue(usartReceivedQueue(),(void*)&received,'u');
        }
	}
	else {
        marker=0;
        completedTransmission=false;
        received.size=usartReceive();
        if (received.size==0)
            completedTransmission=true;
        else
            received.data=malloc(received.size);
	}
}





inline void usartSendText(const __memx char* text, uint8_t size, bool dynamic){
	queue(usartToSendQueue(),(void*)&((UsartPackage){(const __memx uint8_t*)text,size-1,dynamic}),'u');
	USART_TX_vect();
}

inline const char* usartGetText(){
	return (const char*)(dequeue(usartReceivedQueue(),'u')).uPackage.data;
}

inline void usartSendData(const __memx uint8_t* data, uint8_t size, bool dynamic){
	queue(usartToSendQueue(),(void*)&((UsartPackage){data,size,dynamic}),'u');
	USART_TX_vect();
}
inline UsartPackage usartGetData(void){
	return dequeue(usartReceivedQueue(),'u').uPackage;
}





inline void twiStart(bool twea){
	TWCR=1<<TWEN|1<<TWEA|1<<TWIE|1<<TWSTA;
}

inline void twiAddress(uint8_t address, char mode, bool twea){
	switch (mode){
	case 'w':
	case 'W':
		address&= ~(1);
		break;
	case 'r':
	case 'R':
		address|=1;
		break;
	}
	TWDR=address;
	TWCR=1<<TWEN|twea<<TWEA|1<<TWIE;
}

inline void twiData(uint8_t data, bool twea){
	TWDR=data;
	TWCR=1<<TWEN|twea<<TWEA|1<<TWIE;
}

inline void twiStop(bool twea){
	TWCR=1<<TWEN|twea<<TWEA|1<<TWIE|1<<TWSTO;
}

inline void twiStopStart(bool twea){
	TWCR=1<<TWEN|twea<<TWEA|1<<TWIE|1<<TWSTO|1<<TWSTA;
}





inline void twiStartAction(TwiPackage* order,uint8_t twiStatusReg){
	switch(twiStatusReg){
	case 0x08:

	case 0x10:
		switch(order->twiControlStatus.mode){
		case 'W':
			order->twiControlStatus.control=TWI_SLAW;
			twiAddress(order->twiControlStatus.address,'W',true);
			break;
		case 'R':
			order->twiControlStatus.control=TWI_SLAR;
			twiAddress(order->twiControlStatus.address,'R',true);
			break;
		}
		break;
	default:
		resetTwiControlStatus(&order->twiControlStatus);
		order->twiControlStatus.status=twiStatusReg;
		order->twiControlStatus.control=TWI_ERROR;
	}
}


inline void twiSlawAction(TwiPackage* order, uint8_t twiStatusReg){
	switch(twiStatusReg){
	case 0x18:
		if (order->size>0){
			order->twiControlStatus.control=TWI_DATA;
			twiData(*(order->data),true);
		}
		else {
			order->twiControlStatus.control=TWI_STOP;
			twiStop(true);
		}
		break;
	case 0x20:
	default:
		resetTwiControlStatus(&order->twiControlStatus);
		order->twiControlStatus.control=TWI_ERROR;
	}
}


inline void twiSlarAction(TwiPackage* order, uint8_t twiStatusReg){
	switch (twiStatusReg){
	case 0x38:
	case 0x40:
	case 0x48:
	case 0x50:
	case 0x58:
	default:
		resetTwiControlStatus(&order->twiControlStatus);
		order->twiControlStatus.status=twiStatusReg;
		order->twiControlStatus.control=TWI_ERROR;
	}

}

inline void twiDataAction(TwiPackage* order, uint8_t twiStatusReg){
	switch(twiStatusReg){
	case 0x28:
	case 0x30:
	case 0x38:
	default:
		resetTwiControlStatus(&order->twiControlStatus);
		order->twiControlStatus.status=twiStatusReg;
		order->twiControlStatus.control=TWI_ERROR;
	}
}


ISR(TWI_vect){
	uint8_t twiStatusReg=TWSR & (0b11111000);
	static bool orderDone=true;
	static TwiPackage* order=NULL;
	static uint8_t counter=0;
	if (twiMasterQueue()->isEmpty){
		return;
	}
	if (orderDone){
		order=&(twiMasterQueue()->head->tPackage);
		orderDone=false;
		order->twiControlStatus.control=TWI_START;
		twiStart(true);
		return;
	}
	switch(order->twiControlStatus.control){
	case TWI_START:
		twiStartAction(order,twiStatusReg);
		break;
	case TWI_SLAW:
		twiSlawAction(order,twiStatusReg);
		break;
	case TWI_SLAR:
		twiSlarAction(order,twiStatusReg);
		break;
	case TWI_DATA:
		twiDataAction(order,twiStatusReg);
		break;
	default:;
	}
	uint8_t* twiStaTemp=malloc(1);
	*twiStaTemp=twiStatusReg;
	usartSendData(twiStaTemp,1,true);
}



void twiSendData(uint8_t* data, uint8_t size, bool dynamic,uint8_t address){
	queue(twiMasterQueue(),(void*)&((TwiPackage){data,size,dynamic,{TWI_NULL,address,'W',0}}),'t');
}



void twiInit(uint32_t freq){
	TWCR=1<<TWEN|1<<TWEA|1<<TWIE;
	uint32_t twbr=(F_CPU/freq - 16)/2;
	uint8_t prescaler=0;
	while (twbr>255){
		twbr/=4;
		prescaler++;
	}
	TWSR&=0;
	TWSR|=prescaler;
	TWBR=twbr;
}

void usartInit(uint16_t baud){
/*Set baud rate */
	uint16_t ubrr=F_CPU/16/baud-1;
	UBRR0H = (uint8_t)(ubrr>>8);
	UBRR0L = (uint8_t)ubrr;

/*Enable receiver and transmitter and interrupts*/
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<TXCIE0)|(1<<RXCIE0);

/* Set frame format: 8data, 1stop bit */
	UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);
}


int main(void){
	usartInit(BAUD);
    DDRB|=1<<PB5;
    PORTB^=1<<PB5;
    const char __flash * text=PSTR("Czesc\n");
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
