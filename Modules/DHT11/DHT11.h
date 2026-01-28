#ifndef DHT11_H
#define DHT11_H
#include <stdint.h>



enum DHT11_STATUS {
    DHT11_DATA_OK = 0,
    DHT11_DATA_FAILED = 1,
    DHT11_ACK_FAILED = 2
};

struct DHT11_data {
    enum DHT11_STATUS status; //状态
    int8_t temperature;
    int8_t humidity;
};

void DHT11_Init(void);

uint8_t DTH11_check(void);


//读取数据函数
struct DHT11_data DHT11_read_data(void);



#endif //DHT11_H
