#include "my_twi.h"



const uint32_t TWI_FREQ=100000;
const uint8_t TWI_STD_TTL=20;




CommQueue* twiMasterQueue(void){
	static CommQueue twiMasterQueue={NULL,NULL,true,0};
	return &twiMasterQueue;
}

bool twiEnabled(void){
	return (bool)TWCR&1<<TWEN;
}




static inline void twiStart(bool twea){
	TWCR=1<<TWEN|1<<TWINT|1<<TWIE|1<<TWSTA|1<<TWEA;
}

static inline void twiAddress(uint8_t address, char mode, bool twea){
	switch (mode){
	case 'w':
	case 'W':
		usartSafeTransmit('W');
		address&= 0b11111110;
		break;
	case 'r':
	case 'R':
		usartSafeTransmit('R');
		address|=1;
		break;
	}
	usartSafeTransmit(address);
	usartSafeTransmit('\n');
	TWDR=address;
	TWCR=1<<TWEN|1<<TWINT|1<<TWIE|twea<<TWEA;
}

static inline void twiDataSend(uint8_t data, bool twea){
	usartSafeTransmit('d');
	usartSafeTransmit('\n');
	TWDR=data;
	TWCR=1<<TWEN|1<<TWINT|1<<TWIE|twea<<TWEA;
	usartSafeTransmit('d');
	usartSafeTransmit('d');
	usartSafeTransmit('\n');
}

static inline uint8_t twiDataReceive(bool twea){
	uint8_t data=TWDR;
	TWCR=1<<TWEN|1<<TWINT|1<<TWIE|twea<<TWEA;
	return data;
}

static inline void twiStop(bool twea){
	TWCR=1<<TWEN|1<<TWINT|1<<TWIE|1<<TWSTO|twea<<TWEA;
}
static inline void twiClearInt(bool twea){
	TWCR=1<<TWEN|1<<TWINT|1<<TWIE|twea<<TWEA;
}
/*
static inline void twiAck(){
	TWCR=1<<TWEN|1<<TWINT|1<<TWIE|1<<TWEA;
}
static inline void twiNotAck(){
	TWCR=1<<TWEN|1<<TWINT|1<<TWIE;
}
static inline void twiStopStart(bool twea){
	TWCR=1<<TWEN|1<<TWIE|1<<TWSTO|1<<TWSTA|twea<<TWEA;
}
*/






static inline void twiStartAction(TwiPackage* order,uint8_t twiStatusReg){
	usartSafeTransmit('t');
	usartSafeTransmit('\n');
	switch(twiStatusReg){
	case 0x08:
	case 0x10:
		switch(order->mode){
		case 'W':
			order->control=TWI_SLAW;
			twiAddress(order->address,'W',true);
			break;
		case 'R':
			order->control=TWI_SLAR;
			twiAddress(order->address,'R',true);
			break;
		}
		break;
	default:
		order->control=TWI_ERROR;
	}
}


static inline void twiSlawAction(TwiPackage* order, uint8_t twiStatusReg){
	usartSafeTransmit('w');
	usartSafeTransmit('\n');
	switch(twiStatusReg){
	case 0x18:
		if (order->size>0){
			order->control=TWI_DATA;
			order->ttl=TWI_STD_TTL;
			twiDataSend(*(order->data),true);
		}
		else {
			order->control=TWI_STOP;
			twiStop(true);
		}
		break;
	case 0x20:
		order->control=TWI_REP_START;
		twiStart(true);
		break;
	default:

		order->control=TWI_ERROR;
	}
}


static inline void twiSlarAction(TwiPackage* order, uint8_t twiStatusReg){
	usartSafeTransmit('r');
	usartSafeTransmit('\n');
	switch (twiStatusReg){
	case 0x40:
		if (order->size>0){
			order->control=TWI_DATA;
			if (order->size==1)
				*((uint8_t*)order->data)=twiDataReceive(false);
			else
				*((uint8_t*)order->data)=twiDataReceive(true);
			order->marker++;
		}
		break;
	case 0x48:
		order->control=TWI_REP_START;
		twiStart(true);
		break;
	case 0x38:
	default:
		order->control=TWI_ERROR;
	}

}

