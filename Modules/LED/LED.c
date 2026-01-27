#include "stm32f10x.h"

#define LED_PORT GPIOB
#define LED_PIN GPIO_Pin_6
#define LED_CLK_FUNCTION() RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

//#define LED_ACTIVE_HIGH 1

#ifdef LED_ACTIVE_HIGH 
#define LED_ACTIVE_STATUS Bit_SET
#define LED_NOACTIVE_STATUS Bit_RESET
#else
#define LED_ACTIVE_STATUS Bit_RESET
#define LED_NOACTIVE_STATUS Bit_SET
#endif


void LED_init(void)
{
	LED_CLK_FUNCTION();
	
	GPIO_InitTypeDef GPIO_init_structure;
	GPIO_init_structure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_init_structure.GPIO_Pin = LED_PIN;
	GPIO_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LED_PORT,&GPIO_init_structure);
	GPIO_WriteBit(LED_PORT,LED_PIN,LED_NOACTIVE_STATUS);
}

void LED_ON(void)
{
	GPIO_WriteBit(LED_PORT,LED_PIN,LED_ACTIVE_STATUS);
}

void LED_OFF(void)
{
	GPIO_WriteBit(LED_PORT,LED_PIN,LED_NOACTIVE_STATUS);
}

void LED_Toggle(void)
{
	// 注意,读取的是输出电平
	if(GPIO_ReadOutputDataBit(LED_PORT,LED_PIN) == LED_ACTIVE_STATUS)
	{
		GPIO_WriteBit(LED_PORT,LED_PIN,LED_NOACTIVE_STATUS);
	}
	else
	{
		GPIO_WriteBit(LED_PORT,LED_PIN,LED_ACTIVE_STATUS);
	}
}


