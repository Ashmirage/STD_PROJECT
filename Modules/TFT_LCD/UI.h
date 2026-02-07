#ifndef __UI_H
#define __UI_H		
#include "stm32f10x.h"

void Lcd_bootup_scrolling(void);//滚动动画
void APP_init(void); //UI初始化

void APP(u16 ms); //APP状态机

// 更新数据,20ms执行一次
void APP_data_update(void);

// 显示信息,2s执行一次
void APP_show_info(void);

// 手动控制的函数
void APP_control(void);


#endif  
	 
	 



