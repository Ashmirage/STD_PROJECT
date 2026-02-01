#include "stm32f10x.h"
#include "time.h"

// 定义4个控制IN的IO端口
#define STEPMOTOR_A_GPIO_PORT GPIOC
#define STEPMOTOR_B_GPIO_PORT GPIOC
#define STEPMOTOR_C_GPIO_PORT GPIOC
#define STEPMOTOR_D_GPIO_PORT GPIOC

// 定义4个控制IN的IO引脚
#define STEPMOTOR_A_PIN				    GPIO_Pin_0
#define STEPMOTOR_B_PIN 				GPIO_Pin_1
#define STEPMOTOR_C_PIN 				GPIO_Pin_2
#define STEPMOTOR_D_PIN 				GPIO_Pin_3

#define STEPMOTOR_A_HIGH GPIO_SetBits(STEPMOTOR_A_GPIO_PORT,STEPMOTOR_A_PIN)
#define STEPMOTOR_A_LOW GPIO_ResetBits(STEPMOTOR_A_GPIO_PORT,STEPMOTOR_A_PIN)

#define STEPMOTOR_B_HIGH GPIO_SetBits(STEPMOTOR_B_GPIO_PORT,STEPMOTOR_B_PIN)
#define STEPMOTOR_B_LOW GPIO_ResetBits(STEPMOTOR_B_GPIO_PORT,STEPMOTOR_B_PIN)

#define STEPMOTOR_C_HIGH GPIO_SetBits(STEPMOTOR_C_GPIO_PORT,STEPMOTOR_C_PIN)
#define STEPMOTOR_C_LOW GPIO_ResetBits(STEPMOTOR_C_GPIO_PORT,STEPMOTOR_C_PIN)

#define STEPMOTOR_D_HIGH GPIO_SetBits(STEPMOTOR_D_GPIO_PORT,STEPMOTOR_D_PIN)
#define STEPMOTOR_D_LOW GPIO_ResetBits(STEPMOTOR_D_GPIO_PORT,STEPMOTOR_D_PIN)

void step_A(uint8_t x) {GPIO_WriteBit(STEPMOTOR_A_GPIO_PORT,STEPMOTOR_A_PIN,(BitAction)x);}
void step_B(uint8_t x) {GPIO_WriteBit(STEPMOTOR_B_GPIO_PORT,STEPMOTOR_B_PIN,(BitAction)x);}
void step_C(uint8_t x) {GPIO_WriteBit(STEPMOTOR_C_GPIO_PORT,STEPMOTOR_C_PIN,(BitAction)x);}
void step_D(uint8_t x) {GPIO_WriteBit(STEPMOTOR_D_GPIO_PORT,STEPMOTOR_D_PIN,(BitAction)x);}

// 定义函数表
void (*stepmotor_func[])(uint8_t) = {step_A,step_B,step_C,step_D};
// 开始端口的时钟,需要修改
#define STEPMOTOR_GPIO_CLK_FUNCTION() \
do{ \
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE); \
}while(0);


// 最好使用模式0,转矩最大,转速最快,延时可以只给1ms
// 模式一, 单相通电,转矩小,转速慢,需要4ms才能转动,否则只能震动
// 模式二,双向通电,转矩够大,但是由于惯性需要2ms延时


// 模式0
static const uint8_t seq8[8][4] = {
	{1,0,0,0}, // 1: A
	{1,1,0,0}, // 2: A+B
	{0,1,0,0}, // 3: B
	{0,1,1,0}, // 4: B+C
	{0,0,1,0}, // 5: C
	{0,0,1,1}, // 6: C+D
	{0,0,0,1}, // 7: D
	{1,0,0,1}, // 8: A+D
};

////模式一
//static const uint8_t seq4_1[4][4] = {
//            {1,0,0,0}, // A
//            {0,1,0,0}, // B
//            {0,0,1,0}, // C
//            {0,0,0,1}, // D
//};

////模式二
//static const uint8_t seq4_2[4][4] = {
//            {1,0,0,1}, // A+D
//            {1,1,0,0}, // A+B
//            {0,1,1,0}, // B+C
//            {0,0,1,1}, // C+D
//};		
		
typedef struct
{
	uint16_t remain_phrases; //剩余的步数
	uint8_t is_busy; // 电机是否运行
	uint8_t dir; //电机转动的方向
	uint8_t Rhythm_index; //节拍索引
	uint16_t interval_time_ms; //设置速度,每xs动一个节拍
	uint32_t last_run_time; //记录上次运行的时间
}Stepmotor;

