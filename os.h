#ifndef _OS_H_
#define _OS_H_
#include "my_types.h"
#include "my_queue.h"


CommQueue* osQueue(void){
	static CommQueue osQueue={NULL,NULL,true,0};
	return &osQueue;
}

void manageOsQueue(void){
	CommNode* order=osQueue()->head;
	while (order){
		(*order->package.oPackage.runFunc)(&order->package.oPackage);
	}
}

void addOsFunc(void (*runFunc)(OsPackage* package),const __memx void* data, uint8_t size){
	queue(osQueue(),&(Package){.oPackage={runFunc,data,size}});
}

void remOsFunc(void){

}

#endif
