#ifndef _MY_TYPES_H_
#define _MY_TYPES_H_ 1

#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include <stdlib.h>


typedef struct{
	uint8_t low;
	uint8_t high;
}uint16_t_split;



typedef struct {
	const __memx uint8_t* data;
	uint8_t size;
	bool dynamic;
}UsartPackage;





typedef enum {
	TWI_NULL=0,
	TWI_START,
	TWI_REP_START,
	TWI_SLAW,
	TWI_SLAR,
	TWI_DATA,
	TWI_REP_DATA,
	TWI_STOP,
	TWI_ERROR
}TwiControl;

typedef struct TwiPackage TwiPackage;

struct TwiPackage{
	const __memx uint8_t* data;
	uint8_t size;
	uint8_t address;
	char mode;
	volatile TwiControl control;
	void (*runFunc)(TwiPackage* self);
};

extern const TwiPackage NULL_TWI_PACKAGE;

void freeData(TwiPackage* package);





typedef union{
	UsartPackage uPackage;
	TwiPackage tPackage;
}Package;

extern const Package NULL_PACKAGE;




typedef struct CommNode CommNode;

struct CommNode{
	CommNode* volatile next;
	union{
			UsartPackage uPackage;
			TwiPackage tPackage;
	};
};

typedef struct {
	CommNode* volatile head;
	CommNode* volatile tail;
	volatile bool isEmpty;
    volatile uint16_t counter;
}CommQueue;





#endif
