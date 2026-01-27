#include "stm32f10x.h"
#include <stdio.h>
#include <stdarg.h>

#define USART_PORT USART1 // 串口端口选择
#define USART_CLK_ENABLE() RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE) //串口时钟初始化函数
#define GPIO_USART_CLK RCC_APB2Periph_GPIOA // GPIO时钟(注意修改函数名称)
#define GPIO_USART_TX_PIN GPIO_Pin_9 //发送引脚号
#define GPIO_USART_RX_PIN GPIO_Pin_10 //接收引脚号
#define USART_IT_CHANNEL USART1_IRQn // 串口中断通道
#define USART_IT_FUNCTION USART1_IRQHandler //串口中断函数

char Serial_RxPacket[100];				//接收数据包的缓存数组
uint8_t Serial_RxFlag;					//接收数据包标志位

/**
 * @brief  函数的简要功能描述（一句话）
 *
 * @param[in]   xxx   参数含义，单位，取值范围
 * @param[out]  yyy   输出参数的含义
 * @param[in,out] zzz 输入输出参数说明
 *
 * @return 返回值含义
 *         - 0：成功
 *         - -1：失败（原因）
 *
 * @note   使用注意事项（是否阻塞、是否依赖硬件等）
 * @warning 警告（错误使用会造成什么后果）
 */



/**
 * @brief  串口初始化
 *
 * @param[in]  BaudRate 波特率
 *
 * @return None
 *
 */
void My_usart_init(uint32_t BaudRate)
{
	USART_CLK_ENABLE();
	USART_InitTypeDef USART_init_structure;
	USART_init_structure.USART_BaudRate = BaudRate;
	USART_init_structure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_init_structure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; //发送接收
	USART_init_structure.USART_Parity = USART_Parity_No; //无校验
	USART_init_structure.USART_StopBits = USART_StopBits_1; //一位停止位
	USART_init_structure.USART_WordLength = USART_WordLength_8b; //8位字长
	USART_Init(USART_PORT,&USART_init_structure);
	
	// GPIO初始化
	RCC_APB2PeriphClockCmd(GPIO_USART_CLK,ENABLE);
	GPIO_InitTypeDef GPIO_init_structure;
	GPIO_init_structure.GPIO_Mode = GPIO_Mode_AF_PP; //复用推挽输出
	GPIO_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_init_structure.GPIO_Pin = GPIO_USART_TX_PIN;
	GPIO_Init(GPIOA,&GPIO_init_structure);
	GPIO_init_structure.GPIO_Pin = GPIO_USART_RX_PIN;
	GPIO_init_structure.GPIO_Mode = GPIO_Mode_IPU; //上拉
	GPIO_Init(GPIOA,&GPIO_init_structure);
	
	// 开启接收中断
	USART_ITConfig(USART_PORT,USART_IT_RXNE,ENABLE);
	
	// 中断优先级分组
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_init_structure;
	NVIC_init_structure.NVIC_IRQChannel = USART_IT_CHANNEL;
	NVIC_init_structure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_init_structure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_init_structure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_init_structure);
	
	// 串口使能
	USART_Cmd(USART_PORT,ENABLE);
}


/**
 * @brief  串口发送一个字节
 *
 * @param[in]  byte 要发送的字节
 *
 * @return None
 *
 */
void Send_byte(uint8_t byte)
{
	USART_SendData(USART_PORT,byte);
	while(USART_GetFlagStatus(USART_PORT,USART_FLAG_TXE) == RESET);//等待发送寄存器空
}

/**
 * @brief  串口发送一个数组
 *
 * @param[in]  array 数组指针
				length 长度
 *
 * @return None
 *
 */
void Send_array(uint8_t* array,uint16_t length)
{
	uint16_t i;
	for(i = 0;i < length;i++)
	{	
		Send_byte(array[i]);
	}
}


/**
 * @brief  串口发送一个字符串
 *
 * @param[in]  str 数组指针
 *
 * @return None
 *
 */
void Send_string(char* str)
{
	uint16_t i = 0;
	while(str[i] != '\0')
	{
		Send_byte(str[i]);
		i++;
	}
}

/**
  * 函    数：次方函数（内部使用）
  * 返 回 值：返回值等于X的Y次方
  */
uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	
	while (Y --)			
	{
		Result *= X;		
	}
	return Result;
}

/**
  * 函    数：串口发送数字
  * 参    数：Number 要发送的数字，范围：0~4294967295
  * 参    数：Length 要发送数字的长度，范围：0~10
  * 返 回 值：无
  */
void Send_number(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)		
	{
		Send_byte(Number / Serial_Pow(10, Length - i - 1) % 10 + '0');	
	}
}

/**
  * 函    数：自己封装的prinf函数
  * 参    数：format 格式化字符串
  * 参    数：... 可变的参数列表
  * 返 回 值：无
  */
void Send_printf(const char* fm,...)
{
	char str[100]; // 缓存数组
	va_list arg; //可变参数列表指针
	va_start(arg,fm); //将列表指针初始化为固定参数后边的可变参数开始位置
	vsprintf(str,fm,arg); //将其打印到缓存数组里面
	va_end(arg); //释放指针
	Send_string(str);
}


/**
  * 函    数：USART1中断函数
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数为中断函数，无需调用，中断触发后自动执行
  *           函数名为预留的指定名称，可以从启动文件复制
  *           请确保函数名正确，不能有任何差异，否则中断函数将不能进入
  */
void USART_IT_FUNCTION(void)
{
	static uint8_t rx_status = 0;
	static uint8_t rx_pos = 0;
	if(USART_GetITStatus(USART_PORT,USART_IT_RXNE) == SET)
	{
		uint8_t byte = USART_ReceiveData(USART_PORT);
		if(byte == '@' && rx_status == 0 && Serial_RxFlag == 0)
		{
			rx_status++;
		}
		else if(byte == '#' && rx_status == 1)
		{
			Serial_RxPacket[rx_pos] = '\0';
			rx_status = 0;
			rx_pos = 0;
			Serial_RxFlag = 1;
		}
		else if(rx_status == 1)
		{
			Serial_RxPacket[rx_pos] = byte;
			rx_pos++;
		}
		USART_ClearITPendingBit(USART_PORT,USART_IT_RXNE);
	}
}

