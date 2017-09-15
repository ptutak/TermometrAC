#include "my_queue.h"



void queue(CommQueue* queue, Package* package){
    if (queue==NULL)
        return;
	CommNode* tmpNode=malloc(sizeof(CommNode));
	tmpNode->next=NULL;
	tmpNode->package=*package;
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




Package dequeue(CommQueue* queue){
    if (queue==NULL || queue->head==NULL)
    	return NULL_PACKAGE;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    	CommNode* tmpNode=queue->head;
    	queue->head=queue->head->next;

    	if (queue->head==NULL){
    		queue->tail=NULL;
    		queue->isEmpty=true;
    	}
    	Package retPackage=tmpNode->package;
    	free(tmpNode);
    	queue->counter--;
    	return retPackage;
    }
    return NULL_PACKAGE;
}

void insert(CommQueue* queue, Package* package, uint8_t index){
	if (queue==NULL || index>queue->counter)
		return;
	CommNode* tmpNode=malloc(sizeof(CommNode));
	tmpNode->next=NULL;
	tmpNode->package=*package;
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

Package remove(CommQueue* queue, uint8_t index){
	if (queue==NULL || index>=queue->counter)
		return NULL_PACKAGE;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		CommNode* tmpNode=queue->head;
		if (index==0){
			queue->head=queue->head->next;
			if (queue->head==NULL){
				queue->tail=NULL;
				queue->isEmpty=true;
			}
		}
		else{
			CommNode* prev=queue->head;
			index--;
			while(index){
				prev=prev->next;
				index--;
			}
			tmpNode=prev->next;
			prev->next=prev->next->next;
			if (prev->next==NULL)
				queue->tail=prev;
		}
		Package retPackage=tmpNode->package;
		queue->counter--;
		free(tmpNode);
		return retPackage;
	}
	return NULL_PACKAGE;
}
