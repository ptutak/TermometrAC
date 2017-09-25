#ifndef _LCD_CONTROL_H_
#define _LCD_CONTROL_H_

#include "my_types.h"
#include "my_twi.h"
#include "os.h"

//byte structure: 0b d7,d6,d5,d4,bt,e,rw,rs
//byte structure: 0b d3,d2,d1,d0,bt,e,rw,rs

//byte structure:0b 0,0,0,0,bt,e,rw,rs
typedef enum{
	LCD_COMMAND=0x00,
	LCD_WRITE_CG_OR_DDR=0x01,
	LCD_READ_BS_FLAG_AND_ADDR=0x02,
	LCD_READ_CG_OR_DDR=0x03,
	ENABLE=0x04,
	BACKLIGHT_ON=0x08,
	BACKLIGHT_OFF=0x00,
}LCDInstructionType;

//byte structure 0b d7,d6,d5,d4,d3,d2,d1,d0
typedef enum{
	LCD_NULL=0x00,

	LCD_CLEAR_DISPLAY=0x01,
	LCD_RETURN_HOME=0x02,

	LCD_SET_DECREMENT=0x04,
	LCD_SET_DECREMENT_SHIFT=0x05,

	LCD_SET_INCREMENT=0x06,
	LCD_SET_INCREMENT_SHIFT=0x07,

	LCD_SET_DISPLAY=0x08,
	BLINKING_ON=0x01,
	CURSOR_ON=0x02,
	DISPLAY_ON=0x04,
	DISPLAY_OFF=0x00,

	LCD_SHIFT_CURSOR_LEFT=0x10,
	LCD_SHIFT_CURSOR_RIGHT=0x14,
	LCD_SHIFT_DISPLAY_LEFT=0x18,
	LCD_SHIFT_DISPLAY_RIGHT=0x1C,

	LCD_F_SET_4_BIT_1_LINE_8_FONT=0x20,
	LCD_F_SET_4_BIT_1_LINE_10_FONT=0x24,
	LCD_F_SET_4_BIT_2_LINE_8_FONT=0x28,
	LCD_F_SET_8_BIT_1_LINE_8_FONT=0x30,
	LCD_F_SET_8_BIT_1_LINE_10_FONT=0x34,
	LCD_F_SET_8_BIT_2_LINE_8_FONT=0x38,

	LCD_GO_TO_CGR_ADDR=0x40,
	LCD_GO_TO_DDR_ADDR=0x80,

	LCD_FULL=0xFF

}LCDCommand;



typedef struct{
	uint8_t address;
	const __memx LCDCommand* configInitArray;
	uint8_t configInitArraySize;
	uint16_t (*sendSplit) (uint8_t instruction, uint8_t data);
	uint8_t (*receivedMerge)(uint8_t high, uint8_t low);
	LCDInstructionType backlight;
}LCD;



extern LCDCommand LCD_CONFIG_INIT_2X16S[];
extern uint8_t LCD_CONFIG_INIT_2X16S_SIZE;


void lcdInit(LCD* lcd);
void lcdSendText(LCD* lcd,const __memx char* tekst,uint8_t size,bool dynamic);
void lcdBacklightToggle(LCD* lcd);
void lcdGoTo(LCD* lcd, uint8_t x, uint8_t y);
void lcdClear(LCD* lcd);
void lcdHome(LCD* lcd);
uint8_t* lcdReadBsFlagAndAddr(LCD* lcd, uint8_t* bsAndAddr, void (* run)(TwiPackage* package));


uint16_t splitDataPCF8574_DataHigh(uint8_t instructionType, uint8_t data);
uint8_t receivedDataPCF8574_DataHigh(uint8_t high, uint8_t low);



#endif
