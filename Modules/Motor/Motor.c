#include "stm32f10x.h"
#include <stdlib.h>
#include "PWM.h"
#include "time.h"

#define MOTOR_AIN_PORT GPIOC
#define MOTOR_AIN1_PIN GPIO_Pin_13
#define MOTOR_AIN2_PIN GPIO_Pin_7
#define MOTOR_AIN_CLK_FUNCTION() RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);

//注意,直流电机不要大幅度改变方向,否则迅速发热且无法突破扭矩(也可能是电机本身质量问题)
//电压尽量高一点,5V很勉强
//占空比频率不能太低,否则会有明显的噪音


static uint8_t motor_status = 0;

// 获取直流电机的状态
uint8_t Motor_get_status(void)
{
	return motor_status;
}

static void Motor_coast_stop(void)
{
    // 先关PWM再改方向，减少毛刺/瞬态电流
    PWM_set(0);
    GPIO_WriteBit(MOTOR_AIN_PORT, MOTOR_AIN1_PIN, Bit_RESET);
    GPIO_WriteBit(MOTOR_AIN_PORT, MOTOR_AIN2_PIN, Bit_RESET); // IN1=0 IN2=0 => 停止(滑行)
	motor_status = 0;
}

/**
* @brief  直流电机初始化
 *
 * @param[in]  None
 *
 * @return None
 *
 */
void Motor_init(void)
{
	MOTOR_AIN_CLK_FUNCTION();//开启控制端时钟
	
	GPIO_InitTypeDef GPIO_init_structure;
	GPIO_init_structure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_init_structure.GPIO_Pin = MOTOR_AIN1_PIN | MOTOR_AIN2_PIN;
	GPIO_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(MOTOR_AIN_PORT,&GPIO_init_structure);
	
	PWM_init(); //PWM输出比较初始化

	// 上电给一个确定的安全状态
    Motor_coast_stop();
}


/**
* @brief  设置电机驱动力度 -100--+100
 *
 * @param[in]  speed -100-+100
 *
 * @return None
 *
 */
void Motor_set_speed(int8_t speed)
{
	//对输入进行限幅操作
	if(speed > 100)
	{
		speed = 100;
	}
	else if(speed < -100)
	{
		speed = -100;
	}
	// 设置转动方向
	// 0 速度：明确停止，避免“0也给反向方向”
    if (speed == 0)
    {
        Motor_coast_stop();
        return;
    }
	// 方向变化处理：先停再换向，避免反接电流冲击
    static int8_t last_dir = 0;               // -1:反向, +1:正向, 0:未知/停止
    int8_t dir = (speed > 0) ? +1 : -1;

    if (last_dir != 0 && dir != last_dir)
    {
        Motor_coast_stop();
    }

    // 设置方向
    if (dir > 0)
    {
        GPIO_WriteBit(MOTOR_AIN_PORT, MOTOR_AIN1_PIN, Bit_SET);
        GPIO_WriteBit(MOTOR_AIN_PORT, MOTOR_AIN2_PIN, Bit_RESET);
    }
    else
    {
        GPIO_WriteBit(MOTOR_AIN_PORT, MOTOR_AIN1_PIN, Bit_RESET);
        GPIO_WriteBit(MOTOR_AIN_PORT, MOTOR_AIN2_PIN, Bit_SET);
    }

    // 设置PWM（钳位交给PWM_set了）
    PWM_set((uint8_t)abs(speed));
	motor_status = 1;
    last_dir = dir;
}




