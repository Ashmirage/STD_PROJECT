#include "stm32f10x.h"

#define RELAY_GPIO_CLK_FUNCTION()   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
#define RELAY_GPIO_PORT GPIOA
#define RELAY_GPIO_PIN GPIO_Pin_5

void Relay_init(void)
{
	RELAY_GPIO_CLK_FUNCTION();
	
	GPIO_InitTypeDef GPIO_InitStructure;
 	GPIO_InitStructure.GPIO_Pin = RELAY_GPIO_PIN;				 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(RELAY_GPIO_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(RELAY_GPIO_PORT,RELAY_GPIO_PIN);  //高电平有效,默认失效
}

void Relay_on(void)
{
	GPIO_SetBits(RELAY_GPIO_PORT,RELAY_GPIO_PIN);
}


void Relay_off(void)
{
	GPIO_ResetBits(RELAY_GPIO_PORT,RELAY_GPIO_PIN);
}

void Relay_status(uint8_t status)
{
	if(status == 1)
	{
		Relay_on();
	}else
	{
		Relay_off();
	}
}




