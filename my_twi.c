#include "my_twi.h"


inline void resetTwiControlStatus(TwiControlStatus* status){
	status->control=TWI_NULL;
}


