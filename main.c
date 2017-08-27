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
	CommNode* next;
	CommPackage package;
};

typedef struct CommQueue{
	CommNode* head;
	CommNode* tail;
	bool isEmpty;
} CommQueue;

CommQueue* usartToSendQueue(void){
	static CommQueue usartToSendQueue={NULL,NULL,true};
	return &usartToSendQueue;
}

CommQueue* usartReceivedQueue(void){
	static CommQueue usartReceivedQueue={NULL,NULL,true};
	return &usartReceivedQueue;
}

void queue(CommQueue* queue, CommPackage package){
	CommNode* tmpNode=malloc(sizeof(CommNode));
	tmpNode->next=NULL;
	tmpNode->package=package;
	if (queue->head==NULL){
		queue->head=tmpNode;
		queue->tail=tmpNode;
		queue->isEmpty=false;
	}
	else{
		queue->tail->next=tmpNode;
		queue->tail=tmpNode;
	}
}

CommPackage dequeue(CommQueue* queue){
	if (queue->head==NULL){
		CommPackage nullText={NULL,0,false};
		return nullText;
	}
	CommNode* tmpNode=queue->head;
	queue->head=queue->head->next;
	if (queue->head==NULL){
		queue->tail=NULL;
		queue->isEmpty=true;
	}
	CommPackage retPackage=tmpNode->package;
	free(tmpNode);
	return retPackage;
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
            
		    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    		usartTransmit(*(toSend.package+marker));
            marker++;
            if (marker==toSend.size){
				completedTransmission=true;
				toSend=dequeue(usartToSendQueue());
				if (toSend.dynamic)
					free(toSend.package);
			}
            }            
		}
		else{
            completedTransmission=false;
    		marker=0;
			usartTransmit(toSend.size);
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
		if(marker==received.size){
			completedTransmission=true;
            queue(usartReceivedQueue(),received);
        }
	}
	else {
        marker=0;
        completedTransmission=false;
        received.size=usartReceive();
        received.package=malloc(received.size);
	}
}
/*/
ISR(USART_RX_vect){
    uint8_t data=usartReceive();
    usartTransmit(data);

}
//*/
void usartSendText(const __memx char* text, uint8_t size, bool dynamic){
	CommPackage newPackage={(const __memx uint8_t*)text,size,dynamic};
	queue(usartToSendQueue(),newPackage);
	USART_TX_vect();
}

const char* usartGetText(){
	CommPackage receivedPackage=dequeue(usartReceivedQueue());
    usartTransmit((uint8_t)&receivedPackage);
	return (const char*)receivedPackage.package;
    PORTB^=1<<PB5;
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
 
 //       _delay_ms(1000);
 //       usartTransmit(255);
        if(usartReceivedQueue()->isEmpty){
            
        }
        if(!usartReceivedQueue()->isEmpty){
            received=usartGetText();
            while(*(received+size))
                size++;
            size++;
            usartSendText(usartGetText(),size,true);
            size=0;
            
           }
    }

}
