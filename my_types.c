#include "my_types.h"

const TwiControlStatus NULL_TWI_CONTROL_STATUS={TWI_NULL,0,'\0',0};

const TwiControlStatus TWI_MASTER_TO_SEND_STATUS={TWI_NULL,0,'W',0};

const TwiControlStatus TWI_MASTER_RECEIVE_STATUS={TWI_NULL,0,'R',0};

const Package NULL_PACKAGE={.tPackage={NULL,0,false,{TWI_NULL,0,'\0',0}}};
