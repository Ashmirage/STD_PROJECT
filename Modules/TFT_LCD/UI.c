#include "stm32f10x.h"
#include "Lcd.h"
#include "time.h"
#include "my_usart.h"
#include "Matrix_keyboard.h"
#include "Relay.h"
#include "LED.h"
#include "Buzzer.h"
#include "Temp.h"
#include "DHT11.h"
#include "LDR.h"
#include "Motor.h"
#include "LED.h"
#include "Stepmotor.h"
#include <string.h>

typedef enum{
	APP_SLEEP,  // 休眠状态
	APP_LOCKED, //未解锁状态
	APP_PRE_UNLOCKED, //输入密码的状态
	APP_UNLOCKED, //解锁状态
}APP_STATUS;
	
volatile static APP_STATUS app_status;
const char blank[] = "                          ";

// 24的字体大小,320的屏幕宽度,大约可以显示26个字母
const static char bootup_str_row1[] = "  Z   N   J   J   X   T   ";
const static char bootup_str_row2[] = "      0   2   0   8       ";
#define SHOW_LEN 26
#define TEXT_LEN 52

// 开机滚动动画
void Lcd_bootup_scrolling(void)
{
	u32 start = SysTick_GetTick();
//	Send_printf("start=%d\r\n",start);
	while((SysTick_GetTick() - start) <= 3000)// 限制三秒时间
	{
		int8_t right = 25,left = 24;
		while(left > 0)
		{
			LCD_Clear(WHITE);
			uint8_t i;
			for(i = left;i < right;i++)
			{
				LCD_ShowChar((i - left) * 13,100,bootup_str_row1[i],24,1); //依次打印字符
				LCD_ShowChar((i - left) * 13,300,bootup_str_row2[i],24,1); //依次打印字符
			}
			left--;
			Delay_ms(90); //稍微延迟一下,防止刷屏严重
		}	
//		Send_printf("end=%d\r\n",SysTick_GetTick());
//		while(right > left)
//		{
//			LCD_Clear(WHITE);
//			int8_t i;
//			for(i = left;i < right;i++)
//			{
//				LCD_ShowChar((25-right + i) * 13,100,bootup_str_row1[i],24,1);
//				LCD_ShowChar((25-right + i) * 13,300,bootup_str_row2[i],24,1); //依次打印字符
//			}
//			right--;
//			Delay_ms(50);
//		}
	}
}

void APP_init(void)
{
	Lcd_bootup_scrolling(); //开机滚动状态
	app_status = APP_LOCKED; //设置初始状态
	// 滚动结束后熄屏
	LCD_Clear(BLACK);
}

// 输入密码的结构体
typedef struct
{
	uint8_t word_count;
	uint8_t is_true;
}password_st;

const char default_password[] = "123456"; //默认密码
u32 error_t;


password_st Input_password(u16 ms)
{
	static char input[7]; //缓存输入的密码
	static uint8_t error_count = 0;
	static uint8_t count = 0;
	static uint8_t status = 0;
	static uint8_t alarm_flag = 0;
	static uint16_t time_cnt = 0;
	uint8_t num = Matrix_keyboard_get_number();
	static password_st password_structure;
	password_structure.is_true = 0; //默认失败
	switch(status)
	{
		case 0:
			if(num != 0 && num < 10)// 如果键盘有输入而且是0-9的数字
			{
				input[count] = num + '0';//存储字符串
				count++;
				if(count == 6) //是否已经输入六个数
				{
					input[count] = '\0'; //结束标识符
					if(strcmp(input,default_password) == 0)
					{
						error_count = 0; //清除错误次数
						LED_flash_on(500);
						status = 2;
						time_cnt = 0;
					}else //密码错误
					{
						status = 1;
						LCD_Clear(WHITE); //清屏
						error_t = SysTick_GetTick();
					}
				}
			}
		break;
		case 1: //密码错误
			if(alarm_flag == 0)
			{
				LCD_ShowString(0,200,320,24,24,"password error!"); //显示密码错误
			}
			if((SysTick_GetTick() - error_t) > 2000)
			{
				if(error_count >= 2)
				{
					if(alarm_flag == 0)
					{
						Buzzer_alarm_start();//打开蜂鸣器报警
						LCD_Clear(BLACK);//熄屏5s
						alarm_flag = 1;
					}
					if((SysTick_GetTick() - error_t) > 7000)
					{
						alarm_flag = 0; //清除标志位
						error_count = 0;
						count = 0;//重置状态
						status = 0;
						LCD_Clear(WHITE); //清屏
						LCD_ShowString(0,200,320,24,24,"please input password:");
					}
				}else
				{
					error_count++;//错误次数++
					count = 0;//重置状态
					status = 0;
					LCD_Clear(WHITE); //清屏
					LCD_ShowString(0,200,320,24,24,"please input password:");
				}
			}
		break;
		case 2://密码正确
			time_cnt++;
			if(time_cnt * ms >= 5000)
			{
				LED_flash_off();//5s自动关闭
				Relay_on(); //打开继电器
				status = 0;
				password_structure.is_true = 1;
			}
			break;
	}
	password_structure.word_count = count; //已经输入的长度
	return password_structure;
}

// 显示输入密码的函数
uint8_t APP_show_password_text(u16 ms)
{ 
	static uint8_t last_len = 0;
	password_st temp = Input_password(ms);
	uint8_t len = temp.word_count; //获取已经输入的密码长度
	if(last_len != len)
	{
		LCD_ShowString(0,250,320,24,24,blank);
	}
	for(uint8_t i = 0;i < len;i++)
	{
		LCD_ShowChar(i * 30,250,'*',24,1);
	}
	last_len = len;
	return temp.is_true;
}


