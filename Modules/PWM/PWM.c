#include "stm32f10x.h"


#define PWM_TIM_PORT TIM2
#define PWM_OC_INIT_FUNCTION TIM_OC2Init
#define PWM_GPIO_PORT GPIOA
#define PWM_GPIO_PIN GPIO_Pin_1
#define PWM_TIM_CLK_INIT_FUNCTION() RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
#define PWM_GPIO_CLK_INIT_FUNCTION() RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
#define PWM_SET_FUNCTION TIM_SetCompare2

/**
 * @brief  PWM输出初始化,默认输出频率20khz,占空比范围0-100
 *
 * @param[in]  None
 *
 * @return None
 *
 */




void PWM_init(void)
{
	PWM_TIM_CLK_INIT_FUNCTION(); // 定时器开启时钟
	
	PWM_GPIO_CLK_INIT_FUNCTION(); //输出比较端口初始化
	GPIO_InitTypeDef GPIO_init_structure;
	GPIO_init_structure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_init_structure.GPIO_Pin = PWM_GPIO_PIN;
	GPIO_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(PWM_GPIO_PORT,&GPIO_init_structure);
	
	TIM_InternalClockConfig(PWM_TIM_PORT);//时钟源选择
	TIM_TimeBaseInitTypeDef TIM_timebase_init_structure;
	TIM_timebase_init_structure.TIM_ClockDivision = TIM_CKD_DIV1; //不分频
	TIM_timebase_init_structure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数
	TIM_timebase_init_structure.TIM_Period = 100 - 1; //自动重装寄存器(注意实际分频系数需要+1)
	TIM_timebase_init_structure.TIM_Prescaler = 36 - 1; //预分频器 
	TIM_timebase_init_structure.TIM_RepetitionCounter = 0; //自动重装寄存器,高级定时器才有
	TIM_TimeBaseInit(PWM_TIM_PORT,&TIM_timebase_init_structure);
	
	TIM_OCInitTypeDef TIM_oc_init_structure;
	TIM_OCStructInit(&TIM_oc_init_structure); //先给一个默认的参数
	TIM_oc_init_structure.TIM_OCMode = TIM_OCMode_PWM1; //选择PWM模式1
	TIM_oc_init_structure.TIM_OutputState = TIM_OutputState_Enable; //输出引脚使能
	TIM_oc_init_structure.TIM_OCPolarity = TIM_OCPolarity_High; // 极性,若为低就高低电平反向
	TIM_oc_init_structure.TIM_Pulse = 0; //初始脉冲
	PWM_OC_INIT_FUNCTION(PWM_TIM_PORT,&TIM_oc_init_structure); //输出比较初始化
	
	TIM_Cmd(PWM_TIM_PORT,ENABLE); //计时器使能
}

/**
 * @brief 设置PWM占空比
 *
 * @param[in]  pwm 0-100
 *
 * @return None
 *
 */
void PWM_set(uint8_t pwm)
{
	if (pwm > 99) pwm = 99;
	PWM_SET_FUNCTION(PWM_TIM_PORT,pwm);
}


