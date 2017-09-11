#include "my_types.h"

const Package NULL_PACKAGE={.tPackage={NULL,0,0,'\0',TWI_NULL,NULL}};

const TwiPackage NULL_TWI_PACKAGE={NULL,0,0,'\0',TWI_NULL,NULL};


void freeData(TwiPackage* package){
	free((uint8_t*)package->data);
}
