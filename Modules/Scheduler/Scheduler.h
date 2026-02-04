#ifndef __SCHEDULER_H
#define __SCHEDULER_H		

// 裸机任务调度器初始化
void Scheduler_init(void);
// 主循环里面不断循环执行这个函数
void Scheduler_run(void);

// 各种外设,驱动的初始化函数
void Hardware_init(void);






#endif  
	 
	 



