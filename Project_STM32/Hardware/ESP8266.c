#include "esp8266.h"
#include <string.h>
#include <stdio.h>
#include "oled.h" // OLED 显示屏驱动，用于调试信息显示

/**
 * @brief  发送AT指令并等待指定响应
 * @note   核心AT指令交互函数，所有ESP8266操作基于此函数
 * @param  cmd: 要发送的AT指令字符串（不含\r\n）
 * @param  ack: 期望接收到的响应关键字（如"OK"、"WIFI GOT IP"）
 * @param  timeout: 超时时间(ms)，超过时间未收到响应则判定失败
 * @retval 1-指令执行成功（收到期望响应），0-执行失败（超时或无期望响应）
 */
uint8_t ESP8266_SendCmd(const char *cmd, const char *ack, uint32_t timeout)
{
    USART1_RX_CNT = 0;                            // 清空接收计数
    memset(USART1_RX_BUF, 0, USART1_RX_BUF_SIZE); // 清空接收缓存
    USART1_SendString(cmd);                       // 发送AT指令
    USART1_SendString("\r\n");                    // 补全AT指令结束符

    uint32_t timer = 0;
    while (timer < timeout) // 超时等待循环
    {
        // 检测接收缓存中是否包含期望响应关键字
        if (strstr((char *)USART1_RX_BUF, ack) != NULL)
            return 1; // 收到期望响应，返回成功
        Delay_ms(10); // 短延时，避免CPU空转
        timer += 10;
    }
    return 0; // 超时未收到响应，返回失败
}

/**
 * @brief  ESP8266初始化：配置STA模式并连接WiFi
 * @note   初始化流程：AT指令检测→重启→设置STA模式→单连接→连接WiFi→获取IP
 * @param  ssid: 目标WiFi的SSID（名称）
 * @param  password: 目标WiFi的密码
 * @retval 1-初始化成功（WiFi连接并获取IP），0-初始化失败
 */
uint8_t ESP8266_Init(const char *ssid, const char *password)
{
    OLED_Clear();
    OLED_ShowString(1, 1, "WiFi Init..."); // 显示初始化中提示
    Delay_ms(500);

    // 1. 发送基础AT指令，检测ESP8266是否正常响应
    if (!ESP8266_SendCmd("AT", "OK", 1000))
    {
        OLED_ShowString(2, 1, "AT Fail");
        Delay_ms(2000);
        return 0;
    }
    Delay_ms(500);

    // 2. 重启ESP8266，恢复初始状态
    if (!ESP8266_SendCmd("AT+RST", "OK", 3000))
    {
        OLED_ShowString(2, 1, "RST Fail");
        Delay_ms(2000);
        return 0;
    }
    Delay_ms(1500); // 重启后需等待模块稳定

    // 3. 设置ESP8266为STA模式（连接路由器）
    if (!ESP8266_SendCmd("AT+CWMODE=1", "OK", 1000))
    {
        OLED_ShowString(2, 1, "MODE Fail");
        Delay_ms(2000);
        return 0;
    }
    Delay_ms(500);

    // 4. 设置单连接模式（CIPMUX=0），简化TCP/UDP操作
    if (!ESP8266_SendCmd("AT+CIPMUX=0", "OK", 1000))
    {
        OLED_ShowString(2, 1, "MUX Fail");
        Delay_ms(2000);
        return 0;
    }
    Delay_ms(500);

    // 5. 连接指定WiFi，等待"WIFI GOT IP"确认获取IP
    char cmd[128];
    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", ssid, password); // 拼接WiFi连接指令
    if (!ESP8266_SendCmd(cmd, "WIFI GOT IP", 10000))        // 超时设为10s，适配WiFi连接耗时
    {
        OLED_ShowString(2, 1, "WiFi Fail");
        Delay_ms(2000);
        return 0;
    }

    Delay_ms(2000);
    return 1; // 初始化全部完成
}