static uint16_t temp = 0;
static uint16_t hum = 0;
static uint16_t light = 0;
static uint8_t motor_status = 0;
static uint8_t led_status = 0;

//定义控制模式,默认手动
static uint8_t control_mode = 0;


void APP(u16 ms)
{
	uint8_t matrix_keynum = 0;
	switch(app_status)
	{
		case APP_LOCKED: //未解锁状态
			matrix_keynum = Matrix_keyboard_get_number(); //读取键盘的输入
			if(matrix_keynum != 0) //检测是否又按键按下
			{
				LCD_Clear(WHITE); //屏幕点亮
				app_status = APP_PRE_UNLOCKED;
				LCD_ShowString(0,200,320,24,24,"please input password:");
			}
		break;
		case APP_PRE_UNLOCKED:
			if(APP_show_password_text(ms))
			{
				LCD_Clear(WHITE); //刷一下屏幕
//				LCD_ShowString(0,200,320,24,24,"password true!");
				app_status = APP_UNLOCKED;
			}
		break;
		case APP_UNLOCKED:
			
			break;
		case APP_SLEEP:
			break;
	}
}

// 更新数据,20ms执行一次
void APP_data_update(void)
{
	//处于解锁状态才更新数据
	if(app_status == APP_UNLOCKED)
	{
		temp = (uint16_t)Temp_get_t(); //更新温度
		hum = DHT11_getdata().humidity; //更新湿度
		light = LDR_LuxData(); //更新光照
		motor_status = Motor_get_status(); //更新直流电机状态
		led_status = LED_get_status();
	}
}

// 显示信息,2s执行一次
void APP_show_info(void)
{
	if(Stepmotor_is_run() == 0)// 如果步进电机正在运行,不要刷新,太耗时了
	{
		//处于解锁状态才更新数据
		if(app_status == APP_UNLOCKED)
		{
			LCD_Clear(WHITE); //清屏一下
			LCD_ShowString(0,24,160,24,24,"Temperature:");
			LCD_ShowNum(160,24,temp,3,24);
			LCD_ShowString(0,48,160,24,24,"Humidity:");
			LCD_ShowNum(160,48,hum,3,24);
			LCD_ShowString(0,72,160,24,24,"Lux:");
			LCD_ShowNum(160,72,light,3,24);
			LCD_ShowString(0,96,160,24,24,"Motor:");
			if(motor_status)
			{
				LCD_ShowString(160,96,160,24,24,"ON");
			}else
			{
				LCD_ShowString(160,96,160,24,24,"OFF");
			}
			LCD_ShowString(0,120,160,24,24,"Curtain");
	//		LCD_ShowNum(160,120,motor_status,3,24);
			LCD_ShowString(0,144,160,24,24,"LED:");
			if(led_status)
			{
				LCD_ShowString(160,144,160,24,24,"ON");
			}else
			{
				LCD_ShowString(160,144,160,24,24,"OFF");
			}
			LCD_ShowString(0,168,160,24,24,"Mode:");
			if(control_mode == 0)
			{
				LCD_ShowString(160,168,160,24,24,"Manual mode");
			}else
			{
				LCD_ShowString(160,168,160,24,24,"Auto mode");
			}
			LCD_ShowString(0,192,160,24,24,"Curtain:");
			LCD_ShowNum(160,192,(uint16_t)Stepmotor_get_curtain_status(),3,24);
			LCD_ShowString(200,192,160,24,24," %");
			//Stepmotor_get_curtain_status
		}
	}
}




// 手动控制的函数
void APP_control(void)
{
	//判断是否处于解锁状态,并且手动控制
	if(app_status == APP_UNLOCKED)
	{
		if(control_mode == 0)
		{
			uint8_t command = Matrix_keyboard_get_number(); //获取键盘输入值
			if(command != 0)
			{
				switch(command)
				{
					case 1: //开关直流电机
						if(Motor_get_status()){Motor_set_speed(0);}
						else{Motor_set_speed(100);}
						break;
					case 2:// 控制步进电机
						Stepmotor_angle_dir(0,90,1);
						break;
					case 3:
						Stepmotor_angle_dir(1,90,1);
						break;
					case 4:// LED控制
						LED_Toggle();
						break;
					case 5://切换模式
						control_mode = ~control_mode;
						break;
				}
			}
		}else
		{
			//自动模式
			if(light < 250)//光线暗时
			{
				LED_ON();//开灯
				if(Stepmotor_get_curtain_status() < 25)
				{Stepmotor_angle_dir(0,90,1);}
			}
			else if(light < 500)
			{
				LED_ON();//开灯
				if(Stepmotor_get_curtain_status() < 50)
				{Stepmotor_angle_dir(0,90,1);}
			}
			else if(light < 750)
			{
				LED_OFF();//关灯
				if(Stepmotor_get_curtain_status() < 75)
				{Stepmotor_angle_dir(0,90,1);}
			}
			else
			{
				LED_OFF();//关灯
				if(Stepmotor_get_curtain_status() < 100)
				{Stepmotor_angle_dir(0,90,1);}
			}
		}
	}
}

//void APP_auto_control(void)
//{
//	if(control_mode == 0)
//	{
//		
//	}
//}




