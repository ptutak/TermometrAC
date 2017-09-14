#include "my_usart.h"

static inline void usartTransmit(uint8_t data) {
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}

static inline uint8_t usartReceive(void){
	while (!(UCSR0A & (1<<RXC0))){};
	return UDR0;

}

void usartSafeTransmit(uint8_t data) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		uint8_t ucsr0bBackup=UCSR0B&(1<<TXCIE0);
		UCSR0B&=~(1<<TXCIE0);
		while (!(UCSR0A & (1<<UDRE0)));
		UDR0 = data;
		while (!(UCSR0A & (1<<TXC0)));
		UCSR0A|=1<<TXC0;
		UCSR0B|=ucsr0bBackup;
	}
}



CommQueue* usartToSendQueue(void){
	static CommQueue usartToSendQueue={NULL,NULL,true,0};
	return &usartToSendQueue;
}

CommQueue* usartReceivedQueue(void){
	static CommQueue usartReceivedQueue={NULL,NULL,true,0};
	return &usartReceivedQueue;
}


ISR(USART_TX_vect,ISR_NOBLOCK){
	static bool busy=false;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		if (busy)
			return;
		busy=true;
	}
	static bool completedTransmission=true;
	static uint16_t marker=0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
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
				else{
					toSend=(dequeue(usartToSendQueue(),'u')).uPackage;
					if (toSend.dynamic)
						free((uint8_t*)toSend.data);
				}
			}
		}
		busy=false;
	}
	if (completedTransmission && (!usartToSendQueue()->isEmpty))
		USART_TX_vect();
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

void usartSendText(const __memx char* text, uint8_t size, bool dynamic){
	queue(usartToSendQueue(),(void*)&((UsartPackage){(const __memx uint8_t*)text,size-1,dynamic}),'u');
	USART_TX_vect();
}

void usartSendData(const __memx uint8_t* data, uint8_t size, bool dynamic){
	queue(usartToSendQueue(),(void*)&((UsartPackage){data,size,dynamic}),'u');
	USART_TX_vect();
}

const char* usartGetText(){
	return (const char*)(dequeue(usartReceivedQueue(),'u')).uPackage.data;
}

UsartPackage usartGetData(void){
	return dequeue(usartReceivedQueue(),'u').uPackage;
}

void usartInit(uint16_t baud){
	uint16_t ubrr=F_CPU/16/baud-1;
	UBRR0H = (uint8_t)(ubrr>>8);
	UBRR0L = (uint8_t)ubrr;

	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<TXCIE0)|(1<<RXCIE0);

	UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);
}
