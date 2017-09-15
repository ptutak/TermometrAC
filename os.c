#include "os.h"


CommQueue* osQueue(void){
	static CommQueue osQueue={NULL,NULL,true,0};
	return &osQueue;
}

void addOsFunc(void (*runFunc)(OsPackage* package),const __memx void* data, uint8_t size, bool dynamic){
	queue(osQueue(),&(Package){.oPackage={runFunc,data,size,dynamic}});
}

void remOsFunc(uint8_t index){
	OsPackage removed=remove(osQueue(),index).oPackage;
	if (removed.dynamic)
		free(removed.data);
}
void manageOsQueue(void){
	CommNode* order=osQueue()->head;
	while (order){
		(*order->package.oPackage.runFunc)(&order->package.oPackage);
	}
}
