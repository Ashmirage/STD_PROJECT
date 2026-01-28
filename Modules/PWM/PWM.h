#ifndef __PWM_H
#define __PWM_H
#include <stdint.h>

void PWM_init(void);

/**
 * @brief 设置PWM占空比
 *
 * @param[in]  pwm 0-100
 *
 * @return None
 *
 */
void PWM_set(uint8_t pwm);









#endif


