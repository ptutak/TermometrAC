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


#ifndef _MY_TYPES_H_
#define _MY_TYPES_H_ 1

#include <stdlib.h>
#include <avr/io.h>
#include <stdbool.h>


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
	volatile uint8_t ttl;
	volatile uint8_t marker;
	volatile TwiControl control;
	void (*runFunc)(TwiPackage* self);
};

extern const TwiPackage NULL_TWI_PACKAGE;






typedef struct OsPackage OsPackage;

struct OsPackage{
	void (*runFunc)(OsPackage* self);
	const __memx void* data;
	uint8_t size;
	bool dynamic;
};





typedef union{
	OsPackage oPackage;
	UsartPackage uPackage;
	TwiPackage tPackage;
}Package;

extern const Package NULL_PACKAGE;




typedef struct CommNode CommNode;

struct CommNode{
	Package package;
	CommNode* volatile next;
};

typedef struct {
	CommNode* volatile head;
	CommNode* volatile tail;
	volatile bool isEmpty;
    volatile uint8_t counter;
}CommQueue;





typedef struct PriorityNode PriorityNode;

struct PriorityNode{
	Package package;
	uint16_t priority;
	PriorityNode* next;
};

typedef struct {
	PriorityNode* volatile head;
	PriorityNode* volatile tail;
	volatile bool isEmpty;
	volatile uint8_t counter;
}PriorityQueue;



#endif
