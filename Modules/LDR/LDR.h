#ifndef __LDR_H
#define	__LDR_H
#include <stdint.h>


void LDR_Init(void);
	
uint16_t LDR_LuxData(void);

// 任务调度器里面,1ms读取一次
void LDR_ADC_Read_1ms(void);



#endif

