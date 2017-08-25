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
	while (!(UCSR0A & (1<<UDRE0))){};

	UDR0 = data;
}

uint8_t usartReceive(void){
/* Wait for data to be received */
	while (!(UCSR0A & (1<<RXC0)));

	return UDR0;
}

void usartInit(uint16_t baud){
/*Set baud rate */
	uint16_t ubrr=F_CPU/16/baud-1;
	UBRR0H = (uint8_t)(ubrr>>8);
	UBRR0L = (uint8_t)ubrr;

/*Enable receiver and transmitter and interrupts*/
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<TXCIE0)|(1<<RXCIE0);

/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

ISR(USART_TX_vect){
	static bool completedTransmission=true;
	static uint16_t marker;
	if (!usartToSendQueue()->isEmpty)
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			CommPackage toSend=usartToSendQueue()->head->package;
			if (!completedTransmission){
				usartTransmit(*(toSend.package+marker));
				marker++;
				if (marker>toSend.size){
					completedTransmission=true;
					toSend=dequeue(usartToSendQueue());
					if (toSend.dynamic)
						free(toSend.package);
				}
			}
			else{
				usartTransmit(toSend.size);
				marker=0;
			}
		}
}

ISR(USART_RX_vect){
	static bool completedTransmission=true;
	static uint16_t marker;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		CommPackage received;
		if (!completedTransmission){
			received=usartReceivedQueue()->tail->package;
			*((uint8_t*)received.package+marker)=usartReceive();
			marker++;
			if(marker>received.size)
				completedTransmission=true;
		}
		else {
			received.size=usartReceive();
			received.dynamic=true;
			received.package=malloc(received.size+1);
			queue(usartReceivedQueue(),received);
			marker=0;
		}

	}

}

void usartSendText(const __memx char* text,bool dynamic){
	CommPackage newPackage={(const __memx uint8_t*)text,sizeof(text),dynamic};
	queue(usartToSendQueue(),newPackage);
	UCSR0A|=1<<TXC0;
}

const char* usartGetText(){
	CommPackage receivedPackage=dequeue(usartReceivedQueue());
	return (const char*)receivedPackage.package;
}


int main(void){
	usartInit(BAUD);

}
