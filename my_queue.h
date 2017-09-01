#ifndef __FILE__##__COUNTER__
#define __FILE__##__COUNTER__
#include "my_twi.h"
#include "my_usart.h"

typedef union{
	UsartPackage uPackage;
	TwiPackage tPackage;
}Package;

stati const Package NULL_PACKAGE={.tPackage={NULL,0,false,{TWI_NULL,0,'\0',0}}};

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

CommQueue* twiMasterQueue(void){
	static CommQueue twiMasterQueue={NULL,NULL,true,0};
	return &twiMasterQueue;
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



#endif /*__FILE__##__COUNTER__*/
