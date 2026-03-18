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

static char last_WIFI_SSID[32] = {0};
static char last_WIFI_PWD[64] = {0};
static char last_SERVER_IP[32] = {0};
static uint16_t last_SERVER_PORT = 0;
// 配置变更检测函数
uint8_t CheckConfigChanged_WiFi(void)
{
  // 检测WiFi变更
  if (strcmp(last_WIFI_SSID, WIFI_SSID) != 0 || strcmp(last_WIFI_PWD, WIFI_PWD) != 0)
  {
    strcpy(last_WIFI_SSID, WIFI_SSID);
    strcpy(last_WIFI_PWD, WIFI_PWD);
    return 1;
  }
  return 0;
}

uint8_t CheckConfigChanged_Server(void)
{
  if (strcmp(last_SERVER_IP, SERVER_IP) != 0 || last_SERVER_PORT != SERVER_PORT)
  {
    strcpy(last_SERVER_IP, SERVER_IP);
    last_SERVER_PORT = SERVER_PORT;
    return 1;
  }
  return 0;
}

int main(void)
{
  uint8_t esp_status = 0;
  uint8_t server_status = 0;
  char temperature[12];
  char humidity[12];
  uint8_t temp_i, temp_d, humi_i, humi_d;

  // 1. 外设初始化（顺序不可乱）
  Delay_ms(1000);     // 上电延时
  USART1_Init();      // 串口1初始化（ESP8266通信）
  JDY31_Init();       // 初始化蓝牙驱动
  JDY31_Set_Config(); // 自动配置蓝牙名字+PIN码
  DHT11_GPIO_Init();  // DHT11初始化（温湿度传感器）
  OLED_Init();
  OLED_Clear();
  OLED_ShowString(1, 1, "SYS InitAllRight");
  Delay_ms(1000);

  // 初始化测试：持续测试蓝牙连接
  while (1)
  {
    OLED_Clear();
    OLED_ShowString(1, 1, "BTSendTest...");
    OLED_ShowString(2, 1, "temp:20.0`C");
    OLED_ShowString(3, 1, "humi:86.0%");
    JDY31_Send_JSON_Data(20, 0, 86, 0);
    OLED_ShowString(4, 1, "SendToIphone");
    Delay_ms(1000);

    OLED_Clear();
    OLED_ShowString(1, 1, "BT Connected");
    OLED_ShowString(2, 1, "PleaseSend:");
    OLED_ShowString(3, 1, "{\" BT \": \" OK \" }");
    OLED_ShowString(4, 1, "Receiving...");
    Delay_ms(3000);

    if (JDY31_RX_FLAG)
    {
      OLED_Clear();
      OLED_ShowString(1, 1, "BT Recv OK!");
      OLED_ShowString(2, 1, "BT Check...");
      Delay_ms(1000);

      if (JDY31_Parse_TestOK())
      {
        OLED_Clear();
        OLED_ShowString(1, 1, "BTTest OK!");
        Delay_ms(1000);
        break;
      }
      else
      {
        OLED_Clear();
        OLED_ShowString(1, 1, "BT CheckFailed!");
        Delay_ms(1000);
      }

      // 清空缓冲区，准备下次接收
      JDY31_Clear_RX_Buf();
    }

    Delay_ms(100); // 降低CPU占用
  }

  // 测试显示温湿度
  OLED_Clear();
  OLED_ShowString(1, 1, "dht11 test...");
  Delay_ms(2000);
  OLED_Clear();
  // 读取DHT11温湿度数据
  if (DHT11_Read_Data(&humi_i, &humi_d, &temp_i, &temp_d) == 0)
  {
    sprintf(temperature, "Temp:%d.%d`C", temp_i, temp_d);
    sprintf(humidity, "Humi:%d.%d%%", humi_i, humi_d);

    OLED_ShowString(1, 1, temperature);
    OLED_ShowString(2, 1, humidity);
  }
  else
  {
    // 读取失败时显示错误提示
    while (1)
    {
      OLED_Clear();
      OLED_ShowString(1, 1, "DHT11 Error!");
      OLED_ShowString(2, 1, "Read Failed");
      Delay_ms(2000);
    }
  }
  Delay_ms(2000);

  OLED_Clear();
  OLED_ShowString(1, 1, "Test Complete!");
  OLED_ShowString(2, 1, "Starting...");
  Delay_ms(4000);

  // 初始化配置
  strcpy(last_WIFI_SSID, WIFI_SSID);
  strcpy(last_WIFI_PWD, WIFI_PWD);
  strcpy(last_SERVER_IP, SERVER_IP);
  last_SERVER_PORT = SERVER_PORT;
  UPLOAD_INTERVAL = 2000; // 默认上传间隔2秒

  char json_buf[64];
  while (1)
  {
    // BlueTooth
    if (JDY31_RX_FLAG)
    {
      OLED_Clear();
      OLED_ShowString(1, 1, "BT Recv OK!");
      Delay_ms(1000);

      JDY31_Parse_JSON();

      if (CheckConfigChanged_WiFi())
      {
        esp_status = 0; // 重置WiFi状态机
      }

      if (CheckConfigChanged_Server())
      {
        server_status = 0; // 重置服务器状态机
      }

      OLED_Clear();
      OLED_ShowString(1, 1, SERVER_IP);
      OLED_ShowString(2, 1, "Port:");
      OLED_ShowNum(2, 6, SERVER_PORT, 5);
      OLED_ShowString(3, 1, "Interval:");
      OLED_ShowNum(3, 10, UPLOAD_INTERVAL, 4);
      OLED_ShowString(4, 1, "WiFi:");
      OLED_ShowString(4, 6, WIFI_SSID);
      Delay_ms(1000);

      JDY31_Clear_RX_Buf();
    }

    // WiFi
    if (esp_status == 0)
    {
      OLED_Clear();
      OLED_ShowString(1, 1, "Current WiFi:");
      OLED_ShowString(2, 1, WIFI_SSID);
      OLED_ShowString(3, 1, "Connecting...");

      esp_status = ESP8266_Init(WIFI_SSID, WIFI_PWD);

      if (esp_status == 1)
      {
        OLED_Clear();
        OLED_ShowString(1, 1, "WiFi Connected!");
        OLED_ShowString(2, 1, "SSID:");
        OLED_ShowString(2, 6, WIFI_SSID);
        Delay_ms(1000);

        server_status = 0; // 重置服务器状态机
      }
      else
      {
        OLED_Clear();
        OLED_ShowString(1, 1, "WiFi ConnectFail!");
        // 显示诊断信息
        // 分步排查：显示每一步AT指令结果
        OLED_ShowString(2, 1, "AT Test:");
        OLED_ShowNum(2, 9, ESP8266_SendCmd("AT", "OK", 1000), 1);

        OLED_ShowString(3, 1, "RST Test:");
        OLED_ShowNum(3, 9, ESP8266_SendCmd("AT+RST", "ready", 3000), 1);

        OLED_ShowString(4, 1, "CWMODE:");
        OLED_ShowNum(4, 9, ESP8266_SendCmd("AT+CWMODE=1", "OK", 1000), 1);

        Delay_ms(3000);
      }
    }
    else if (server_status == 0)
    {
      // 测试服务器连接
      OLED_Clear();
      OLED_ShowString(1, 1, "Testing Server...");
      if (ESP8266_SendTest(SERVER_IP, SERVER_PORT, TEST_JSON))
      {
        OLED_ShowString(4, 1, "Server OK!");
        server_status = 1;
      }
      else
      {
        OLED_ShowString(4, 1, "Server Test Fail");
        server_status = 0;
      }
      Delay_ms(1000);

      OLED_Clear();
      OLED_ShowString(1, 1, "IP Unreachable!");
      OLED_ShowString(2, 1, "Check Config:");
      OLED_ShowString(3, 1, SERVER_IP);
      OLED_ShowString(4, 1, "Port:");
      OLED_ShowNum(4, 6, SERVER_PORT, 5);
      Delay_ms(3000);
    }
    // 正常工作状态
    else
    {
      OLED_Clear();
      if (DHT11_Read_Data(&humi_i, &humi_d, &temp_i, &temp_d) == 0)
      {
        sprintf(temperature, "Temp:%d.%d`C", temp_i, temp_d);
        sprintf(humidity, "Humi:%d.%d%%", humi_i, humi_d);
        sprintf(json_buf, "{\"temperature\":\"%d.%d\",\"humidity\":\"%d.%d\"}",
                temp_i, temp_d,  // 温度：整数.小数
                humi_i, humi_d); // 湿度：整数.小数

        // 发送JSON数据到服务器
        if (!ESP8266_SendJSON(SERVER_IP, SERVER_PORT, json_buf))
        {
          OLED_Clear();
          OLED_ShowString(1, 1, "Send Failed");
          Delay_ms(2000);
          server_status = 0;
        }
        OLED_ShowString(1, 1, temperature);
        OLED_ShowString(2, 1, humidity);
      }
      else
      {
        OLED_ShowString(1, 1, "DHT11 Error!");
        OLED_ShowString(2, 1, "Read Failed");
      }

      Delay_ms(UPLOAD_INTERVAL);
    }

    Delay_ms(100);
  }

  OLED_Clear();
  OLED_ShowString(1, 1, "Task Stopped");
}
