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

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

#include "my_types.h"
#include "my_queue.h"
#include "my_twi.h"
#include "my_usart.h"
#include "lcd_control.h"
#include "termometr.h"
#include "os.h"


static const uint16_t BAUD=9600;

LCD lcd;




void resendUsartMsg(OsPackage* package){
    static const char* received;
    static uint8_t size=0;

    if(!usartReceivedQueue()->isEmpty){
        received=usartGetText();
        while(*(received+size))
            size++;
        usartSendText(received,size,true);
        size=0;
    }
}

void lcdSetTemperature(OsPackage* lcdPackage){
	static char tempLow[5];
	static char ledTempStr[10];
	static char usartTempStr[10];
	static char degree[2]={0xDF,0x00};

	int temp=getTemperature(PC1,PC0,2000);

	itoa(temp/10,ledTempStr,10);
	itoa(temp%10,tempLow,10);

	strcat(ledTempStr,",");
	strcat(ledTempStr,tempLow);
	strcat(ledTempStr," ");

	strcpy(usartTempStr,ledTempStr);

	strcat(ledTempStr,degree);
	strcat(ledTempStr,"C");

	strcat(usartTempStr,"C\n");

	usartSendText(usartTempStr,strlen(usartTempStr),false);

	lcdClear(&lcd);
    lcdHome(&lcd);
	lcdSendText(&lcd,ledTempStr,strlen(ledTempStr),false);
}



void initSystem(OsPackage* package){


	usartInit(BAUD);
    addOsPriorFunc(osStaticPriorQueue(),usartManageToSendQueue,NULL,0,false,1001);

    twiInit(TWI_FREQ,true);
	addOsPriorFunc(osStaticPriorQueue(),twiManageMasterQueue,NULL,0,false,1000);


    lcd.address=0x40;
    lcd.configInitArray=LCD_CONFIG_INIT_2X16S;
    lcd.configInitArraySize=LCD_CONFIG_INIT_2X16S_SIZE;
    lcd.sendSplit=splitDataPCF8574_DataHigh;
    lcd.receivedMerge=receivedDataPCF8574_DataHigh;
    lcd.backlight=BACKLIGHT_ON;


    lcdInit(&lcd);



    usartSendText(PSTR("Copyright PT\n"),sizeof("Copyright PT\n")-1,false);
    usartManageToSendQueue(NULL);

    lcdSendText(&lcd,PSTR("Termometr!"),sizeof("Termometr!")-1,false);
    lcdGoTo(&lcd,0,1);
    lcdSendText(&lcd,PSTR("Copyright PT"),sizeof("Copyright PT")-1,false);
    twiManageMasterQueue(NULL);

    addOsPriorFunc(osStaticPriorQueue(),lcdSetTemperature,NULL,0,false,0xffff/4);

    _delay_ms(4000);
}



int main(void){

	addOsFunc(osInitQueue(),initSystem,NULL,0,false);

	manageOsDynamicQueue(osInitQueue());

	while(1){
    	manageOsDynamicQueue(osDynamicQueue());
    	manageOsQueue(osStaticQueue());
    	manageOsDynamicPriorQueue(osDynamicPriorQueue());
    	manageOsPriorQueue(osStaticPriorQueue());
    }

}
