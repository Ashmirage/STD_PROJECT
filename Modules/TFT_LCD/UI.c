#include "stm32f10x.h"
#include "Lcd.h"
#include "my_time.h"
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
#include "W25Q128.h"
#include "RTC_clk.h"
#include <string.h>

typedef enum{
	APP_CHANGE_PASSWORD, //修改密码状态
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

static char default_password[7]; //默认密码

void APP_init(void)
{
	Lcd_bootup_scrolling(); //开机滚动状态
	app_status = APP_LOCKED; //设置初始状态
	//从flash中读取密码
	W25Q128_ReadData(PASSWORD_ADDRESS,default_password,7); //
	// 滚动结束后熄屏
	LCD_Clear(BLACK);
	//Send_printf("pd=%c%c%c%c%c%c",default_password[0],default_password[1],default_password[2],default_password[3],default_password[4],default_password[5]);
}

// 输入密码的结构体
typedef struct
{
	uint8_t word_count;
	uint8_t is_true;
}password_st;


u32 error_t;

const ChineseIndex_16x16 shurumima[] = {
        ZH_SHU,ZH_RU,ZH_MI,ZH_MA,ZH_COUNT
};
const ChineseIndex_16x16 mimacuowu[] = {
		ZH_MI,ZH_MA,ZH_CUO,ZH_WU,ZH_COUNT
};

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
				LCD_ShowChineseString24(0,200,mimacuowu,0); //显示密码错误
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
						LCD_ShowChineseString24(0,200,mimacuowu,0);
					}
				}else
				{
					error_count++;//错误次数++
					count = 0;//重置状态
					status = 0;
					LCD_Clear(WHITE); //清屏
					LCD_ShowChineseString24(0,200,shurumima,0);
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


password_st APP_change_password(void)
{
	static char input[7]; //缓存输入的密码
	static uint8_t count = 0;
	uint8_t num = Matrix_keyboard_get_number();
	static password_st password_structure;
	password_structure.is_true = 0; //默认失败
	if(num != 0 && num < 10)// 如果键盘有输入而且是0-9的数字
	{
		input[count] = num + '0';//存储字符串
		count++;
		if(count == 6) //是否已经输入六个数
		{
			input[count] = '\0'; //结束标识符
			password_structure.is_true = 1;
			for(uint8_t i = 0;i < 7;i++)
			{
				default_password[i] = input[i]; // 修改原密码
			}
			W25Q128_SectorErase(PASSWORD_ADDRESS); //擦除
			W25Q128_PageProgram(PASSWORD_ADDRESS,default_password,7); //写入flash,掉电不丢失
			count = 0; //清零count,以便下次修改
		}
	}
	password_structure.word_count = count; //已经输入的长度
	return password_structure;
}

// 显示修改密码的函数
uint8_t APP_change_password_text(u16 ms)
{ 
	static uint8_t last_len = 0;
	password_st temp = APP_change_password();
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
				LCD_ShowChineseString24(0,200,shurumima,0);
			}
		break;
		case APP_PRE_UNLOCKED:
			if(APP_show_password_text(ms))
			{
				LCD_Clear(WHITE); //刷一下屏幕
//				LCD_ShowString(0,200,320,24,24,"password true!");
				LED_OFF();
				app_status = APP_UNLOCKED;
			}
		break;
		case APP_UNLOCKED:
			
			break;
		case APP_CHANGE_PASSWORD:
			if(APP_change_password_text(ms))
			{
				LCD_Clear(WHITE); //刷一下屏幕
				app_status = APP_UNLOCKED;
			}
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

const ChineseIndex_16x16 wendu[] = {
		ZH_WEN,ZH_DU,ZH_COUNT
};



const ChineseIndex_16x16 shidu[] = {
		ZH_SHI,ZH_DU,ZH_COUNT
};

const ChineseIndex_16x16 guangzhao[] = {
		ZH_GUANG,ZH_ZHAO,ZH_COUNT
};

const ChineseIndex_16x16 fengshan[] = {
		ZH_FENG,ZH_SHAN,ZH_COUNT
};

const ChineseIndex_16x16 chuanglian[] = {
		ZH_CHUANG,ZH_LIANG,ZH_COUNT
};

const ChineseIndex_16x16 moshi[] = {
		ZH_MO,ZH_SHI2,ZH_COUNT
};

const ChineseIndex_16x16 shijian[] = {
		ZH_SHI3,ZH_SHI4,ZH_SHI4,ZH_JIAN,ZH_COUNT
};

char show_time_str[50];

// 显示信息,2s执行一次
void APP_show_info(void)
{
	if(Stepmotor_is_run() == 0)// 如果步进电机正在运行,不要刷新,太耗时了
	{
		//处于解锁状态才更新数据
		if(app_status == APP_UNLOCKED)
		{
			LCD_Clear(WHITE); //清屏一下
			LCD_ShowChineseString24(0,24,wendu,0);
			LCD_ShowNum(160,24,temp,3,24);
			LCD_ShowString(250,24,70,24,24,"'C");
			LCD_ShowChineseString24(0,48,shidu,0);
			LCD_ShowNum(160,48,hum,3,24);
			LCD_ShowString(250,48,70,24,24," %");
			LCD_ShowChineseString24(0,72,guangzhao,0);
			LCD_ShowNum(160,72,light,3,24);
			LCD_ShowString(250,72,70,24,24,"Lux");
			LCD_ShowChineseString24(0,96,fengshan,0);
			if(motor_status)
			{
				LCD_ShowString(160,96,160,24,24,"ON");
			}else
			{
				LCD_ShowString(160,96,160,24,24,"OFF");
			}
//			LCD_ShowString(0,120,160,24,24,"Curtain");
	//		LCD_ShowNum(160,120,motor_status,3,24);
			LCD_ShowString(0,144,160,24,24,"LED:");
			if(led_status)
			{
				LCD_ShowString(160,144,160,24,24,"ON");
			}else
			{
				LCD_ShowString(160,144,160,24,24,"OFF");
			}
			LCD_ShowChineseString24(0,168,moshi,0);
			if(control_mode == 0)
			{
				LCD_ShowString(160,168,160,24,24,"Manual mode");
			}else
			{
				LCD_ShowString(160,168,160,24,24,"Auto mode");
			}
			LCD_ShowChineseString24(0,192,chuanglian,0);
			LCD_ShowNum(160,192,(uint16_t)Stepmotor_get_curtain_status(),3,24);
			LCD_ShowString(200,192,160,24,24," %");
			// 显示时间
			LCD_ShowChineseString24(0,216,shijian,0);
			sprintf(show_time_str,"%d-%d-%d %d:%d:%d",My_RTC_time[0],My_RTC_time[1],My_RTC_time[2],My_RTC_time[3],My_RTC_time[4],My_RTC_time[5]);
			LCD_ShowString(120,216,200,24,24,show_time_str);
		}
	}
}




// 控制的函数
void APP_control(void)
{
	//判断是否处于解锁状态,并且手动控制
	if(app_status == APP_UNLOCKED)
	{
		uint8_t command = Matrix_keyboard_get_number(); //获取键盘输入值
		if(command == 5)
		{
			control_mode = ~control_mode; //切换模式
			return;
		}
		if(command == 6) // 修改密码
		{
			app_status = APP_CHANGE_PASSWORD; //切换状态
			LCD_Clear(WHITE); //清屏
			LCD_ShowString(0,200,320,24,24,"please input password:");
			return;
		}
		if(control_mode == 0)
		{
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
				}
			}
		}else
		{
			//自动模式
			if(light < 250)//光线暗时
			{
				LED_ON();//开灯
			}
			else if(light < 500)
			{
				LED_ON();//开灯
			}
			else if(light < 750)
			{
				LED_OFF();//关灯
			}
			else
			{
				LED_OFF();//关灯
			}
			if(temp > 25)
			{
				Motor_set_speed(100); //打开风扇
			}else
			{
				Motor_set_speed(0); //关闭风扇
			}
			//Send_printf("curtain=%d\r\n",Stepmotor_get_curtain_status());
			Stepmotor_light_control(light);
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




