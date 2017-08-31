#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <util/twi.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include <stdlib.h>



#define BAUD 9600



typedef struct{
	const __memx uint8_t* data;
	uint8_t size;
	bool dynamic;
} UsartPackage;





typedef struct{
	volatile bool start : 1;
	volatile bool slaw : 1;
	volatile bool slar : 1;
	volatile bool data : 1;
	volatile bool stop : 1;
	volatile bool error : 1;
	bool : 0;
	volatile uint8_t address;
	volatile char mode;
	volatile uint8_t status;
} TwiControlStatus;

const TwiControlStatus NULL_TWI_CONTROL_STATUS={0,0,0,0,0,0,0,'\0',0};

const TwiControlStatus TWI_MASTER_TO_SEND_STATUS={0,0,0,0,0,0,0,'W',0};

const TwiControlStatus TWI_MASTER_RECEIVE_STATUS={0,0,0,0,0,0,0,'R',0};

void resetTwiControlStatus(TwiControlStatus* status){
	status->error=false;
	status->slar=false;
	status->slaw=false;
	status->start=false;
	status->stop=false;
}

typedef struct{
	const __memx uint8_t* data;
	uint8_t size;
	bool dynamic;
	TwiControlStatus twiControlStatus;
} TwiPackage;





typedef union{
	UsartPackage uPackage;
	TwiPackage tPackage;
}Package;

const Package NULL_PACKAGE={.tPackage={NULL,0,false,{0,0,0,0,0,0,0,'\0',0}}};




typedef struct CommNode CommNode;

struct CommNode{
	CommNode* volatile next;
	union{
			UsartPackage uPackage;
			TwiPackage tPackage;
	};
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

CommQueue* twiCommQueue(void){
	static CommQueue twiCommQueue={NULL,NULL,true,0};
	return &twiCommQueue;
}


CommQueue* twiToSendQueue(void){
	static CommQueue twiToSendQueue={NULL,NULL,true,0};
	return &twiToSendQueue;
}

CommQueue* twiReceivedQueue(void){
	static CommQueue twiReceivedQueue={NULL,NULL,true,0};
	return &twiReceivedQueue;
}

void queue(CommQueue* queue, void* package, char type){
    if (queue==NULL)
        return;
	CommNode* tmpNode=malloc(sizeof(CommNode));
	tmpNode->next=NULL;
	switch(type){
	case 'u':
		tmpNode->uPackage=*((UsartPackage*)package);
		break;
	case 't':
		tmpNode->tPackage=*((TwiPackage*)package);
		break;
	}
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

Package dequeue(CommQueue* queue,char type){
    if (queue==NULL || queue->head==NULL)
    	switch(type){
    	case 'u':
    	case 't':
    	default:
    		return NULL_PACKAGE;
    	}
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    	CommNode* tmpNode=queue->head;
    	queue->head=queue->head->next;
    
    	if (queue->head==NULL){
    		queue->tail=NULL;
    		queue->isEmpty=true;
    	}
    	Package retPackage;
    	switch(type){
    	case 'u':
    		retPackage.uPackage=tmpNode->uPackage;
    		break;
    	case 't':
    		retPackage.tPackage=tmpNode->tPackage;
    		break;
    	}
    	free(tmpNode);
    	queue->counter--;
    	return retPackage;
    }
    return NULL_PACKAGE;
}

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






inline void twiStart(){
	TWCR=1<<TWEN|1<<TWEA|1<<TWIE|1<<TWSTA;
}

inline void twiAddress(uint8_t address, char mode){
	switch (mode){
	case 'w':
		address&= ~(1);
		break;
	case 'r':
		address|=1;
		break;
	}
	TWDR=address;
	TWCR=1<<TWEN|1<<TWEA|1<<TWIE;
}

inline void twiData(uint8_t data){
	TWDR=data;
	TWCR=1<<TWEN|1<<TWEA|1<<TWIE;
}

inline void twiStop(){
	TWCR=1<<TWEN|1<<TWEA|1<<TWIE|1<<TWSTO;
}


ISR(TWI_vect){
	uint8_t twiStatusReg=TWSR & (0b11111000);
	static bool orderDone=true;
	static TwiPackage* order;
	if (twiCommQueue()->isEmpty){
		return;
	} else if (orderDone){
		order=&(twiCommQueue()->head->tPackage);
		orderDone=false;
	}
	if (order->twiControlStatus.start)
		switch(twiStatusReg){
		case 0x08:
		case 0x10:
		default:
			resetTwiControlStatus(&order->twiControlStatus);
			order->twiControlStatus.error=true;
		}
	else if (order->twiControlStatus.slaw)
		switch(twiStatusReg){
		case 0x18:
		case 0x20:
		default:
			resetTwiControlStatus(&order->twiControlStatus);
			order->twiControlStatus.error=true;
		}
	else if (order->twiControlStatus.data)
		switch(twiStatusReg){
		case 0x28:
		case 0x30:
		case 0x38:
		default:
			resetTwiControlStatus(&order->twiControlStatus);
			order->twiControlStatus.error=true;
		}
	else if (order->twiControlStatus.slar)
		switch (twiStatusReg){
		case 0x38:
		case 0x40:
		case 0x48:
		case 0x50:
		case 0x58:
		default:
			resetTwiControlStatus(&order->twiControlStatus);
			order->twiControlStatus.error=true;

		}
	else {

	}
	/*
		uint8_t* twiStaTemp=malloc(1);
		*twiStaTemp=twiStatus;
		usartSendData(twiStaTemp,1,true);
	*/
}



void twiSendData(uint8_t* data, uint8_t size, bool dynamic,uint8_t address){
	queue(twiCommQueue(),(void*)&((TwiPackage){data,size,dynamic,{0,0,0,0,0,0,address,'W',0}}),'t');
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
