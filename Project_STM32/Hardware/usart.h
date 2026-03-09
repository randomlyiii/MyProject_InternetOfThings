#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"
#include "delay.h"

// 串口接收缓冲区配置
#define USART1_RX_BUF_SIZE 256
extern uint8_t USART1_RX_BUF[USART1_RX_BUF_SIZE];
extern uint16_t USART1_RX_CNT;

// 函数声明
void USART1_Init(void);
void USART1_SendString(const char* str);

#endif
