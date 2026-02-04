#ifndef __LED_H
#define __LED_H
#include <stdint.h>

typedef enum
{
	LED_STATUS_ON = 1,
	LED_STATUS_OFF = 0,
}LED_status;


void LED_init(void);

void LED_ON(void);

void LED_OFF(void);

void LED_Toggle(void);

void LED_tick(void);

// 开启LED闪烁
void LED_flash_on(uint16_t ms);

void LED_flash_off(void);

LED_status LED_get_status(void);



#endif


