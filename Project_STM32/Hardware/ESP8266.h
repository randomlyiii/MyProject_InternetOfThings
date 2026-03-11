#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f10x.h"
#include "usart.h"

uint8_t ESP8266_SendCmd(const char *cmd, const char *ack, uint32_t timeout);
uint8_t ESP8266_Init(const char *ssid, const char *password);
uint8_t ESP8266_SendTest(const char *ip, uint16_t port, const char *json);
uint8_t ESP8266_SendJSON(const char *ip, uint16_t port, const char *json);

#endif
