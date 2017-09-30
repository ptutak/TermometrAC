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


#include "usart.h"

static inline void usartTransmit(uint8_t data) {
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}

static inline uint8_t usartReceive(void){
	while (!(UCSR0A & (1<<RXC0))){};
	return UDR0;

}

void usartSafeTransmit(uint8_t data) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		uint8_t ucsr0bBackup=UCSR0B&(1<<TXCIE0);
		UCSR0B&=~(1<<TXCIE0);
		while (!(UCSR0A & (1<<UDRE0)));
		UDR0 = data;
		while (!(UCSR0A & (1<<TXC0)));
		UCSR0A|=1<<TXC0;
		UCSR0B|=ucsr0bBackup;
	}
}



CommQueue* usartToSendQueue(void){
	static CommQueue usartToSendQueue={NULL,NULL,true,0};
	return &usartToSendQueue;
}

CommQueue* usartReceivedQueue(void){
	static CommQueue usartReceivedQueue={NULL,NULL,true,0};
	return &usartReceivedQueue;
}

static volatile bool CompletedSendTransmission=true;

ISR(USART_TX_vect){
	static uint16_t marker=0;
	if (!usartToSendQueue()->isEmpty){
		UsartPackage toSend=usartToSendQueue()->head->package.uPackage;
		if (!CompletedSendTransmission){
			marker++;
			if (marker==toSend.size){
				 CompletedSendTransmission=true;
				 toSend=(dequeue(usartToSendQueue())).uPackage;
				 if (toSend.dynamic)
					free((uint8_t*)toSend.data);
			 }
			 else
			   usartTransmit(*(toSend.data+marker));
		}
		else{
			CompletedSendTransmission=false;
			marker=0;
			if (toSend.size>0)
				usartTransmit(*toSend.data);
			else{
				toSend=(dequeue(usartToSendQueue())).uPackage;
				if (toSend.dynamic)
					free((uint8_t*)toSend.data);
			}
		}
	}
}

ISR(USART_RX_vect){
	static bool completedTransmission=true;
	static uint16_t marker;
	static UsartPackage received;
    received.dynamic=true;
	if (!completedTransmission){
    	*((uint8_t*)received.data+marker)=usartReceive();
        marker++;
		if(marker>=received.size){
			completedTransmission=true;
            queue(usartReceivedQueue(),&(Package){.uPackage=received});
        }
	}
	else {
        marker=0;
        completedTransmission=false;
        received.size=usartReceive();
        if (received.size==0)
            completedTransmission=true;
        else
            received.data=malloc(received.size);
	}
}

void usartSendInterrupt(OsPackage* notUsed){
	if (!usartToSendQueue()->isEmpty && CompletedSendTransmission)
		USART_TX_vect();
}

void usartSendText(const __memx char* text, uint8_t size, bool dynamic){
	queue(usartToSendQueue(),&(Package){.uPackage={(const __memx uint8_t*)text,size,dynamic}});
}

void usartSendData(const __memx uint8_t* data, uint8_t size, bool dynamic){
	queue(usartToSendQueue(),&(Package){.uPackage={data,size,dynamic}});
}

const char* usartGetText(){
	return (const char*)(dequeue(usartReceivedQueue())).uPackage.data;
}

UsartPackage usartGetData(void){
	return dequeue(usartReceivedQueue()).uPackage;
}

void usartManageToSendQueue(OsPackage* notUsed){
	while(!usartToSendQueue()->isEmpty){
		if (CompletedSendTransmission){
			USART_TX_vect();
		}
	}
}

void usartInit(uint16_t baud){
	uint16_t ubrr=F_CPU/16/baud-1;
	UBRR0H = (uint8_t)(ubrr>>8);
	UBRR0L = (uint8_t)ubrr;

	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<TXCIE0)|(1<<RXCIE0);

	UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);
}
