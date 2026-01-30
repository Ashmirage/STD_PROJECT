#include "stm32f10x.h"
#include "Delay.h"

// 定义4个控制IN的IO端口
#define STEPMOTOR_A_GPIO_PORT GPIOA
#define STEPMOTOR_B_GPIO_PORT GPIOB
#define STEPMOTOR_C_GPIO_PORT GPIOC
#define STEPMOTOR_D_GPIO_PORT GPIOE

// 定义4个控制IN的IO引脚
#define STEPMOTOR_A_PIN				    GPIO_Pin_0
#define STEPMOTOR_B_PIN 				GPIO_Pin_12
#define STEPMOTOR_C_PIN 				GPIO_Pin_3
#define STEPMOTOR_D_PIN 				GPIO_Pin_2

#define STEPMOTOR_A_HIGH GPIO_SetBits(STEPMOTOR_A_GPIO_PORT,STEPMOTOR_A_PIN)
#define STEPMOTOR_A_LOW GPIO_ResetBits(STEPMOTOR_A_GPIO_PORT,STEPMOTOR_A_PIN)

#define STEPMOTOR_B_HIGH GPIO_SetBits(STEPMOTOR_B_GPIO_PORT,STEPMOTOR_B_PIN)
#define STEPMOTOR_B_LOW GPIO_ResetBits(STEPMOTOR_B_GPIO_PORT,STEPMOTOR_B_PIN)

#define STEPMOTOR_C_HIGH GPIO_SetBits(STEPMOTOR_C_GPIO_PORT,STEPMOTOR_C_PIN)
#define STEPMOTOR_C_LOW GPIO_ResetBits(STEPMOTOR_C_GPIO_PORT,STEPMOTOR_C_PIN)

#define STEPMOTOR_D_HIGH GPIO_SetBits(STEPMOTOR_D_GPIO_PORT,STEPMOTOR_D_PIN)
#define STEPMOTOR_D_LOW GPIO_ResetBits(STEPMOTOR_D_GPIO_PORT,STEPMOTOR_D_PIN)

// 开始端口的时钟,需要修改
#define STEPMOTOR_GPIO_CLK_FUNCTION() \
do{ \
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE); \
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE); \
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE); \
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE); \
}while(0);


// 最好使用模式0,转矩最大,转速最快,延时可以只给1ms
// 模式一, 单相通电,转矩小,转速慢,需要4ms才能转动,否则只能震动
// 模式二,双向通电,转矩够大,但是由于惯性需要2ms延时


