#include "os.h"


CommQueue* osStaticQueue(void){
	static CommQueue osStaticQueue={NULL,NULL,true,0};
	return &osStaticQueue;
}

CommQueue* osDynamicQueue(void){
	static CommQueue osDynamicQueue={NULL,NULL,true,0};
	return &osDynamicQueue;
}

CommQueue* osInitQueue(void){
	static CommQueue osInitQueue={NULL,NULL,true,0};
	return &osInitQueue;
}

PriorityQueue* osStaticPriorQueue(void){
	static PriorityQueue osPriorStaticQueue={NULL,NULL,true,0};
	return &osPriorStaticQueue;
}

PriorityQueue* osDynamicPriorQueue(void){
	static PriorityQueue osPriorDynamicQueue={NULL,NULL,true,0};
	return &osPriorDynamicQueue;
}

void addOsFunc(CommQueue* osQueue,void (*runFunc)(OsPackage* package),const __memx void* data, uint8_t size, bool dynamic){
	queue(osQueue,&(Package){.oPackage={runFunc,data,size,dynamic}});
}

void remOsFunc(CommQueue* osQueue,uint8_t index){
	OsPackage removed=remove(osQueue,index).oPackage;
	if (removed.dynamic)
		free((void*)removed.data);
}

void addOsPriorFunc(PriorityQueue* osQueue,void (*runFunc)(OsPackage* package),const __memx void* data, uint8_t size, bool dynamic, uint16_t priority){
	queuePrior(osQueue,&(Package){.oPackage={runFunc,data,size,dynamic}},priority);
}

void remOsPriorFunc(PriorityQueue* osQueue,uint16_t priority){
	OsPackage removed=removePrior(osQueue,priority).oPackage;
	if (removed.dynamic)
		free((void*)removed.data);
}

void manageOsDynamicQueue(CommQueue* osQueue){
	CommNode* order=osQueue->head;
	while (order){
		(*order->package.oPackage.runFunc)(&order->package.oPackage);
		order=order->next;
		OsPackage package=dequeue(osQueue).oPackage;
		if (package.dynamic)
			free((uint8_t*)package.data);
	}
}

void manageOsQueue(CommQueue* osQueue){
	CommNode* order=osQueue->head;
	while (order){
		(*order->package.oPackage.runFunc)(&order->package.oPackage);
		order=order->next;
	}
}

void manageOsDynamicPriorQueue(PriorityQueue* osQueue){
	PriorityNode* order=osQueue->head;
	while (order){
		(*order->package.oPackage.runFunc)(&order->package.oPackage);
		order=order->next;
		OsPackage package=dequeuePrior(osQueue).oPackage;
		if (package.dynamic)
			free((uint8_t*)package.data);
	}
}


void manageOsPriorQueue(PriorityQueue* osQueue){
	static uint16_t priority=0;
	PriorityNode* order=osQueue->head;
	while (order && order->priority==0)
		order=order->next;
	while (order){
		if (priority%(order->priority)==0)
			(*order->package.oPackage.runFunc)(&order->package.oPackage);
		order=order->next;
	}
	priority--;
}
