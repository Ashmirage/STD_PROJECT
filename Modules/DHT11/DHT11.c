#include "stm32f10x.h"
#include "my_time.h"
#include "DHT11.h"
#include "my_usart.h"
#include "Stepmotor.h"

#define DHT11_GPIO_CLK_FUNCTION()  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
#define DHT11_GPIO_PORT GPIOC
#define DHT11_GPIO_PIN GPIO_Pin_6

#define DHT11_W_SDA(x) GPIO_WriteBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN,(BitAction)x)
#define DHT11_R_SDA()  GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)

// 注意DHT11采样不能够太频繁,至少间隔1/2s采样一次
// 注意不要再DHT11里面加入任何耗时的串口打印操作, 极有可能影响时序, DHT11超绝时序敏感肌!

// 使用任务调度器时,不能将其放入调度函数中,因为DHT11的时序不可被中断,只能暂时关闭任务调度器

//切换模式的函数
void DHT11_mode(uint8_t mode) {
	DHT11_GPIO_CLK_FUNCTION();
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = DHT11_GPIO_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    if (mode) {
        //输出模式
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    } else {
        //输入模式
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //上拉模式
    }
    GPIO_Init(DHT11_GPIO_PORT,&GPIO_InitStructure);
}


//主机发起读取数据的请求
void DHT11_Init(void) {
    DHT11_mode(1); //切换为输出模式
    DHT11_W_SDA(0); //拉低总线
    Delay_ms(20); //保持至少18ms
    DHT11_W_SDA(1); //拉高总线
    Delay_us(30); //拉高20-40us
    DHT11_mode(0); //切换为输入模式
}


//检测从机是否应答
uint8_t DTH11_check(void) {
	uint32_t start = GetSysTime_us(); //获取微秒时间戳
    while (DHT11_R_SDA() == 1) {
        if ((GetSysTime_us() - start) > 150) {
            return 0; //超时响应失败
        }
    }
	start = GetSysTime_us() ;
    while (DHT11_R_SDA() == 0) {
        if ((GetSysTime_us() - start) > 150) {
            return 0;
        }
    }
    return 1; //响应成功
}

//读取一个比特位
uint8_t DHT11_read_bit(void) {	
	uint32_t start = GetSysTime_us(); 
    while (DHT11_R_SDA() == 1) {
        //等待拉低
        if ((GetSysTime_us() - start) > 150) {
            return 0;
        }
    }
    start = GetSysTime_us(); 
    while (DHT11_R_SDA() == 0) {
        //等待拉高
        if ((GetSysTime_us() - start) > 150) {
            return 0;
        }
    }
	start = GetSysTime_us(); 
	while(DHT11_R_SDA() == 1)
	{
		if((GetSysTime_us() - start) > 40)
		{
			return 1;
		}
	}	
    return 0; //读取数据线状态
}

//读取一个字节
//DHT11数据格式为8位数据，低位在前，高位在后
uint8_t DHT11_read_byte(void) {
    uint8_t i = 0;
    for (uint8_t j = 0; j < 8; j++) {
        i = i<<1;
        i |= DHT11_read_bit(); //读取8个比特位
    }
    return i;
}


/**
 * @brief  读取DHT11传感器数据
 * @param  无参数
 * @retval 结构体,包含数据读取结果,以及温湿度数据
 * @date 2025.8.14
 */
struct DHT11_data DHT11_read_data(void) {
	struct DHT11_data dht11_data;
    uint8_t data[5] = {0},sum = 0; //数据缓冲区
    DHT11_Init();//初始化DHT11
    if (DTH11_check()) {//等待应答
        for (uint8_t i = 0;i < 5;i++) {
            data[i] = DHT11_read_byte(); //读取5个字节
        }
        sum=data[0]+data[1]+data[2]+data[3]; //计算校验和
        if (sum == data[4]) {
            dht11_data.status = DHT11_DATA_OK;
            dht11_data.humidity = (int8_t)data[0];
            dht11_data.temperature = (int8_t)data[2];
            return dht11_data;
        }else {
            dht11_data.status = DHT11_DATA_FAILED;
            return dht11_data;
        }
    }else {
        dht11_data.status = DHT11_ACK_FAILED;
        return dht11_data;
    }
}

volatile struct DHT11_data dht11_data;

// 任务调度器里面1s执行一次
void DHT11_update_data(void)
{
	// 当步进电机运行时, 直接结束
	if(Stepmotor_is_run() == 1)
	{
		return;
	}
	dht11_data = DHT11_read_data(); //读取一次数据
}

struct DHT11_data DHT11_getdata(void)
{
	return dht11_data;
}







