#ifndef _MY_TWI_H_
#define _MY_TWI_H_

#include <util/twi.h>
#include "my_types.h"
#include "my_usart.h"
#include "my_queue.h"


#define MAX_TWI_COUNT 10;


CommQueue* twiMasterQueue(void);



void twiSendMasterData(const __memx uint8_t* data, uint8_t size,uint8_t address, void (*callFunc)(TwiPackage* self));
void twiSendMasterDataNoInterrupt(const __memx uint8_t* data, uint8_t size,uint8_t address, void (*callFunc)(TwiPackage* self));


void twiReadMasterData(uint8_t* data, uint8_t size, uint8_t address, void (*callFunc)(TwiPackage* self));
void twiReadMasterDataNoInterrupt(uint8_t* data, uint8_t size, uint8_t address, void (*callFunc)(TwiPackage* self));

void twiManageOrders();

void twiInit(uint32_t freq,bool twea);



#endif /*__FILE__##__COUNTER__*/
