#ifndef _MY_USART_H_
#define _MY_USART_H_ 1

#include "my_types.h"
#include "my_queue.h"


CommQueue* usartToSendQueue(void);
CommQueue* usartReceivedQueue(void);


void usartSendText(const __memx char* text, uint8_t size, bool dynamic);
void usartSendData(const __memx uint8_t* data, uint8_t size, bool dynamic);


const char* usartGetText();
UsartPackage usartGetData(void);


void usartInit(uint16_t baud);


#endif /*__FILE____COUNTER__*/
