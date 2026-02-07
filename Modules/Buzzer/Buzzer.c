#include "stm32f10x.h"
#include "my_time.h"

#define Buzzer_PORT GPIOB
#define Buzzer_PIN GPIO_Pin_8
#define Buzzer_CLK_FUNCTION() RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);



void Buzzer_init(void)
{
	Buzzer_CLK_FUNCTION();
	
	GPIO_InitTypeDef GPIO_init_structure;
	GPIO_init_structure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_init_structure.GPIO_Pin = Buzzer_PIN;
	GPIO_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(Buzzer_PORT,&GPIO_init_structure);
	GPIO_WriteBit(Buzzer_PORT,Buzzer_PIN,Bit_RESET);
}

// 注意区分是否是高电平点亮
void Buzzer_ON(void)
{
	GPIO_WriteBit(Buzzer_PORT,Buzzer_PIN,Bit_SET);
}

void Buzzer_OFF(void)
{
	GPIO_WriteBit(Buzzer_PORT,Buzzer_PIN,Bit_RESET);
}

static uint8_t alarm_mode = 0; //默认非报警模式


void Buzzer_alarm_start(void)
{
	alarm_mode = 1;
}

void Buzzer_alarm(uint16_t ms)
{
	if(alarm_mode == 1)//如果是报警模式
	{
		static uint8_t status = 0;
		static uint32_t count = 0;
		switch(status)
		{
			case 0://最短鸣叫
				count++;
				Buzzer_ON();
				if(count * ms >= 150)
				{
					Buzzer_OFF();
					count = 0;
					status = 1;
				}
			break;
			case 1:
				count++;
				if(count * ms >= 200)
				{
					count = 0;
					status = 2;
				}
			break;
			case 2:
				count++;
				Buzzer_ON();
				if(count * ms >= 300)
				{
					Buzzer_OFF();
					count = 0;
					status = 3;
				}
			break;
			case 3:
				count++;
				if(count * ms >= 200)
				{
					count = 0;
					status = 4;
				}
			break;
			case 4:
				count++;
				Buzzer_ON();
				if(count * ms >= 450)
				{
					Buzzer_OFF();
					count = 0;
					status = 0;
					alarm_mode = 0;
				}
			break;
		}
	}
}


