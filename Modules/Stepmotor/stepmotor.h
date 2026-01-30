#ifndef __STEPMOTOR_H
#define	__STEPMOTOR_H
#include "stm32f10x.h"



void STEPMOTOR_Init(void);
void STEPMOTOR_Rhythm_4_1_4(u8 step,u8 dly);
void STEPMOTOR_Direction(u8 dir,u8 num,u8 dly);
void STEPMOTOR_Direction_Angle(u8 dir,u8 num,u16 angle,u8 dly);
void STEPMOTOR_STOP(void);
#endif



