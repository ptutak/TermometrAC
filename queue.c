/*
Copyright 2017 Piotr Tutak

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/


#include "queue.h"



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

Package removePrior(PriorityQueue* queue, uint16_t priority){
	if (queue==NULL || queue->tail==NULL || priority>queue->tail->priority)
		return NULL_PACKAGE;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		PriorityNode* tmpNode=queue->head;
		if (priority<=queue->head->priority){
			queue->head=queue->head->next;
			if (queue->head==NULL){
				queue->tail=NULL;
				queue->isEmpty=true;
			}
		}
		else{
			PriorityNode* prev=queue->head;
			while(prev->next && prev->next->priority<priority)
					prev=prev->next;
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



void queuePrior(PriorityQueue* queue, Package* package, uint16_t priority){
    if (queue==NULL)
        return;
	PriorityNode* tmpNode=malloc(sizeof(PriorityNode));
	tmpNode->next=NULL;
	tmpNode->priority=priority;
	tmpNode->package=*package;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		if (queue->head==NULL){
			queue->head=tmpNode;
			queue->tail=tmpNode;
			queue->isEmpty=false;
		}
		else{
			if (queue->head->priority>priority){
				tmpNode->next=queue->head;
				queue->head=tmpNode;
			}
			else {
				PriorityNode* prev=queue->head;
				while(prev->next!=NULL && prev->next->priority<=priority)
					prev=prev->next;
				tmpNode->next=prev->next;
				prev->next=tmpNode;
				if (prev==queue->tail)
					queue->tail=tmpNode;
			}
		}
		queue->counter++;
	}
}




Package dequeuePrior(PriorityQueue* queue){
    if (queue==NULL || queue->head==NULL)
    	return NULL_PACKAGE;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    	PriorityNode* tmpNode=queue->head;
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

