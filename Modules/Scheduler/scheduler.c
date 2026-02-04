#include "stm32f10x.h"
#include "time.h"
#include <stdint.h>
#include "my_usart.h"
#include "stepmotor.h"
#include "Lcd.h"
#include "UI.h"
#include "Matrix_keyboard.h"
#include "LED.h"
#include "Buzzer.h"
#include "Relay.h"
#include "TIM_IT.h"
#include "Temp.h"
#include "AD.h"
#include "LDR.h"
#include "DHT11.h"
#include "PWM.h"
#include "Motor.h"

#define TASK_NUM 7
// 1ms执行一次
static void Loop_1000hz(void)
{
	Stepmotor_Rhythm_1ms(); // 步进电机
	LDR_ADC_Read_1ms(); //光敏ADC采样读取
}

// 2ms执行一次
static void Loop_500hz(void)
{
	
}

// 5ms执行一次
static void Loop_200hz(void)
{
	
}

// 20ms执行一次
static void Loop_50hz(void)
{
	APP(20); //APP业务
	Buzzer_alarm(20); //蜂鸣器报警
	APP_data_update();
	APP_control();
}

// 500ms执行一次
static void Loop_2hz(void)
{
	//Send_printf("main running...\r\n");
//	Send_printf("hum=%d\r\n",DHT11_getdata().humidity);
}

// 1s执行一次
static void Loop_1hz(void)
{
	DHT11_update_data();//DHT11数据读取,这是阻塞式的,25ms左右
}

static void Loop_half_hz(void)
{
	APP_show_info(); //更新显示屏数据
}

// 定义执行任务的结构体
typedef struct
{
	void (*task_func)(void); //任务函数的指针
	float task_hz; //任务的执行频率
	uint16_t interval_ticks; //任务执行的间隔tick数
	uint32_t last_runtime; //任务上次执行的时间
}scheduler_task;


// 定义所有要执行的任务,一个数组
scheduler_task task_array[] = {
	{Loop_1000hz,1000,0,0},
	{Loop_500hz,500,0,0},
	{Loop_200hz,200,0,0},
	{Loop_50hz,50,0,0},
	{Loop_2hz,2,0,0},
	{Loop_1hz,1,0,0},
	{Loop_half_hz,0.5,0,0}
};

// 裸机任务调度器初始化
void Scheduler_init(void)
{
	uint8_t i;
	for(i = 0;i < TASK_NUM;i++)
	{
		task_array[i].interval_ticks = (uint16_t)1000 / task_array[i].task_hz; //计算每个任务多少个tick执行一次 T * 1000 = 1000 / f
		if(task_array[i].interval_ticks < 1) // 如果执行tick间隔小于1,至少应该为1
		{
			task_array[i].interval_ticks = 1;
		}
	}
}

// 主循环里面不断循环执行这个函数
void Scheduler_run(void)
{
	uint8_t index;
	uint32_t now_t = SysTick_GetTick(); // 一次读取，所有任务共用
	// 轮询检查不同周期的函数是否需要执行
	for(index = 0;index < TASK_NUM;index++)
	{
		if((now_t - task_array[index].last_runtime) >= task_array[index].interval_ticks)
		{
			// 如果当前间隔数>=任务间隔数
			task_array[index].last_runtime = now_t; //更新该任务上次执行的时间
			task_array[index].task_func(); // 执行该任务
		}
	}
}


// 各种外设,驱动的初始化函数
void Hardware_init(void)
{
	My_usart_init(115200); //串口初始化
	TIM_it_init(); //定时器初始化
	Matrix_keyboard_init(); //矩阵键盘初始化
	LED_init(); //LED初始化
	Buzzer_init(); //蜂鸣器初始化
	LCD_Init(1); //LCD显示屏初始化
	Relay_init(); //继电器初始化
	AD_init(); //AD转换初始化
	Motor_init();//直流电机初始化
	STEPMOTOR_Init(); //步进电机初始化
	APP_init(); //开机动画
	
//	Send_printf("start=%d\r\n",SysTick_GetTick());
//	struct DHT11_data data = DHT11_read_data();
//	if(data.status == DHT11_DATA_OK)
//	{
//		Send_printf("end=%d\r\n,hum=%d",SysTick_GetTick(),data.humidity);
//	}
}