/**
 * @brief  向指定服务器发送JSON数据（HTTP POST方式）
 * @note   核心数据发送流程：PING检测→TCP连接→HTTP POST→关闭连接
 * @param  ip: 服务器IP地址（如"192.168.1.100"）
 * @param  port: 服务器端口（如80、8080）
 * @param  json: 要发送的JSON格式字符串
 * @retval 1-发送成功，0-发送失败（网络/连接/发送任一环节出错）
 */
uint8_t ESP8266_SendTest(const char *ip, uint16_t port, const char *json)
{
    char cmd[256], http_req[512]; // 指令缓存和HTTP请求缓存

    // 1. PING服务器，检测网络层连通性
    OLED_Clear();
    OLED_ShowString(1, 1, "PING Server...");
    sprintf(cmd, "AT+PING=\"%s\"", ip); // 拼接PING指令
    if (!ESP8266_SendCmd(cmd, "OK", 3000))
    {
        OLED_ShowString(2, 1, "PING Fail");
        Delay_ms(4000);
        return 0;
    }
    OLED_ShowString(2, 1, "PING OK");
    Delay_ms(1000);

    // 2. 建立TCP连接（最多重试2次）
    uint8_t retry = 0;
    while (retry < 2) // 重试机制，提升连接成功率
    {
        sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%d", ip, port); // 拼接TCP连接指令
        OLED_Clear();
        OLED_ShowString(1, 1, "TCP Try:");
        OLED_ShowNum(1, 9, retry + 1, 1); // 显示重试次数

        if (ESP8266_SendCmd(cmd, "OK", 5000)) // 建立TCP连接
        {
            break; // 连接成功，退出重试循环
        }

        // 连接失败处理
        OLED_ShowString(2, 1, "TCP Fail");
        Delay_ms(1500);
        ESP8266_SendCmd("AT+CIPCLOSE", "OK", 1000); // 关闭无效连接
        Delay_ms(500);
        retry++;
    }

    // 重试2次仍失败，判定TCP连接失败
    if (retry >= 2)
    {
        ESP8266_SendCmd("AT+CIPCLOSE", "OK", 1000);
        OLED_Clear();
        OLED_ShowString(1, 1, "TCP Final Fail");
        Delay_ms(2000);
        return 0;
    }

    OLED_ShowString(2, 1, "TCP OK");
    Delay_ms(1000);

    // 3. 构建并发送HTTP POST请求
    uint16_t json_len = strlen(json);
    // 拼接HTTP POST请求头+JSON数据体
    sprintf(http_req, "POST /data HTTP/1.1\r\n"
                      "Host: %s:%d\r\n"
                      "Content-Type: application/json\r\n"
                      "Content-Length: %d\r\n"
                      "\r\n"
                      "%s",
            ip, port, json_len, json);
    uint16_t req_len = strlen(http_req); // 获取完整HTTP请求长度

    // 发送数据长度指令，等待模块返回">"提示符
    sprintf(cmd, "AT+CIPSEND=%d", req_len);
    Delay_ms(100);
    USART1_RX_CNT = 0;
    memset(USART1_RX_BUF, 0, USART1_RX_BUF_SIZE);
    Delay_ms(100);

    OLED_Clear();
    OLED_ShowString(1, 1, "Wait >...");
    if (!ESP8266_SendCmd(cmd, ">", 3000)) // 等待模块准备接收数据
    {
        ESP8266_SendCmd("AT+CIPCLOSE", "OK", 1000); // 失败则关闭连接
        OLED_Clear();
        OLED_ShowString(1, 1, "Send Prep Fail");
        Delay_ms(2000);
        return 0;
    }
    Delay_ms(300);

    // 发送完整的HTTP POST请求数据
    USART1_RX_CNT = 0;
    USART1_SendString(http_req);
    OLED_Clear();
    OLED_ShowString(1, 1, "Sending...");
    Delay_ms(200);

    // 等待数据发送成功响应
    uint32_t timer = 0;
    while (timer < 5000)
    {
        if (strstr((char *)USART1_RX_BUF, "SEND OK") != NULL)
            break; // 检测到发送成功
        Delay_ms(10);
        timer += 10;
    }
    // 发送超时处理
    if (timer >= 5000)
    {
        ESP8266_SendCmd("AT+CIPCLOSE", "OK", 1000);
        OLED_Clear();
        OLED_ShowString(1, 1, "Send Timeout");
        Delay_ms(2000);
        return 0;
    }

    // 4. 发送完成，关闭TCP连接
    ESP8266_SendCmd("AT+CIPCLOSE", "OK", 1000);
    OLED_Clear();
    OLED_ShowString(1, 1, "SendTest Success");
    Delay_ms(2000);
    return 1;
}

