#ifndef _MY_TWI_H_
#define _MY_TWI_H_ 1

#include "my_types.h"
#include "my_usart.h"
#include "my_queue.h"

#define MAX_TWI_COUNT 10;


CommQueue* twiMasterQueue(void);


void twiSendData(uint8_t* data, uint8_t size, bool dynamic,uint8_t address);


void twiInit(uint32_t freq);



#endif /*__FILE__##__COUNTER__*/
