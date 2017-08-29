#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include <stdlib.h>


#define BAUD 9600

typedef struct CommPackage{
	const __memx uint8_t* package;
	uint8_t size;
	bool dynamic;
} CommPackage;

typedef struct CommNode CommNode;

struct CommNode{
	CommNode* volatile next;
	CommPackage package;
};

typedef struct CommQueue{
	CommNode* volatile head;
	CommNode* volatile tail;
	volatile bool isEmpty;
    volatile uint16_t counter;
} CommQueue;

CommQueue* usartToSendQueue(void){
	static CommQueue usartToSendQueue={NULL,NULL,true,0};
	return &usartToSendQueue;
}

CommQueue* usartReceivedQueue(void){
	static CommQueue usartReceivedQueue={NULL,NULL,true,0};
	return &usartReceivedQueue;
}

void queue(CommQueue* queue, CommPackage package){
    if (queue==NULL)
        return;
	CommNode* tmpNode=malloc(sizeof(CommNode));
	tmpNode->next=NULL;
	tmpNode->package=package;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		if (queue->head==NULL){
			queue->head=tmpNode;
			queue->tail=tmpNode;
			queue->isEmpty=false;
		}
		else{
			queue->tail->next=tmpNode;
			queue->tail=tmpNode;
		}
		queue->counter++;
	}
}

CommPackage dequeue(CommQueue* queue){
    if (queue==NULL || queue->head==NULL)
		return (CommPackage){NULL,0,false};
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    	CommNode* tmpNode=queue->head;
    	queue->head=queue->head->next;
    
    	if (queue->head==NULL){
    		queue->tail=NULL;
    		queue->isEmpty=true;
    	}
    	CommPackage retPackage=tmpNode->package;
    	free(tmpNode);
    	queue->counter--;
    	return retPackage;
    }
    return (CommPackage){NULL,0,false};
}

void usartTransmit(uint8_t data) {
/* Wait for empty transmit buffer */
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}

uint8_t usartReceive(void){
/* Wait for data to be received */
	while (!(UCSR0A & (1<<RXC0))){};
	return UDR0;

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

ISR(USART_TX_vect){
	static bool completedTransmission=true;
	static uint16_t marker;
	if (!usartToSendQueue()->isEmpty){
	    CommPackage toSend=usartToSendQueue()->head->package;
		if (!completedTransmission){
            marker++;
            if (marker==toSend.size){
                 completedTransmission=true;
                 toSend=dequeue(usartToSendQueue());
                 if (toSend.dynamic)
                    free(toSend.package);
             }
             else
		       usartTransmit(*(toSend.package+marker));            
		}
		else{
            completedTransmission=false;
    		marker=0;
            if (toSend.size>0)
                usartTransmit(*toSend.package);
            else
                completedTransmission=true;
		}
    }
}

ISR(USART_RX_vect){
	static bool completedTransmission=true;
	static uint16_t marker;
	static CommPackage received;
    received.dynamic=true;
	if (!completedTransmission){
    	*((uint8_t*)received.package+marker)=usartReceive();
        marker++;
		if(marker>=received.size){
			completedTransmission=true;
            queue(usartReceivedQueue(),received);
        }
	}
	else {
        marker=0;
        completedTransmission=false;
        received.size=usartReceive();
        if (received.size==0)
            completedTransmission=true;
        else
            received.package=malloc(received.size);
	}
}

inline void usartSendText(const __memx char* text, uint8_t size, bool dynamic){
	queue(usartToSendQueue(),(CommPackage){(const __memx uint8_t*)text,size-1,dynamic});
	USART_TX_vect();
}

inline const char* usartGetText(){
	return (const char*)dequeue(usartReceivedQueue()).package;
}

inline void usartSendData(const __memx uint8_t* data, uint8_t size, bool dynamic){
	queue(usartToSendQueue(),(CommPackage){data,size,dynamic});
	USART_TX_vect();
}
inline CommPackage usartGetData(void){
	return dequeue(usartReceivedQueue());
}

ISR(TWI_vect){
	uint8_t twiStatus=TWSR>>3;
	twiStatus<<=3;
	switch(twiStatus){
	default:{
		uint8_t* twiStaTemp=malloc(1);
		*twiStaTemp=twiStatus;
		usartSendData(twiStaTemp,1,true);
	}
	}
}

void i2cInit(uint32_t freq){
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
