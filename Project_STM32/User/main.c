#include "stm32f10x.h"
#include "usart.h"
#include "delay.h"
#include "ESP8266.h"
#include "esp8266_config.h"
#include "OLED.h"
#include "dht11.h"
#include <stdio.h>
#include <string.h>

// 调试用JSON数据
#define TEST_JSON "{\"temperature\": \"55\",\"humidity\": \"8\"}"

int main(void)
{
    uint8_t esp_status = 0;
    char temperature[12];
    char humidity[12];
    uint8_t temp_i, temp_d, humi_i, humi_d;

    // 1. 外设初始化（顺序不可乱）
    Delay_ms(1000);    // 上电延时
    USART1_Init();     // 串口1初始化（ESP8266通信）
    DHT11_GPIO_Init(); // DHT11初始化（温湿度传感器）
    OLED_Init();       // OLED显示（调试用）
    OLED_Clear();

    // // 2. 显示初始化提示
    // OLED_ShowString(1, 1, "ESP8266 Init...");
    // Delay_ms(500);

    // // 3. ESP8266初始化（连接WiFi）
    // esp_status = ESP8266_Init(WIFI_SSID, WIFI_PWD);

    // // 4. 显示初始化结果
    // if (esp_status == 1)
    // {
    //     OLED_ShowString(1, 1, "WiFi Connected!");
    //     OLED_ShowString(2, 1, "SSID:");
    //     OLED_ShowString(2, 6, WIFI_SSID);
    // 		  Delay_ms(2000);

    //     //测试发送JSON数据
    //     OLED_ShowString(3, 1, "Send JSON...");
    //     if (ESP8266_SendJSON(SERVER_IP, SERVER_PORT, TEST_JSON))
    //     {
    //         OLED_ShowString(4, 1, "Send OK!");
    //     }
    //     else
    //     {
    // 					while(1){} //
    // 				  OLED_Clear();
    // 				  OLED_ShowString(1, 1, "Fail");
    // 				  OLED_ShowString(2, 1, "a");
    //     }
    // }
    // else
    // {
    //     OLED_ShowString(1, 1, "Init Failed!");
    //     // 分步排查：显示每一步AT指令结果
    //     OLED_ShowString(2, 1, "AT Test:");
    //     OLED_ShowNum(2, 9, ESP8266_SendCmd("AT", "OK", 1000), 1);

    //     OLED_ShowString(3, 1, "RST Test:");
    //     OLED_ShowNum(3, 9, ESP8266_SendCmd("AT+RST", "ready", 3000), 1);

    //     OLED_ShowString(4, 1, "CWMODE:");
    //     OLED_ShowNum(4, 9, ESP8266_SendCmd("AT+CWMODE=1", "OK", 1000), 1);
    // }

    while (1)
    {
        // ========== 新增/修改：DHT11读取+OLED显示逻辑 ==========
        OLED_Clear(); // 清屏

        // 1. 读取DHT11温湿度数据（需确保dht11.h中定义了DHT11_Read_Data函数）
        if (DHT11_Read_Data(&humi_i, &humi_d, &temp_i, &temp_d) == 0)
        {
            // 2. 格式化温湿度字符串（带单位，便于阅读）
					  sprintf(temperature,"Temp:%d.%d`C", temp_i, temp_d);
            sprintf(humidity,"Humi:%d.%d%%", humi_i, humi_d);

            // 3. OLED显示温湿度（第1行显示温度，第2行显示湿度）
            OLED_ShowString(1, 1, temperature);
            OLED_ShowString(2, 1, humidity);
        }
        else
        {
            // 读取失败时显示错误提示
            OLED_ShowString(1, 1, "DHT11 Error!");
            OLED_ShowString(2, 1, "Read Failed");
        }
        // ========== DHT11+OLED逻辑结束 ==========

        Delay_ms(2000); // 2秒刷新一次
    }
}