static Stepmotor stepmotor_st;

/**
* @brief  步进电机停止函数
 *
* @param[in] None
 * @return None
 *
*/
void STEPMOTOR_STOP(void)
{
	uint8_t i;
	for(i = 0;i < 4;i++)
	{
		stepmotor_func[i](0); //全部置为低电平
	}
	stepmotor_st.is_busy = 0; //清除运行标志
	stepmotor_st.Rhythm_index = 0; //清零变量
	stepmotor_st.remain_phrases = 0;
}


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

	STEPMOTOR_STOP(); //初始关闭电机
}

// 1ms动一个节拍
void Stepmotor_Rhythm_1ms(void)
{
	if(stepmotor_st.is_busy == 1) //判断是否处于运行状态下
	{
		uint32_t now_t = SysTick_GetTick(); //获取现在时间
		if((now_t - stepmotor_st.last_run_time) < stepmotor_st.interval_time_ms)
		{
			return;//不满足执行条件,退出
		}
		//满足执行条件,更新时间
		stepmotor_st.last_run_time = now_t;
		// 防止下溢出
		if(stepmotor_st.remain_phrases == 0)
		{
			STEPMOTOR_STOP(); // 停止电机
			return;
		}
		if(stepmotor_st.dir == 1)// 判断方向
		{ // 四相依次运动一个拍
			uint8_t i;
			for(i = 0;i < 4;i++)
			{
				if(seq8[stepmotor_st.Rhythm_index][i]){stepmotor_func[i](1);}
				else{stepmotor_func[i](0);}
			}
		}else
		{
			uint8_t i;
			for(i = 0;i < 4;i++)
			{
				if(seq8[7 - stepmotor_st.Rhythm_index][i]){stepmotor_func[i](1);}
				else{stepmotor_func[i](0);}
			}
		}
		stepmotor_st.Rhythm_index++; //节拍++
		if(stepmotor_st.Rhythm_index > 7){stepmotor_st.Rhythm_index = 0;} //循环处理
		stepmotor_st.remain_phrases--; //剩余步数--
		if(stepmotor_st.remain_phrases == 0)
		{
			STEPMOTOR_STOP(); // 停止电机
		}
	}
}

//启动电机的函数
void Stepmotor_angle_dir(uint8_t dir,u16 angle,uint16_t interval_ms)
{
	// 如果忙碌就忽略这个指令
	if(stepmotor_st.is_busy == 1)
	{
		return;
	}
	stepmotor_st.dir = dir; //设置方向
	stepmotor_st.remain_phrases = 64*angle/45*8; //计算步数
	stepmotor_st.Rhythm_index = 0; //初始化状态
	if(interval_ms == 0)
	{
		interval_ms = 1; //对下限限幅
	}
	stepmotor_st.interval_time_ms = interval_ms; //设置速度
	stepmotor_st.last_run_time = SysTick_GetTick();//初始化上次运行时间
	stepmotor_st.is_busy = 1; //启动电机
}

// 判断电机是否运行
uint8_t Stepmotor_is_run(void)
{
	return stepmotor_st.is_busy;
}

// 单相模式
//void STEPMOTOR_Rhythm_4_1_4(uint8_t step,uint8_t dly)
//{
//	switch(step)
//	{
//		case 0 :
//			;;
//		break;
//		
//		case 1 :
//			STEPMOTOR_A_LOW;STEPMOTOR_B_HIGH;STEPMOTOR_C_HIGH;STEPMOTOR_D_HIGH;	//1
//		break;
//		
//		case 2 :
//			STEPMOTOR_A_HIGH;STEPMOTOR_B_LOW;STEPMOTOR_C_HIGH;STEPMOTOR_D_HIGH;	//2
//		break;
//		
//		case 3 :
//			STEPMOTOR_A_HIGH;STEPMOTOR_B_HIGH;STEPMOTOR_C_LOW;STEPMOTOR_D_HIGH;	//3
//		break;
//		
//		case 4 :
//			STEPMOTOR_A_HIGH;STEPMOTOR_B_HIGH;STEPMOTOR_C_HIGH;STEPMOTOR_D_LOW;	//4
//		break;		
//	}
//	Delay_ms(dly);
//}

