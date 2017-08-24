#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdbool.h>

#define BAUD 9600

typedef struct CommText{
	const __memx char* text;
	uint8_t size;
	bool dynamic;
}CommText;

typedef struct CommNode CommNode;

struct CommNode{
	CommNode* next;
	CommText text;
};

typedef struct CommQueue{
	CommNode* head;
	CommNode* tail;
	bool isEmpty;
}CommQueue;



CommQueue* getUsartToSendQueue(void){
	static CommQueue usartToSendQueue={NULL,NULL,true};
	return &usartToSendQueue;
}

CommQueue* getUsartReceivedQueue(void){
	static CommQueue usartReceivedQueue={NULL,NULL,true};
	return &usartReceivedQueue;
}

void queue(CommQueue* queue, CommText text){
	CommNode* tmpNode=malloc(sizeof(CommNode));
	tmpNode->next=NULL;
	tmpNode->text=text;
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

CommText dequeue(CommQueue* queue){
	if (queue->head==NULL){
		CommText nullText={NULL,0,false};
		return nullText;
	}
	CommNode* tmpNode=queue->head;
	queue->head=queue->head->next;
	if (queue->head==NULL){
		queue->tail=NULL;
		queue->isEmpty=true;
	}
	CommText retText=tmpNode->text;
	free(tmpNode);
	return retText;
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
	if (!getUsartToSendQueue()->isEmpty)
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			CommText toSend=dequeue(getUsartToSendQueue());
			const __memx char* p=toSend.text;
			usartTransmit((uint8_t)toSend.size);
			while(p){
				usartTransmit(*p);
				p++;
			}
			if (toSend.dynamic)
				free(toSend.text);

		}
}

ISR(USART_RX_vect){
	static completedTransmission=true;
	static uint8_t marker;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		CommText received;
		if (!completedTransmission){
			received=getUsartReceivedQueue()->head->text;
			*(received.text+marker)=usartRecive();
			marker++;
			if(marker==received.size){
				completedTransmission=true;
				*(received.text+marker)='\0';
			}
		}
		else {
			received.size=usartReceive();
			received.dynamic=true;
			received.text=malloc(received.size+1);
			marker=0;
		}

	}

}


int main(void){
	usartInit(BAUD);


}
