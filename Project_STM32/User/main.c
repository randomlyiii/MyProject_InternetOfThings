#include "stm32f10x.h"
#include "usart.h"
#include "delay.h"
#include "ESP8266.h"
#include "esp8266_config.h"
#include "OLED.h"
#include <stdio.h>

// 调试用JSON数据
#define TEST_JSON "{\"temperature\": \"55\",\"humidity\": \"8\"}"

int main(void)
{
    uint8_t esp_status = 0;

    // 1. 外设初始化（顺序不可乱）
    Delay_ms(100); // 上电延时
    USART1_Init(); // 串口1初始化（ESP8266通信）
    OLED_Init();   // OLED显示（调试用）
    OLED_Clear();

    // 2. 显示初始化提示
    OLED_ShowString(1, 1, "ESP8266 Init...");
    Delay_ms(500);

    // 3. ESP8266初始化（连接WiFi）
    esp_status = ESP8266_Init(WIFI_SSID, WIFI_PWD);

    // 4. 显示初始化结果
    if (esp_status == 1)
    {
        OLED_ShowString(1, 1, "WiFi Connected!");
        OLED_ShowString(2, 1, "SSID:");
        OLED_ShowString(2, 6, WIFI_SSID);
			  Delay_ms(2000);

        //测试发送JSON数据
        OLED_ShowString(3, 1, "Send JSON...");
        if (ESP8266_SendJSON(SERVER_IP, SERVER_PORT, TEST_JSON))
        {
            OLED_ShowString(4, 1, "Send OK!");
        }
        else
        {
						while(1){} //
					  OLED_Clear();
					  OLED_ShowString(1, 1, "Fail");
					  OLED_ShowString(2, 1, "a");
        }
    }
    else
    {
        OLED_ShowString(1, 1, "Init Failed!");
        // 分步排查：显示每一步AT指令结果
        OLED_ShowString(2, 1, "AT Test:");
        OLED_ShowNum(2, 9, ESP8266_SendCmd("AT", "OK", 1000), 1);

        OLED_ShowString(3, 1, "RST Test:");
        OLED_ShowNum(3, 9, ESP8266_SendCmd("AT+RST", "ready", 3000), 1);

        OLED_ShowString(4, 1, "CWMODE:");
        OLED_ShowNum(4, 9, ESP8266_SendCmd("AT+CWMODE=1", "OK", 1000), 1);
    }

    while (1)
    {
        // 主循环
        Delay_ms(1000);
    }
}
