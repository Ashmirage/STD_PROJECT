#include "stm32f10x.h"
   
#define ADC_GPIO_CLK_FUNCTION() \
do{ \
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); \
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE); \
}while(0);


#define ADC_CHANNEL1_PORT GPIOC
#define ADC_CHANNEL1_PIN  GPIO_Pin_5
#define ADC_CHANNEL1_NAME ADC_Channel_15
#define ADC_CHANNEL2_PORT GPIOF
#define ADC_CHANNEL2_PIN  GPIO_Pin_6
#define ADC_CHANNEL2_NAME ADC_Channel_4
#define ADC_CHANNEL_NUM 1
#define DMA_CHANNEL_NUM 1


// 注意f103系列的芯片的dma外设只有一个dma1 
// dma转运时通道对应关系是硬件定死的,不可以随便选择,比如ADC采样结果转运就只能使用通道一


// f103系列的只有ADC1才支持DMA,不要使用ADC3和ADC2
// 扫描模式注意修改结构体配置


volatile uint16_t AD_Value[4];

#define ADC_DATA_READ_ADDRESS AD_Value
void AD_init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	//开启ADC1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);	//开启ADC3的时钟
	ADC_GPIO_CLK_FUNCTION();	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);		//开启DMA1的时钟
	
	/*设置ADC时钟*/
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);						//选择时钟6分频，ADCCLK = 72MHz / 6 = 12MHz
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = ADC_CHANNEL1_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(ADC_CHANNEL1_PORT, &GPIO_InitStructure);	 //ADC1的IO口初始化
	
	GPIO_InitStructure.GPIO_Pin = ADC_CHANNEL2_PIN;
	GPIO_Init(ADC_CHANNEL2_PORT, &GPIO_InitStructure);	//ADC3的IO口初始化
	
	/*规则组通道配置*/
	ADC_RegularChannelConfig(ADC1, ADC_CHANNEL1_NAME, 1, ADC_SampleTime_71Cycles5);	//规则组序列1的位置，配置为通道0
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_55Cycles5);	//规则组序列2的位置，配置为通道1
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_55Cycles5);	//规则组序列3的位置，配置为通道2
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_55Cycles5);	//规则组序列4的位置，配置为通道3
	
	ADC_RegularChannelConfig(ADC3, ADC_CHANNEL2_NAME, 1, ADC_SampleTime_71Cycles5);	
	
	/*ADC初始化*/
	ADC_InitTypeDef ADC_InitStructure;											//定义结构体变量
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;							//模式，选择独立模式，即单独使用ADC1
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;						//数据对齐，选择右对齐
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;			//外部触发，使用软件触发，不需要外部触发
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;							//连续转换，使能，每转换一次规则组序列后立刻开始下一次转换
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;								//扫描模式，使能，扫描规则组的序列，扫描数量由ADC_NbrOfChannel确定
	ADC_InitStructure.ADC_NbrOfChannel = ADC_CHANNEL_NUM;										//通道数，为4，扫描规则组的前4个通道
	ADC_Init(ADC1, &ADC_InitStructure);											//将结构体变量交给ADC_Init，配置ADC1
	ADC_Init(ADC3, &ADC_InitStructure);											//将结构体变量交给ADC_Init，配置ADC3
	
	
	/*DMA初始化*/
	DMA_InitTypeDef DMA_InitStructure;											//定义结构体变量
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;				//外设基地址，给定形参AddrA
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//外设数据宽度，选择半字，对应16为的ADC数据寄存器
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			//外设地址自增，选择失能，始终以ADC数据寄存器为源
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC_DATA_READ_ADDRESS;					//存储器基地址，给定存放AD转换结果的全局数组AD_Value
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			//存储器数据宽度，选择半字，与源数据宽度对应
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						//存储器地址自增，选择使能，每次转运后，数组移到下一个位置
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;							//数据传输方向，选择由外设到存储器，ADC数据寄存器转到数组
	DMA_InitStructure.DMA_BufferSize = DMA_CHANNEL_NUM;										//转运的数据大小（转运次数），与ADC通道数一致
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;								//模式，选择循环模式，与ADC的连续转换一致
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;								//存储器到存储器，选择失能，数据由ADC外设触发转运到存储器
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;						//优先级，选择中等
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);								//将结构体变量交给DMA_Init，配置DMA1的通道1
	
	/*DMA和ADC使能*/
	DMA_Cmd(DMA1_Channel1, ENABLE);							//DMA1的通道1使能
	ADC_DMACmd(ADC1, ENABLE);								//ADC1触发DMA1的信号使能
	ADC_Cmd(ADC1, ENABLE);									//ADC1使能
	
	ADC_Cmd(ADC3, ENABLE);									//ADC3使能
	/*ADC校准*/
	ADC_ResetCalibration(ADC1);								//固定流程，内部有电路会自动执行校准
	while (ADC_GetResetCalibrationStatus(ADC1) == SET);
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1) == SET);
	
	ADC_ResetCalibration(ADC3);								//固定流程，内部有电路会自动执行校准
	while (ADC_GetResetCalibrationStatus(ADC3) == SET);
	ADC_StartCalibration(ADC3);
	while (ADC_GetCalibrationStatus(ADC3) == SET);
	
	/*ADC触发*/
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	//软件触发ADC开始工作，由于ADC处于连续转换模式，故触发一次后ADC就可以一直连续不断地工作
	
	ADC_SoftwareStartConvCmd(ADC3, ENABLE);
}







