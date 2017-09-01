#ifndef __FILE__##__COUNTER__
#define __FILE__##__COUNTER__

static const uint8_t MAX_TWI_COUNT=10;


typedef enum {
	TWI_NULL=0,
	TWI_START,
	TWI_REP_START,
	TWI_SLAW,
	TWI_SLAR,
	TWI_DATA,
	TWI_STOP,
	TWI_ERROR
}TwiControl;

typedef struct{
	volatile TwiControl control;
	volatile uint8_t address;
	volatile char mode;
	volatile uint8_t status;
} TwiControlStatus;

const TwiControlStatus NULL_TWI_CONTROL_STATUS={TWI_NULL,0,'\0',0};

const TwiControlStatus TWI_MASTER_TO_SEND_STATUS={TWI_NULL,0,'W',0};

const TwiControlStatus TWI_MASTER_RECEIVE_STATUS={TWI_NULL,0,'R',0};



typedef struct{
	const __memx uint8_t* data;
	uint8_t size;
	bool dynamic;
	TwiControlStatus twiControlStatus;
} TwiPackage;


#endif /*__FILE__##__COUNTER__*/
