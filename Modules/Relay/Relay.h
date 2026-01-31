#ifndef __RELAY_H
#define __RELAY_H		
#include "stm32f10x.h"
void Relay_init(void);

void Relay_on(void);
void Relay_status(uint8_t status);

void Relay_off(void);
#endif  
	 
	 