/**
* @brief  步进电机初始化函数
 *
* @param[in] None
 * @return None
 *
*/
void STEPMOTOR_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	STEPMOTOR_GPIO_CLK_FUNCTION();
	
	GPIO_InitStructure.GPIO_Pin = STEPMOTOR_A_PIN; //配置复用时钟
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(STEPMOTOR_A_GPIO_PORT,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = STEPMOTOR_B_PIN; //配置复用时钟
	GPIO_Init(STEPMOTOR_B_GPIO_PORT,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = STEPMOTOR_C_PIN; //配置复用时钟
	GPIO_Init(STEPMOTOR_C_GPIO_PORT,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = STEPMOTOR_D_PIN; //配置复用时钟
	GPIO_Init(STEPMOTOR_D_GPIO_PORT,&GPIO_InitStructure);

	GPIO_ResetBits(STEPMOTOR_A_GPIO_PORT, STEPMOTOR_A_PIN);
	GPIO_ResetBits(STEPMOTOR_B_GPIO_PORT, STEPMOTOR_B_PIN);
	GPIO_ResetBits(STEPMOTOR_C_GPIO_PORT, STEPMOTOR_C_PIN);
	GPIO_ResetBits(STEPMOTOR_D_GPIO_PORT, STEPMOTOR_D_PIN);
}

// 单相模式
void STEPMOTOR_Rhythm_4_1_4(uint8_t step,uint8_t dly)
{
	switch(step)
	{
		case 0 :
			;;
		break;
		
		case 1 :
			STEPMOTOR_A_LOW;STEPMOTOR_B_HIGH;STEPMOTOR_C_HIGH;STEPMOTOR_D_HIGH;	//1
		break;
		
		case 2 :
			STEPMOTOR_A_HIGH;STEPMOTOR_B_LOW;STEPMOTOR_C_HIGH;STEPMOTOR_D_HIGH;	//2
		break;
		
		case 3 :
			STEPMOTOR_A_HIGH;STEPMOTOR_B_HIGH;STEPMOTOR_C_LOW;STEPMOTOR_D_HIGH;	//3
		break;
		
		case 4 :
			STEPMOTOR_A_HIGH;STEPMOTOR_B_HIGH;STEPMOTOR_C_HIGH;STEPMOTOR_D_LOW;	//4
		break;		
	}
	Delay_ms(dly);
}

// 双相模式
void STEPMOTOR_Rhythm_4_2_4(uint8_t step,uint8_t dly)
{
	switch(step)
	{
		case 0 :
			;;
		break;
		
		case 1 :
			STEPMOTOR_A_LOW;STEPMOTOR_B_HIGH;STEPMOTOR_C_HIGH;STEPMOTOR_D_LOW;	//1
		break;
		
		case 2 :
			STEPMOTOR_A_LOW;STEPMOTOR_B_LOW;STEPMOTOR_C_HIGH;STEPMOTOR_D_HIGH;	//2
		break;
		
		case 3 :
			STEPMOTOR_A_HIGH;STEPMOTOR_B_LOW;STEPMOTOR_C_LOW;STEPMOTOR_D_HIGH;	//3
		break;
		
		case 4 :
			STEPMOTOR_A_HIGH;STEPMOTOR_B_HIGH;STEPMOTOR_C_LOW;STEPMOTOR_D_LOW;	//4
		break;		
	}
	
	Delay_ms(dly);
}

// 半双相模式
void STEPMOTOR_Rhythm_4_1_8(uint8_t step,uint8_t dly)
{
	switch(step)
	{
		case 0 :
			;;
		break;
		
		case 1 :
			STEPMOTOR_A_LOW;STEPMOTOR_B_HIGH;STEPMOTOR_C_HIGH;STEPMOTOR_D_HIGH;	//1
		break;
		
		case 2 :
			STEPMOTOR_A_LOW;STEPMOTOR_B_LOW;STEPMOTOR_C_HIGH;STEPMOTOR_D_HIGH;	//2
		break;
		
		case 3 :
			STEPMOTOR_A_HIGH;STEPMOTOR_B_LOW;STEPMOTOR_C_HIGH;STEPMOTOR_D_HIGH;	//3
		break;
		
		case 4 :
			STEPMOTOR_A_HIGH;STEPMOTOR_B_LOW;STEPMOTOR_C_LOW;STEPMOTOR_D_HIGH;	//4
		break;	
		
		case 5 :
			STEPMOTOR_A_HIGH;STEPMOTOR_B_HIGH;STEPMOTOR_C_LOW;STEPMOTOR_D_HIGH;	//5
		break;
		
		case 6 :
			STEPMOTOR_A_HIGH;STEPMOTOR_B_HIGH;STEPMOTOR_C_LOW;STEPMOTOR_D_LOW;	//6
		break;
		
		case 7 :
			STEPMOTOR_A_HIGH;STEPMOTOR_B_HIGH;STEPMOTOR_C_HIGH;STEPMOTOR_D_LOW;	//7
		break;
		
		case 8 :
			STEPMOTOR_A_LOW;STEPMOTOR_B_HIGH;STEPMOTOR_C_HIGH;STEPMOTOR_D_LOW;	//8
		break;			
	}
	
	Delay_ms(dly);
}

void STEPMOTOR_Direction(uint8_t dir,uint8_t num,uint8_t dly)		//方向 节拍 频率
{
	if(dir)
	{
		switch(num)
		{
			case 0:for(uint8_t i=1;i<9;i++){STEPMOTOR_Rhythm_4_1_8(i,dly);}break;
			case 1:for(uint8_t i=1;i<5;i++){STEPMOTOR_Rhythm_4_1_4(i,dly);}break;
			case 2:for(uint8_t i=1;i<5;i++){STEPMOTOR_Rhythm_4_2_4(i,dly);}break;
			default:break;
		}
	}
	else
	{
		switch(num)
		{
			case 0:for(uint8_t i=8;i>0;i--){STEPMOTOR_Rhythm_4_1_8(i,dly);}break;
			case 1:for(uint8_t i=4;i>0;i--){STEPMOTOR_Rhythm_4_1_4(i,dly);}break;
			case 2:for(uint8_t i=4;i>0;i--){STEPMOTOR_Rhythm_4_2_4(i,dly);}break;
			default:break;
		}	
	}
}

/**
* @brief  步进电机控制函数
 *
* @param[in]  dir 转动方向:0顺时针,1逆时针
*             num 转动模式:0半双相,1单相,2双相
 *            angle 转动角度
*             dly 延时的频率,越低转速越快
 * @return None
 *
*/
void STEPMOTOR_Direction_Angle(uint8_t dir,uint8_t num,u16 angle,uint8_t dly) //方向 节拍  角度 频率
{
		for(u16 i=0;i<(64*angle/45);i++)	//64*64=4096 64*180=256 //512
		{
			STEPMOTOR_Direction(dir,num,dly);	
		}
}

/**
* @brief  步进电机停止函数
 *
* @param[in] None
 * @return None
 *
*/
void STEPMOTOR_STOP(void)
{
	GPIO_ResetBits(STEPMOTOR_A_GPIO_PORT, STEPMOTOR_A_PIN);
	GPIO_ResetBits(STEPMOTOR_B_GPIO_PORT, STEPMOTOR_B_PIN);
	GPIO_ResetBits(STEPMOTOR_C_GPIO_PORT, STEPMOTOR_C_PIN);
	GPIO_ResetBits(STEPMOTOR_D_GPIO_PORT, STEPMOTOR_D_PIN);
}

