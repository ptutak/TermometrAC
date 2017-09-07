#include "my_twi.h"


CommQueue* twiMasterQueue(void){
	static CommQueue twiMasterQueue={NULL,NULL,true,0};
	return &twiMasterQueue;
}






static inline void twiStart(bool twea){
	TWCR=1<<TWEN|1<<TWIE|1<<TWSTA|1<<TWEA;
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
	TWCR=1<<TWEN|1<<TWIE|twea<<TWEA;
}

static inline void twiData(uint8_t data, bool twea){
	TWDR=data;
	TWCR=1<<TWEN|1<<TWIE|twea<<TWEA;
}

static inline void twiStop(bool twea){
	TWCR=1<<TWEN|1<<TWIE|1<<TWSTO|twea<<TWEA;
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
		order->twiControlStatus.status=twiStatusReg;
		order->twiControlStatus.control=TWI_ERROR;
	}
}


static inline void twiSlawAction(TwiPackage* order, uint8_t twiStatusReg){
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
	case 0x38:
	case 0x40:
	case 0x48:
	case 0x50:
	case 0x58:
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
			twiData(*(order->data+marker),true);
			marker++;
		}
		break;
	case 0x30:
		order->twiControlStatus.control=TWI_REP_DATA;
		twiData(*(order->data+marker-1),true);
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
		dequeue(twiMasterQueue(),'t');
	}
	uint8_t* twiStaTemp=malloc(1);
	*twiStaTemp=twiStatusReg;
	usartSendData(twiStaTemp,1,true);
}



void twiSendData(uint8_t* data, uint8_t size, bool dynamic,uint8_t address){
	queue(twiMasterQueue(),(void*)&((TwiPackage){data,size,dynamic,{TWI_NULL,address,'W',0}}),'t');
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


