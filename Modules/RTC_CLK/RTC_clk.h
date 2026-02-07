#ifndef __RTC_CLK_H
#define __RTC_CLK_H
#include <stdint.h>

extern volatile uint16_t My_RTC_time[];

void RTC_clk_init(void);
void My_RTC_settime(void);

void My_RTC_readtime(void);

#endif //RTC_CLK_H









