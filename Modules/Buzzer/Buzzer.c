#include "stm32f10x.h"
#include "Delay.h"

#define Buzzer_PORT GPIOB
#define Buzzer_PIN GPIO_Pin_6
#define Buzzer_CLK_FUNCTION() RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);



void Buzzer_init(void)
{
	Buzzer_CLK_FUNCTION();
	
	GPIO_InitTypeDef GPIO_init_structure;
	GPIO_init_structure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_init_structure.GPIO_Pin = Buzzer_PIN;
	GPIO_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(Buzzer_PORT,&GPIO_init_structure);
	GPIO_WriteBit(Buzzer_PORT,Buzzer_PIN,Bit_SET);
}

void Buzzer_ON(void)
{
	GPIO_WriteBit(Buzzer_PORT,Buzzer_PIN,Bit_RESET);
}

void Buzzer_OFF(void)
{
	GPIO_WriteBit(Buzzer_PORT,Buzzer_PIN,Bit_SET);
}


