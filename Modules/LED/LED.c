#include "stm32f10x.h"
#include "LED.h"

#define LED_PORT GPIOB
#define LED_PIN GPIO_Pin_5
#define LED_CLK_FUNCTION() RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

//#define LED_ACTIVE_HIGH 1

#ifdef LED_ACTIVE_HIGH 
#define LED_ACTIVE_STATUS Bit_SET
#define LED_NOACTIVE_STATUS Bit_RESET
#else
#define LED_ACTIVE_STATUS Bit_RESET
#define LED_NOACTIVE_STATUS Bit_SET
#endif



static LED_status led_status =  LED_STATUS_OFF;

LED_status LED_get_status(void)
{
	return led_status;
}

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
	led_status = LED_STATUS_ON;
}

void LED_OFF(void)
{
	GPIO_WriteBit(LED_PORT,LED_PIN,LED_NOACTIVE_STATUS);
	led_status = LED_STATUS_OFF;
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
	led_status = ~led_status;
}

static uint32_t led_tick = 0;
static uint8_t mode = 0;// 默认0非闪烁模式,1闪烁模式
static uint16_t led_ms = 500;//闪烁间隔,默认500ms


// LED专用时钟,在定时器中断函数中被调用
void LED_tick(void)
{
	led_tick++;
	if(led_tick >= led_ms)
	{
		led_tick = 0;
		if(mode == 1)
		{
			LED_Toggle();//到时间了,toggle一下
		}
	}
}

// 开启LED闪烁
void LED_flash_on(uint16_t ms)
{
	if(ms <= 100) //进行粗略限幅
	{
		ms = 100;
	}
	led_ms = ms;//设置LED闪烁的间隔时间
	mode = 1;//开启闪烁
}

void LED_flash_off(void)
{
	mode = 0;//关闭闪烁
}

