#include "stm32f10x.h"
#include "my_usart.h"
#include <stdio.h>

#define TIM_PORT TIM2 //定时器端口
#define TIM_IT_CLK_FUNCTION() RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE); //定时器时钟
#define TIM_IT_FUNCTION  TIM2_IRQHandler //定时器函数
// 需要修改中断函数
/**
 * @brief  定时器1s中断初始化
 *
 * @param[in]  None
 *
 * @return None
 *
 */
void TIM_it_init(void)
{
	TIM_IT_CLK_FUNCTION();
	
	TIM_InternalClockConfig(TIM_PORT); //时钟源选择
	TIM_TimeBaseInitTypeDef TIM_timebase_init_structure;
	TIM_timebase_init_structure.TIM_ClockDivision = TIM_CKD_DIV1; //不分频
	TIM_timebase_init_structure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数
	TIM_timebase_init_structure.TIM_Period = 9999; //自动重装寄存器(注意实际分频系数需要+1)
	TIM_timebase_init_structure.TIM_Prescaler = 7199; //预分频器 
	TIM_timebase_init_structure.TIM_RepetitionCounter = 0; //自动重装寄存器,高级定时器才有
	TIM_TimeBaseInit(TIM_PORT,&TIM_timebase_init_structure);
	TIM_ClearFlag(TIM_PORT, TIM_FLAG_Update); //清除更新标志位
	TIM_ITConfig(TIM_PORT,TIM_IT_Update,ENABLE); //使能定时器更新中断
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_init_structure;
	NVIC_init_structure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_init_structure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_init_structure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_init_structure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_init_structure);
	
	TIM_Cmd(TIM_PORT,ENABLE);
}


/**
 * @brief  中断函数
 *
 * @param[in]  BaudRate 波特率
 *
 * @return None
 *
 */
void TIM_IT_FUNCTION(void)
{
	if(TIM_GetITStatus(TIM_PORT,TIM_IT_Update) == SET)
	{
		//Send_printf("tim interrupt!");
		TIM_ClearITPendingBit(TIM_PORT,TIM_IT_Update); //清除中断
	}
}





