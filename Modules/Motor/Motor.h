#ifndef __MOTOR_H
#define __MOTOR_H
#include <stdint.h>

/**
* @brief  直流电机初始化
 *
 * @param[in]  None
 *
 * @return None
 *
 */
void Motor_init(void);


/**
* @brief  设置电机驱动力度 -100--+100
 *
 * @param[in]  speed -100-+100
 *
 * @return None
 *
 */
void Motor_set_speed(int8_t speed);








#endif


