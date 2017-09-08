#include "my_twi.h"


CommQueue* twiMasterQueue(void){
	static CommQueue twiMasterQueue={NULL,NULL,true,0};
	return &twiMasterQueue;
}






static inline void twiStart(bool twea){
	TWCR=1<<TWEN|1<<TWINT|1<<TWIE|1<<TWSTA|1<<TWEA;
}

static inline void twiAddress(uint8_t address, char mode, bool twea){
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
	TWCR=1<<TWEN|1<<TWINT|1<<TWIE|twea<<TWEA;
}

static inline void twiDataSend(uint8_t data, bool twea){
	TWDR=data;
	TWCR=1<<TWEN|1<<TWINT|1<<TWIE|twea<<TWEA;
}

static inline uint8_t twiDataReceive(bool twea){
	uint8_t data=TWDR;
	TWCR=1<<TWEN|1<<TWINT|1<<TWIE|twea<<TWEA;
	return data;
}

static inline void twiStop(){
	TWCR=1<<TWEN|1<<TWINT|1<<TWIE|1<<TWSTO;
}
/*
static inline void twiAck(){
	TWCR=1<<TWEN|1<<TWINT|1<<TWIE|1<<TWEA;
}
static inline void twiNotAck(){
	TWCR=1<<TWEN|1<<TWINT|1<<TWIE;
}

/*
static inline void twiStopStart(bool twea){
	TWCR=1<<TWEN|1<<TWIE|1<<TWSTO|1<<TWSTA|twea<<TWEA;
}
*/






static inline void twiStartAction(TwiPackage* order,uint8_t twiStatusReg){
	switch(twiStatusReg){
	case 0x08:

	case 0x10:
		switch(order->mode){
		case 'W':
			order->twiControlStatus.control=TWI_SLAW;
			twiAddress(order->address,'W',true);
			break;
		case 'R':
			order->twiControlStatus.control=TWI_SLAR;
			twiAddress(order->address,'R',true);
			break;
		}
		break;
	default:
		order->twiControlStatus.status=twiStatusReg;
		order->twiControlStatus.control=TWI_ERROR;
	}
}


static inline void twiSlawAction(TwiPackage* order, uint8_t twiStatusReg){
	switch(twiStatusReg){
	case 0x18:
		if (order->size>0){
			order->twiControlStatus.control=TWI_DATA;
			twiDataSend(*(order->data),true);
		}
		else {
			order->twiControlStatus.control=TWI_STOP;
			twiStop(true);
		}
		break;
	case 0x20:
		order->twiControlStatus.control=TWI_REP_START;
		twiStart(true);
		break;
	default:
		order->twiControlStatus.status=twiStatusReg;
		order->twiControlStatus.control=TWI_ERROR;
	}
}


static inline void twiSlarAction(TwiPackage* order, uint8_t twiStatusReg){
	switch (twiStatusReg){
	case 0x40:
		if (order->size>0){
			order->twiControlStatus.control=TWI_DATA;
			if (order->size==1)
				*((uint8_t*)order->data)=twiDataReceive(false);
			else
				*((uint8_t*)order->data)=twiDataReceive(true);
		}
		break;
	case 0x48:
		order->twiControlStatus.control=TWI_REP_START;
		twiStart(true);
		break;
	case 0x50:

	case 0x58:
	case 0x38:
	default:
		order->twiControlStatus.status=twiStatusReg;
		order->twiControlStatus.control=TWI_ERROR;
	}

}

static inline void twiDataAction(TwiPackage* order, uint8_t twiStatusReg){
	static uint8_t marker=1;
	switch(twiStatusReg){
	case 0x28:
		if (order->size==marker){
			order->twiControlStatus.control=TWI_STOP;
			marker=1;
			twiStop(true);
		}
		else {
			twiDataSend(*(order->data+marker),true);
			marker++;
		}
		break;
	case 0x30:
		order->twiControlStatus.control=TWI_REP_DATA;
		twiDataSend(*(order->data+marker-1),true);
		break;
	case 0x38:
	default:
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
	case TWI_REP_START:
		if (counter==10){
			order->twiControlStatus.control=TWI_STOP;
			counter=0;
			twiStop(true);
		}
		else {
			twiStartAction(order,twiStatusReg);
			counter++;
		}
		break;
	case TWI_SLAW:
		twiSlawAction(order,twiStatusReg);
		break;
	case TWI_SLAR:
		twiSlarAction(order,twiStatusReg);
		break;
	case TWI_DATA:
		counter=0;
		twiDataAction(order,twiStatusReg);
		break;
	case TWI_REP_DATA:
		if (counter==10){
			order->twiControlStatus.control=TWI_STOP;
			counter=0;
			twiStop(true);
		} else{
			counter++;
			twiDataAction(order,twiStatusReg);
		}
		break;
	default:;
	}
	if (order->twiControlStatus.control==TWI_STOP){
		orderDone=true;
		TwiPackage orderToRemove=dequeue(twiMasterQueue(),'t').tPackage;
		if (orderToRemove.runFunc)
			(*orderToRemove.runFunc)(&orderToRemove);
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
	TWSR&=0;
	TWSR|=prescaler;
	TWBR=twbr;
}

void twiOff(){
	TWBR=0;
	TWSR=0;
	TWCR=0;
}

void twiSendData(const __memx uint8_t* data, uint8_t size, uint8_t address, void (*callFunc)(TwiPackage* self)){
	queue(twiMasterQueue(),(void*)&((TwiPackage){data,size,address,'W',{TWI_NULL,0},callFunc}),'t');
}

