#include "stm32f10x.h"
#include "usart.h"
#include "jdy31.h"
#include "delay.h"
#include "ESP8266.h"
#include "esp8266_config.h"
#include "OLED.h"
#include "dht11.h"
#include <stdio.h>
#include <string.h>

// WiFiAndServer_Config -> ESP8266_Config.h Esp8266_Config.c
// 调试用JSON数据
#define TEST_JSON "{\"temperature\": \"20\",\"humidity\": \"86\"}"

int main(void)
{
  uint8_t esp_statusesp_status = 0;
  char temperature[12];
  char humidity[12];
  uint8_t temp_i, temp_d, humi_i, humi_d;

  // 1. 外设初始化（顺序不可乱）
  Delay_ms(1000); // 上电延时
  // USART1_Init();      // 串口1初始化（ESP8266通信）
  JDY31_Init();       // 初始化蓝牙驱动
  JDY31_Set_Config(); // 自动配置蓝牙名字+PIN码
  DHT11_GPIO_Init();  // DHT11初始化（温湿度传感器）
  OLED_Init();
  OLED_Clear();
  OLED_ShowString(1, 1, "SYS InitAllRight");
  Delay_ms(1000);

  OLED_Clear();
  OLED_ShowString(1, 1, "BT Test...");
  Delay_ms(1000);

  // 新增：初始化后清屏，避免残留"BT Test..."
  OLED_Clear();
  OLED_ShowString(1, 1, "BT Waiting...");
  OLED_ShowString(2, 1, "RX: None");

  while (1)
  {
    // 发送温湿度JSON
    if (DHT11_Read_Data(&humi_i, &humi_d, &temp_i, &temp_d) == 0)
    {
      sprintf(temperature, "Temp:%d.%d`C", temp_i, temp_d);
      sprintf(humidity, "Humi:%d.%d%%", humi_i, humi_d);

      OLED_Clear();
      OLED_ShowString(1, 1, temperature);
      OLED_ShowString(2, 1, humidity);
    }
    JDY31_Send_JSON_Data(temp_i, temp_d, humi_i, humi_d);
    OLED_ShowString(4, 1, "SendToIphone");
    Delay_ms(1000);

    // 蓝牙接收数据处理
    if (JDY31_RX_FLAG)
    {
      OLED_Clear();
      OLED_ShowString(1, 1, "BT Recv OK!");
      Delay_ms(2000);

      // 解析JSON配置
      JDY31_Parse_JSON();

      // 显示解析结果
      OLED_Clear();
      OLED_ShowString(1, 1, "Parse OK!");
      OLED_ShowString(2, 1, "IP:");
      OLED_ShowString(2, 4, ServerCfg.server_ip);
      OLED_ShowString(3, 1, "Port:");
      OLED_ShowNum(3, 6, ServerCfg.port, 5);
      Delay_ms(2000);

      // 清空缓冲区，准备下次接收
      JDY31_Clear_RX_Buf();

      // // 回到等待状态
      // OLED_Clear();
      // OLED_ShowString(1, 1, "BT Waiting...");
      // OLED_ShowString(2, 1, "RX: None");
    }

    // // 显示蓝牙连接状态
    // if (JDY31_Connected_Flag)
    // {
    //   OLED_ShowString(3, 1, "BT Connected");
    // }
    // else
    // {
    //   OLED_ShowString(3, 1, "BT Disconnect");
    // }

    Delay_ms(100); // 降低CPU占用
  }

  // 以下为ESP8266相关代码（保留原逻辑，可按需启用）
  /*
  //     // ESP8266初始化（连接WiFi）
  //     esp_status = ESP8266_Init(WIFI_SSID, WIFI_PWD);

  //     // 初始化测试
  //     if (esp_status == 1)
  //     {
  //         OLED_ShowString(1, 1, "WiFi Connected!");
  //         OLED_ShowString(2, 1, "SSID:");
  //         OLED_ShowString(2, 6, WIFI_SSID);
  //         Delay_ms(2000);

  //         // 测试发送JSON数据
  //         OLED_ShowString(3, 1, "Send JSON...");
  //         if (ESP8266_SendTest(SERVER_IP, SERVER_PORT, TEST_JSON))
  //         {
  //             OLED_ShowString(4, 1, "Send OK!");
  //             Delay_ms(4000);
  //         }
  //         else
  //         {
  //             // 发送失败时，循环显示错误提示
  //             while (1)
  //             {
  //                 OLED_Clear();
  //                 OLED_ShowString(1, 1, "SendTest Failed!");
  //                 Delay_ms(2000);
  //             }
  //         }
  //     }
  //     else
  //     {
  //         OLED_Clear();
  //         OLED_ShowString(1, 1, "WiFi Init Failed");

  //         // 分步排查：显示每一步AT指令结果
  //         OLED_ShowString(2, 1, "AT Test:");
  //         OLED_ShowNum(2, 9, ESP8266_SendCmd("AT", "OK", 1000), 1);

  //         OLED_ShowString(3, 1, "RST Test:");
  //         OLED_ShowNum(3, 9, ESP8266_SendCmd("AT+RST", "ready", 3000), 1);

  //         OLED_ShowString(4, 1, "CWMODE:");
  //         OLED_ShowNum(4, 9, ESP8266_SendCmd("AT+CWMODE=1", "OK", 1000), 1);

  //         Delay_ms(4000);

  //         while (1)
  //         {
  //             OLED_Clear();
  //             OLED_ShowString(1, 1, "WiFi Init Failed");

  //             Delay_ms(2000);
  //         }
  //     }

  //     // 测试显示温湿度
  //     OLED_Clear();
  //     OLED_ShowString(1, 1, "dht11 test...");
  //     Delay_ms(2000);
  //     OLED_Clear();
  //     // 1. 读取DHT11温湿度数据（需确保dht11.h中定义了DHT11_Read_Data函数）
  //     if (DHT11_Read_Data(&humi_i, &humi_d, &temp_i, &temp_d) == 0)
  //     {
  //         // 2. 格式化温湿度字符串（带单位，便于阅读）
  //         sprintf(temperature, "Temp:%d.%d`C", temp_i, temp_d);
  //         sprintf(humidity, "Humi:%d.%d%%", humi_i, humi_d);

  //         // 3. OLED显示温湿度（第1行显示温度，第2行显示湿度）
  //         OLED_ShowString(1, 1, temperature);
  //         OLED_ShowString(2, 1, humidity);
  //     }
  //     else
  //     {
  //         // 读取失败时显示错误提示
  //         OLED_ShowString(1, 1, "DHT11 Error!");
  //         OLED_ShowString(2, 1, "Read Failed");
  //     }
  //     Delay_ms(2000);

  //     OLED_Clear();
  //     OLED_ShowString(1, 1, "Test Complete!");
  //     Delay_ms(2000);

  //     char json_buf[64];
  //     while (1)
  //     {
  //         OLED_Clear();
  //         if (DHT11_Read_Data(&humi_i, &humi_d, &temp_i, &temp_d) == 0)
  //         {
  //             sprintf(temperature, "Temp:%d.%d`C", temp_i, temp_d);
  //             sprintf(humidity, "Humi:%d.%d%%", humi_i, humi_d);
  //             sprintf(json_buf, "{\"temperature\":\"%d.%d\",\"humidity\":\"%d.%d\"}",
  //                     temp_i, temp_d,  // 温度：整数.小数
  //                     humi_i, humi_d); // 湿度：整数.小数

  //             // 发送JSON数据到服务器
  //             if (!ESP8266_SendJSON(SERVER_IP, SERVER_PORT, json_buf))
  //             {
  //                 OLED_Clear();
  //                 OLED_ShowString(1, 1, "Send Failed");
  //                 Delay_ms(2000);
  //                 break;
  //             }
  //             OLED_ShowString(1, 1, temperature);
  //             OLED_ShowString(2, 1, humidity);
  //         }
  //         else
  //         {
  //             OLED_ShowString(1, 1, "DHT11 Error!");
  //             OLED_ShowString(2, 1, "Read Failed");
  //         }

  //         Delay_ms(2000);
  //     }

  //     OLED_Clear();
  //     OLED_ShowString(1, 1, "Task Stopped");
  */
}