// 双相模式
//void STEPMOTOR_Rhythm_4_2_4(uint8_t step,uint8_t dly)
//{
//	switch(step)
//	{
//		case 0 :
//			;;
//		break;
//		
//		case 1 :
//			STEPMOTOR_A_LOW;STEPMOTOR_B_HIGH;STEPMOTOR_C_HIGH;STEPMOTOR_D_LOW;	//1
//		break;
//		
//		case 2 :
//			STEPMOTOR_A_LOW;STEPMOTOR_B_LOW;STEPMOTOR_C_HIGH;STEPMOTOR_D_HIGH;	//2
//		break;
//		
//		case 3 :
//			STEPMOTOR_A_HIGH;STEPMOTOR_B_LOW;STEPMOTOR_C_LOW;STEPMOTOR_D_HIGH;	//3
//		break;
//		
//		case 4 :
//			STEPMOTOR_A_HIGH;STEPMOTOR_B_HIGH;STEPMOTOR_C_LOW;STEPMOTOR_D_LOW;	//4
//		break;		
//	}
//	
//	Delay_ms(dly);
//}

// 半双相模式
//void STEPMOTOR_Rhythm_4_1_8(uint8_t step,uint8_t dly)
//{
//	switch(step)
//	{
//		case 0 :
//			;;
//		break;
//		
//		case 1 :
//			STEPMOTOR_A_LOW;STEPMOTOR_B_HIGH;STEPMOTOR_C_HIGH;STEPMOTOR_D_HIGH;	//1
//		break;
//		
//		case 2 :
//			STEPMOTOR_A_LOW;STEPMOTOR_B_LOW;STEPMOTOR_C_HIGH;STEPMOTOR_D_HIGH;	//2
//		break;
//		
//		case 3 :
//			STEPMOTOR_A_HIGH;STEPMOTOR_B_LOW;STEPMOTOR_C_HIGH;STEPMOTOR_D_HIGH;	//3
//		break;
//		
//		case 4 :
//			STEPMOTOR_A_HIGH;STEPMOTOR_B_LOW;STEPMOTOR_C_LOW;STEPMOTOR_D_HIGH;	//4
//		break;	
//		
//		case 5 :
//			STEPMOTOR_A_HIGH;STEPMOTOR_B_HIGH;STEPMOTOR_C_LOW;STEPMOTOR_D_HIGH;	//5
//		break;
//		
//		case 6 :
//			STEPMOTOR_A_HIGH;STEPMOTOR_B_HIGH;STEPMOTOR_C_LOW;STEPMOTOR_D_LOW;	//6
//		break;
//		
//		case 7 :
//			STEPMOTOR_A_HIGH;STEPMOTOR_B_HIGH;STEPMOTOR_C_HIGH;STEPMOTOR_D_LOW;	//7
//		break;
//		
//		case 8 :
//			STEPMOTOR_A_LOW;STEPMOTOR_B_HIGH;STEPMOTOR_C_HIGH;STEPMOTOR_D_LOW;	//8
//		break;			
//	}
//	
//	Delay_ms(dly);
//}

//void STEPMOTOR_Direction(uint8_t dir,uint8_t num,uint8_t dly)		//方向 节拍 频率
//{
//	if(dir)
//	{
//		switch(num)
//		{
//			case 0:for(uint8_t i=1;i<9;i++){STEPMOTOR_Rhythm_4_1_8(i,dly);}break;
//			case 1:for(uint8_t i=1;i<5;i++){STEPMOTOR_Rhythm_4_1_4(i,dly);}break;
//			case 2:for(uint8_t i=1;i<5;i++){STEPMOTOR_Rhythm_4_2_4(i,dly);}break;
//			default:break;
//		}
//	}
//	else
//	{
//		switch(num)
//		{
//			case 0:for(uint8_t i=8;i>0;i--){STEPMOTOR_Rhythm_4_1_8(i,dly);}break;
//			case 1:for(uint8_t i=4;i>0;i--){STEPMOTOR_Rhythm_4_1_4(i,dly);}break;
//			case 2:for(uint8_t i=4;i>0;i--){STEPMOTOR_Rhythm_4_2_4(i,dly);}break;
//			default:break;
//		}	
//	}
//}

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
//void STEPMOTOR_Direction_Angle(uint8_t dir,uint8_t num,u16 angle,uint8_t dly) //方向 节拍  角度 频率
//{
//		for(u16 i=0;i<(64*angle/45);i++)	//64*64=4096 64*180=256 //512
//		{
//			STEPMOTOR_Direction(dir,num,dly);	
//		}
//}