static inline void twiDataAction(TwiPackage* order, uint8_t twiStatusReg){
	usartSafeTransmit('d');
	usartSafeTransmit('\n');
	switch(twiStatusReg){
	case 0x28:
		if (order->size==order->marker){
			order->control=TWI_STOP;
			twiStop(true);
		}
		else {
			twiDataSend(*(order->data+order->marker),false);
			order->marker++;
		}
		break;
	case 0x30:
		order->control=TWI_REP_DATA;
		twiDataSend(*(order->data+order->marker-1),true);
		break;
	case 0x50:
		if (order->size+1==order->marker)
			*((uint8_t*)order->data+order->marker)=twiDataReceive(false);
		else
			*((uint8_t*)order->data+order->marker)=twiDataReceive(true);
		order->marker++;
		break;
	case 0x58:
		twiStop(true);
		break;
	case 0x38:
	default:
		order->control=TWI_ERROR;
	}
}



ISR(TWI_vect){
	static uint8_t counter=0;
	usartSafeTransmit('c');
	usartSafeTransmit(counter);
	usartSafeTransmit('\n');
	counter++;
	TwiPackage* order=NULL;
	uint8_t twiStatusReg=TWSR & (0b11111000);
	TwiPackage orderToRemove=NULL_TWI_PACKAGE;
	if (!(TWCR&1<<TWEN)){
		return;
	}
	if (twiMasterQueue()->isEmpty){
		twiClearInt(false);
		return;
	}
	order=&(twiMasterQueue()->head->tPackage);
	if (order->ttl==0){
		order->control=TWI_STOP;
		twiStop(true);
	}
	usartSafeTransmit(order->ttl);
	usartSafeTransmit('\n');
	usartSafeTransmit(order->control);
	usartSafeTransmit('\n');
	usartSafeTransmit(order->size);
	usartSafeTransmit('\n');
	usartSafeTransmit(order->marker);
	usartSafeTransmit('\n');

	switch(order->control){
	case TWI_NULL:
		order->control=TWI_START;
		twiStart(true);
		break;
	case TWI_START:
		twiStartAction(order,twiStatusReg);
		break;
	case TWI_REP_START:
		twiStartAction(order,twiStatusReg);
		order->ttl--;
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
	case TWI_REP_DATA:
		twiDataAction(order,twiStatusReg);
		order->ttl--;
		break;
	case TWI_ERROR:
		order->ttl=0;
		break;
	default:
		break;
	}
	if (order->control==TWI_STOP)
		orderToRemove=dequeue(twiMasterQueue(),'t').tPackage;
	usartSafeTransmit('k');
	usartSafeTransmit('\n');

	if (orderToRemove.runFunc){
		(*orderToRemove.runFunc)(&orderToRemove);
		if (!twiMasterQueue()->isEmpty)
			TWI_vect();
	}

}



void twiInit(uint32_t freq, bool twea){
	TWCR=1<<TWEN|1<<TWIE|twea<<TWEA;
	uint32_t twbr=(F_CPU/freq - 16)/2;
	uint8_t prescaler=0;
	while (twbr>255){
		twbr/=4;
		prescaler++;
	}
	TWSR&=(0b11111000);
	TWSR|=prescaler;
	TWBR=(uint8_t)twbr;
}

void twiOff(){
	TWBR=0;
	TWSR=0;
	TWCR=0;
}

void twiSendMasterData(const __memx uint8_t* data, uint8_t size, uint8_t address, void (*callFunc)(TwiPackage* self)){
	queue(twiMasterQueue(),(void*)&((TwiPackage){data,size,address,'W',TWI_STD_TTL,0,TWI_NULL,callFunc}),'t');
	TWI_vect();
}
void twiSendMasterDataNoInterrupt(const __memx uint8_t* data, uint8_t size, uint8_t address, void (*callFunc)(TwiPackage* self)){
	queue(twiMasterQueue(),(void*)&((TwiPackage){data,size,address,'W',TWI_STD_TTL,0,TWI_NULL,callFunc}),'t');
}

void twiReadMasterData(uint8_t* data, uint8_t size, uint8_t address, void(*callFunc)(TwiPackage* self)){
	queue(twiMasterQueue(),(void*)&((TwiPackage){data,size,address,'R',TWI_STD_TTL,0,TWI_NULL,callFunc}),'t');
	TWI_vect();
}
void twiReadMasterDataNoInterrupt(uint8_t* data, uint8_t size, uint8_t address, void(*callFunc)(TwiPackage* self)){
	queue(twiMasterQueue(),(void*)&((TwiPackage){data,size,address,'R',TWI_STD_TTL,0,TWI_NULL,callFunc}),'t');
}

void twiManageOrders(){
	TWI_vect();
}
