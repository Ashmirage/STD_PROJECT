#include "stm32f10x.h"
#include "time.h"
#include "RTC_clk.h"

volatile uint16_t My_RTC_time[] = {2026,2,7,14,25,29};

void RTC_clk_init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP,ENABLE); //使能BKP和PWR的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
	
	PWR_BackupAccessCmd(ENABLE);//使能访问控制
	
	RCC_LSEConfig(RCC_LSE_ON); //启动外部低速晶振
	
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) != SET); // 等晶振准备好
	
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE); //给RTC选择LSE时钟源
	RCC_RTCCLKCmd(ENABLE); //使能时钟
	
	//系统彻底掉电才会设置时间
	if(BKP_ReadBackupRegister(BKP_DR1) == 0)
	{
		RTC_WaitForSynchro(); //等待时钟同步
		RTC_WaitForLastTask(); //等待上一次操作完成
		RTC_SetPrescaler(32768 - 1); //分频为1hz
		RTC_WaitForLastTask(); //等待上一次操作完成
		
		RTC_SetCounter(1672588795); //设置初始时间
		RTC_WaitForLastTask(); //等待一下
		My_RTC_settime();
	}
}

void My_RTC_settime(void)
{
	time_t time_cnt;
	struct tm time_date;
	
	time_date.tm_year = My_RTC_time[0] - 1900;
	time_date.tm_mon = My_RTC_time[1] - 1;
	time_date.tm_mday = My_RTC_time[2];
	time_date.tm_hour = My_RTC_time[3];
	time_date.tm_min = My_RTC_time[4];
	time_date.tm_sec = My_RTC_time[5];
	
	time_cnt = mktime(&time_date);
	RTC_SetCounter(time_cnt); //设置时间
	RTC_WaitForLastTask(); //等待完成
	
}

void My_RTC_readtime(void)
{
	time_t time_cnt = RTC_GetCounter(); //读取时间
	struct tm time_date;
	
	time_date = *localtime(&time_cnt);
	
	My_RTC_time[0] = time_date.tm_year + 1900;
	My_RTC_time[1] = time_date.tm_mon + 1;
	My_RTC_time[2] = time_date.tm_mday;
	My_RTC_time[3] = time_date.tm_hour;
	My_RTC_time[4] = time_date.tm_min;
	My_RTC_time[5] = time_date.tm_sec;
	
}