/**
 * @brief  向指定服务器发送JSON数据（HTTP POST方式）
 * @note   核心数据发送流程：PING检测→TCP连接→HTTP POST→关闭连接
 * @param  ip: 服务器IP地址（如"192.168.1.100"）
 * @param  port: 服务器端口（如80、8080）
 * @param  json: 要发送的JSON格式字符串
 * @retval 1-发送成功，0-发送失败（网络/连接/发送任一环节出错）
 */
uint8_t ESP8266_SendJSON(const char *ip, uint16_t port, const char *json)
{
    char cmd[256], http_req[512]; // 指令缓存和HTTP请求缓存

    // 建立TCP连接（最多重试2次）
    uint8_t retry = 0;
    while (retry < 2) // 重试机制，提升连接成功率
    {
        sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%d", ip, port); // 拼接TCP连接指令

        if (ESP8266_SendCmd(cmd, "OK", 5000)) // 建立TCP连接
        {
            break; // 连接成功，退出重试循环
        }

        ESP8266_SendCmd("AT+CIPCLOSE", "OK", 1000); // 关闭无效连接
        Delay_ms(500);
        retry++;
    }

    // 重试2次仍失败，判定TCP连接失败
    if (retry >= 2)
    {
        ESP8266_SendCmd("AT+CIPCLOSE", "OK", 1000);
        OLED_Clear();
        OLED_ShowString(1, 1, "TCP Final Fail");
        Delay_ms(2000);
        return 0;
    }

    // 构建并发送HTTP POST请求
    uint16_t json_len = strlen(json);
    // 拼接HTTP POST请求头+JSON数据体
    sprintf(http_req, "POST /data HTTP/1.1\r\n"
                      "Host: %s:%d\r\n"
                      "Content-Type: application/json\r\n"
                      "Content-Length: %d\r\n"
                      "\r\n"
                      "%s",
            ip, port, json_len, json);
    uint16_t req_len = strlen(http_req); // 获取完整HTTP请求长度

    // 发送数据长度指令，等待模块返回">"提示符
    sprintf(cmd, "AT+CIPSEND=%d", req_len);
    USART1_RX_CNT = 0;
    memset(USART1_RX_BUF, 0, USART1_RX_BUF_SIZE);

    if (!ESP8266_SendCmd(cmd, ">", 3000)) // 等待模块准备接收数据
    {
        ESP8266_SendCmd("AT+CIPCLOSE", "OK", 1000); // 失败则关闭连接
        return 0;
    }

    // 发送完整的HTTP POST请求数据
    USART1_RX_CNT = 0;
    USART1_SendString(http_req);

    // 等待数据发送成功响应
    uint32_t timer = 0;
    while (timer < 5000)
    {
        if (strstr((char *)USART1_RX_BUF, "SEND OK") != NULL)
            break; // 检测到发送成功
        Delay_ms(10);
        timer += 10;
    }
    // 发送超时处理
    if (timer >= 5000)
    {
        ESP8266_SendCmd("AT+CIPCLOSE", "OK", 1000);
        OLED_Clear();
        OLED_ShowString(1, 1, "Send Timeout");
        Delay_ms(2000);
        return 0;
    }

    // 4. 发送完成，关闭TCP连接
    ESP8266_SendCmd("AT+CIPCLOSE", "OK", 1000);
    OLED_ShowString(4, 1, "KEEP SENDING");
    return 1;
}
