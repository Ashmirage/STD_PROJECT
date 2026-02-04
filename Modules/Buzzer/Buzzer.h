#ifndef __BUZZER_H
#define __BUZZER_H
#include <stdint.h>

void Buzzer_init(void);

void Buzzer_ON(void);
void Buzzer_OFF(void);
void Buzzer_alarm_start(void);

void Buzzer_alarm(uint16_t ms);



#endif


