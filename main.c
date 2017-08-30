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

const TwiControlStatus RESET_STATUS_TWI={0,0,0,0,0,0,~0,~0,~0};

typedef struct{
	const __memx uint8_t* data;
	uint8_t size;
	bool dynamic;
	TwiControlStatus twiControlStatus;
} TwiPackage;



typedef struct CommNode CommNode;

typedef union{
	UsartPackage uPackage;
	TwiPackage tPackage;
}Package;

const Package NULL_PACKAGE={.tPackage={NULL,0,false,NULL_TWI_CONTROL_STATUS}};

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
    		return NULL_PACKAGE;
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
    return (Package){NULL,0,false,NULL_TWI_CONTROL_STATUS};
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
	    UsartPackage toSend=usartToSendQueue()->head->uPackage;
		if (!completedTransmission){
            marker++;
            if (marker==toSend.size){
                 completedTransmission=true;
                 toSend=(UsartPackage)dequeue(usartToSendQueue(),'u');
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
            queue(usartReceivedQueue(),received,'u');
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
	queue(usartToSendQueue(),(UsartPackage){(const __memx uint8_t*)text,size-1,dynamic},'u');
	USART_TX_vect();
}

inline const char* usartGetText(){
	return (const char*)((UsartPackage)dequeue(usartReceivedQueue(),'u')).data;
}

inline void usartSendData(const __memx uint8_t* data, uint8_t size, bool dynamic){
	queue(usartToSendQueue(),(UsartPackage){data,size,dynamic},'u');
	USART_TX_vect();
}
inline UsartPackage usartGetData(void){
	return dequeue(usartReceivedQueue());
}

inline void resetTwiStatus(){
	twiControlStatus()->data=false;
	twiControlStatus()->error=false;
	twiControlStatus()->slar=false;
	twiControlStatus()->slaw=false;
	twiControlStatus()->start=false;
	twiControlStatus()->stop=false;
}

ISR(TWI_vect){
	uint8_t twiStatusReg=TWSR & (0b11111000);
	if (twiControlStatus()->start)
		switch(twiStatusReg){
		case 0x08:
		case 0x10:
		default:
			resetTwiStatus();
			twiControlStatus()->error=true;
		}
	else if (twiControlStatus()->slaw)
		switch(twiStatusReg){
		case 0x18:
		case 0x20:
		default:
			resetTwiStatus();
			twiControlStatus()->error=true;
		}
	else if (twiControlStatus()->data)
		switch(twiStatusReg){
		case 0x28:
		case 0x30:
		case 0x38:
		default:
			resetTwiStatus();
			twiControlStatus()->error=true;
		}
	else if (twiControlStatus()->slar)
		switch (twiStatusReg){
		case 0x38:
		case 0x40:
		case 0x48:
		case 0x50:
		case 0x58:
		default:
			resetTwiStatus();
			twiControlStatus()->error=true;

		}
	/*
		uint8_t* twiStaTemp=malloc(1);
		*twiStaTemp=twiStatus;
		usartSendData(twiStaTemp,1,true);
	*/
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

inline void twiStart(){
	TWCR=1<<TWEN|1<<TWEA|1<<TWIE|1<<TWSTA;
	resetTwiStatus();
	twiControlStatus()->start=true;
}

inline void twiAddress(uint8_t address, char mode){
	switch (mode){
	case 'w':
		address&= ~(1);
		resetTwiStatus();
		twiControlStatus()->slaw=true;
		break;
	case 'r':
		address|=1;
		resetTwiStatus();
		twiControlStatus()->slar=true;
		break;
	}
	TWDR=address;
	TWCR=1<<TWEN|1<<TWEA|1<<TWIE;
}

inline void twiData(uint8_t data){
	TWDR=data;
	TWCR=1<<TWEN|1<<TWEA|1<<TWIE;
	resetTwiStatus();
	twiControlStatus()->data=true;
}

void twiSendData(uint8_t* data, uint8_t size, bool dynamic,uint8_t address){
	queue(twiToSendQueue(),(void*)((UsartPackage){data,size,dynamic}),'t');
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
