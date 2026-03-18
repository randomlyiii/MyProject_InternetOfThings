#ifndef __JDY31_H
#define __JDY31_H
#include "stm32f10x.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//======================== 蓝牙名字和PIN码配置 ========================
#define JDY31_NAME "MyStm"
#define JDY31_BAUDRATE 9600
//=====================================================================
#define JDY31_USART USART2
#define JDY31_RX_BUF_SIZE 128

// // 服务器参数结构体
// typedef struct
// {
//     char server_ip[32];
//     uint16_t port;
//     uint16_t upload_interval;
// } Server_Config_t;

// 全局变量声明
extern uint8_t JDY31_RX_BUF[JDY31_RX_BUF_SIZE];
extern uint8_t JDY31_RX_CNT;
extern uint8_t JDY31_RX_FLAG;
extern uint8_t JDY31_Connected_Flag; // 蓝牙连接状态标志
// extern Server_Config_t ServerCfg;

// 函数声明
void JDY31_Init(void);
void JDY31_Send_String(char *str);
void JDY31_Set_Config(void);
void JDY31_Send_JSON_Data(uint8_t temp_i, uint8_t temp_d, uint8_t humi_i, uint8_t humi_d);
void JDY31_Parse_JSON(void);
uint8_t JDY31_Parse_TestOK(void);
void JDY31_Clear_RX_Buf(void);
void JDY31_Send_Config(void);

#endif
