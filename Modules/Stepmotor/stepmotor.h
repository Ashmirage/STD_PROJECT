#ifndef __STEPMOTOR_H
#define	__STEPMOTOR_H
#include "stm32f10x.h"

void STEPMOTOR_STOP(void);

// 1ms动一个节拍
void Stepmotor_Rhythm_1ms(void);

//启动电机的函数
void Stepmotor_angle_dir(uint8_t dir,u16 angle,uint16_t interval_ms);

// 判断电机是否运行
uint8_t Stepmotor_is_run(void);

void STEPMOTOR_Init(void);

uint16_t Stepmotor_get_curtain_status(void);

void Stepmotor_light_control(uint16_t light);

//void STEPMOTOR_Rhythm_4_1_4(u8 step,u8 dly);
//void STEPMOTOR_Direction(u8 dir,u8 num,u8 dly);
//void STEPMOTOR_Direction_Angle(u8 dir,u8 num,u16 angle,u8 dly);
//void STEPMOTOR_STOP(void);
#endif



