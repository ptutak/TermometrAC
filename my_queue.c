#include "my_queue.h"



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

void insert(CommQueue* queue, void* package, char type, uint16_t index){
	if (queue==NULL || index>queue->counter)
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
		if (index==0){
			if (queue->head!=NULL)
				tmpNode=queue->head->next;
			else{
				queue->tail=tmpNode;
				queue->isEmpty=false;
			}
			queue->head=tmpNode;
		}
		else {
			CommNode* prev=queue->head;
			index--;
			while (index){
				index--;
				prev=prev->next;
			}
			tmpNode->next=prev->next;
			prev->next=tmpNode;
			if(prev==queue->tail)
				queue->tail=tmpNode;
		}
		queue->counter++;
	}
}

