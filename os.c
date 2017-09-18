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

void addOsFunc(CommQueue* osQueue,void (*runFunc)(OsPackage* package),const __memx void* data, uint8_t size, bool dynamic){
	queue(osQueue,&(Package){.oPackage={runFunc,data,size,dynamic}});
}

void remOsFunc(CommQueue* osQueue,uint8_t index){
	OsPackage removed=remove(osQueue,index).oPackage;
	if (removed.dynamic)
		free((void*)removed.data);
}
void manageOsQueue(CommQueue* osQueue,bool dynamic){
	CommNode* order=osQueue->head;
	while (order){
		(*order->package.oPackage.runFunc)(&order->package.oPackage);
		order=order->next;
		if (dynamic){
			OsPackage package=dequeue(osQueue).oPackage;
			if (package.dynamic)
				free((uint8_t*)package.data);
		}
	}
}
