#ifndef __MY_USART_H
#define __MY_USART_H
#include <stdio.h>
#include <stdint.h>

extern char Serial_RxPacket[100];				//缓存数据包
extern volatile uint8_t Serial_RxFlag;					//数据包读取标志位


void My_usart_init(uint32_t BaudRate);


// 发送一个字节
void Send_byte(uint8_t byte);

// 发送一个数组
void Send_array(uint8_t* array,uint16_t length);


// 发送一个字符串
void Send_string(char* str);

void Send_number(uint32_t Number, uint8_t Length);

// 自定义重定向printf函数
void Send_printf(const char* fm,...);



#endif






