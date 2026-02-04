#include "stm32f10x.h"
#include "AD.h"
#include "time.h"
#include <math.h>

#define LDR_READ_TIMES	10  //光照传感器ADC循环读取次数

// 初始化函数,初始化AD转换GPIO口,通道配置,DMA配置
//void LDR_Init(void)
//{
//	AD_init();
//}

static uint16_t ldr_data[LDR_READ_TIMES]; 
static uint16_t average_data = 0;

// 任务调度器里面,1ms读取一次
void LDR_ADC_Read_1ms(void)
{
	static uint8_t cnt = 0;
	ldr_data[cnt++] = ADC_GetConversionValue(ADC3);
	if(cnt >= LDR_READ_TIMES)
	{
		cnt = 0;
		uint32_t sum = 0;
		for(uint8_t i = 0;i < LDR_READ_TIMES;i++)
		{
			sum += ldr_data[i];//累加
		}
		average_data = sum / LDR_READ_TIMES;
	}
}

//// 计算平均数据
//uint16_t LDR_Average_Data(void)
//{
//	uint32_t  tempData = 0;
//	for (uint8_t i = 0; i < LDR_READ_TIMES; i++)
//	{
//		tempData += LDR_ADC_Read();
//		Delay_ms(5);
//	}

//	tempData /= LDR_READ_TIMES;
//	return (uint16_t)tempData;
//}

// 得到lux值,每2s读取一次
uint16_t LDR_LuxData(void)
{
	float voltage = 0;	
	float R = 0;	
	uint16_t Lux = 0;
	voltage = average_data;
	voltage  = voltage / 4096 * 3.3f;
	
	R = voltage / (3.3f - voltage) * 10000;
		
	Lux = 40000 * pow(R, -0.6021);
	
//	if (Lux > 999)
//	{
//		Lux = 999;
//	}
	return Lux;
}

//void LDR_LuxData(uint16_t *Lux)
//{
//	float voltage = 0;	
//	float R = 0;	
//	voltage = LDR_Average_Data();
//	voltage  = voltage / 4096 * 3.3f;
//	
//	R = voltage / (3.3f - voltage) * 10000;
//		
//	*Lux = 40000 * pow(R, -0.6021);
//	
//	if (*Lux > 999)
//	{
//		*Lux = 999;
//	}
//}

