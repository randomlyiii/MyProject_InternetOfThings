#include "jdy31.h"
#include "delay.h"
#include "esp8266_config.h"

// 全局变量定义
uint8_t JDY31_RX_BUF[JDY31_RX_BUF_SIZE];
uint8_t JDY31_RX_CNT = 0;
uint8_t JDY31_RX_FLAG = 0;
uint8_t JDY31_Connected_Flag = 0; // 0=未连接 1=已连接，6针jdy31防重复发送
// Server_Config_t ServerCfg;

// 串口2初始化 PA2=TX(接JDY31 RX) PA3=RX(接JDY31 TX)
void JDY31_Init(void)
{
    // 开启外设时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    Delay_ms(1000); // 等待JDY31上电稳定，进入未连接的AT模式

    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // PA3 RX 上拉输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 串口核心配置
    USART_InitStructure.USART_BaudRate = JDY31_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(JDY31_USART, &USART_InitStructure);

    // PA2 TX 复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 中断配置：接收中断+空闲中断（解决无换行数据无法接收）
    USART_ITConfig(JDY31_USART, USART_IT_RXNE, ENABLE);
    USART_ITConfig(JDY31_USART, USART_IT_IDLE, ENABLE);
    USART_Cmd(JDY31_USART, ENABLE);

    // 中断优先级配置
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 默认参数初始化
    // strcpy(ServerCfg.server_ip, "192.168.1.100");
    // ServerCfg.port = 8080;
    // ServerCfg.upload_interval = 2000;
    JDY31_Connected_Flag = 0;
}

// 串口发送字符串
void JDY31_Send_String(char *str)
{
    while (*str)
    {
        USART_SendData(JDY31_USART, *str++);
        while (USART_GetFlagStatus(JDY31_USART, USART_FLAG_TXE) == RESET)
            ;
    }
}

// 发送AT指令并检测应答
uint8_t JDY31_Send_AT_Cmd(char *cmd, char *ack, uint16_t timeout)
{
    JDY31_Clear_RX_Buf();
    JDY31_Send_String(cmd);
    uint16_t t = 0;
    while (t < timeout)
    {
        if (JDY31_RX_FLAG)
        {
            if (strstr((char *)JDY31_RX_BUF, ack) != NULL)
            {
                return 1;
            }
            JDY31_Clear_RX_Buf();
        }
        Delay_ms(1);
        t++;
    }
    return 0;
}

// 配置蓝牙名称
void JDY31_Set_Config(void)
{
    char cmd_buf[32];
    Delay_ms(1000); // 确保模块处于未连接的AT模式

    // 设置蓝牙名称
    sprintf(cmd_buf, "AT+NAME%s\r\n", JDY31_NAME);
    if (JDY31_Send_AT_Cmd(cmd_buf, "OK", 1000))
    {
        JDY31_Send_String("Name Set OK\r\n");
    }
    else
    {
        JDY31_Send_String("Name Set Fail\r\n");
    }
    Delay_ms(500);

    // 重启模块，确保配置永久生效
    if (JDY31_Send_AT_Cmd("AT+RST\r\n", "ready", 2000))
    {
        JDY31_Send_String("Module Reset OK\r\n");
    }
    else
    {
        JDY31_Send_String("Module Reset Fail\r\n");
    }
    Delay_ms(1000); // 等待模块重启
    JDY31_Send_String("JDY31 Config Done\r\n");
}

// 发送温湿度JSON数据
void JDY31_Send_JSON_Data(uint8_t temp_i, uint8_t temp_d, uint8_t humi_i, uint8_t humi_d)
{
    char buf[64];
    sprintf(buf, "{\"temperature\": \"%d.%d\",\"humidity\": \"%d.%d\"}\r\n", temp_i, temp_d, humi_i, humi_d);
    JDY31_Send_String(buf);
}

// 解析JSON配置
// 解析 JSON 服务器参数
void JDY31_Parse_JSON(void)
{
    char *p = (char *)JDY31_RX_BUF;
    char *start = NULL;
    char *end = NULL;

    // 1. 提取 WiFi 名称 (wifi)
    start = strstr(p, "\"wifi\":");
    if (start != NULL)
    {
        start += 7; // 跳过 "\"wifi\":"
        // 跳过可能的空格
        while (*start == ' ' || *start == '\"')
            start++;
        end = strstr(start, "\"");
        if (end != NULL && (end - start) < sizeof(WIFI_SSID))
        {
            uint8_t len = end - start;
            strncpy(WIFI_SSID, start, len);
            WIFI_SSID[len] = '\0';
        }
    }

    // 2. 提取 WiFi 密码 (password)
    start = strstr(p, "\"password\":");
    if (start != NULL)
    {
        start += 11; // 跳过 "\"password\":"
        while (*start == ' ' || *start == '\"')
            start++;
        end = strstr(start, "\"");
        if (end != NULL && (end - start) < sizeof(WIFI_PWD))
        {
            uint8_t len = end - start;
            strncpy(WIFI_PWD, start, len);
            WIFI_PWD[len] = '\0';
        }
    }

    // 3. 提取服务器地址 (server)
    start = strstr(p, "\"server\":");
    if (start != NULL)
    {
        start += 9; // 跳过 "\"server\":"
        while (*start == ' ' || *start == '\"')
            start++;
        end = strstr(start, "\"");
        if (end != NULL && (end - start) < sizeof(SERVER_IP))
        {
            uint8_t len = end - start;
            strncpy(SERVER_IP, start, len);
            SERVER_IP[len] = '\0';
        }
    }

    // 4. 提取端口号 (port)
    start = strstr(p, "\"port\":");
    if (start != NULL)
    {
        start += 7; // 跳过 "\"port\":"
        while (*start == ' ' || *start == '\"')
            start++; // 兼容带引号或不带引号的数字
        SERVER_PORT = atoi(start);
    }

    // 5. 提取上报间隔 (interval)
    start = strstr(p, "\"interval\":");
    if (start != NULL)
    {
        start += 11; // 跳过 "\"interval\":"
        while (*start == ' ' || *start == '\"')
            start++;
        UPLOAD_INTERVAL = atoi(start);
        if (UPLOAD_INTERVAL < 500) // 最小间隔限制
            UPLOAD_INTERVAL = 500;
    }

    // 回显解析结果，方便调试
    char resp_buf[256];
    sprintf(resp_buf, "[ConfigSet] WiFi:%s Pwd:%s IP:%s Port:%d Interval:%dms\r\n",
            WIFI_SSID, WIFI_PWD, SERVER_IP, SERVER_PORT, UPLOAD_INTERVAL);
    JDY31_Send_String(resp_buf);
}

void JDY31_Send_Config(void)
{
    char resp_buf[256];
    sprintf(resp_buf, "[CurrentConfig] WiFi:%s Pwd:%s IP:%s Port:%d Interval:%dms\r\n",
            WIFI_SSID, WIFI_PWD, SERVER_IP, SERVER_PORT, UPLOAD_INTERVAL);
    JDY31_Send_String(resp_buf);
}

/**
 * @brief  解析JDY31接收到的JSON数据，检测是否包含 {" BT ":" OK "}
 * @retval 1-检测到目标数据  0-未检测到
 */
#include <string.h>
#include <ctype.h>

uint8_t JDY31_Parse_TestOK(void)
{
    // 1. 没有数据直接返回
    if (JDY31_RX_CNT == 0)
        return 0;

    // 2. 安全结束字符串
    JDY31_RX_BUF[JDY31_RX_CNT] = '\0';

    char *p = (char *)JDY31_RX_BUF;
    // 匹配 { （允许前后空格）
    while (*p && isspace((unsigned char)*p))
        p++;
    if (*p != '{')
        return 0;
    p++;

    // 匹配 "BT"
    while (*p && isspace((unsigned char)*p))
        p++;
    if (*p != '"')
        return 0;
    p++;

    // 匹配 BT
    if (*p != 'B' || *(p + 1) != 'T')
        return 0;
    p += 2;

    // 匹配结束引号
    while (*p && isspace((unsigned char)*p))
        p++;
    if (*p != '"')
        return 0;
    p++;

    // 匹配 :
    while (*p && isspace((unsigned char)*p))
        p++;
    if (*p != ':')
        return 0;
    p++;

    // 匹配 "OK"
    while (*p && isspace((unsigned char)*p))
        p++;
    if (*p != '"')
        return 0;
    p++;

    // 匹配 OK
    if (*p != 'O' || *(p + 1) != 'K')
        return 0;
    p += 2;

    // 匹配结束引号
    while (*p && isspace((unsigned char)*p))
        p++;
    if (*p != '"')
        return 0;

    return 1;
}

// 清空接收缓冲区和标志
void JDY31_Clear_RX_Buf(void)
{
    JDY31_RX_CNT = 0;
    JDY31_RX_FLAG = 0;
    memset(JDY31_RX_BUF, 0, sizeof(JDY31_RX_BUF));
}

// 串口2中断服务函数（核心修复：空闲中断+连接状态检测）
void USART2_IRQHandler(void)
{
    uint8_t ch;
    // 接收中断：处理单个字节
    if (USART_GetITStatus(JDY31_USART, USART_IT_RXNE) != RESET)
    {
        ch = USART_ReceiveData(JDY31_USART);
        USART_ClearITPendingBit(JDY31_USART, USART_IT_RXNE);

        if (JDY31_RX_CNT < JDY31_RX_BUF_SIZE - 1)
        {
            JDY31_RX_BUF[JDY31_RX_CNT++] = ch;
            // 兼容带换行的帧结束触发
            if (ch == '\n' || ch == '\r')
            {
                JDY31_RX_FLAG = 1;
                if (ch == '\r')
                    JDY31_RX_BUF[JDY31_RX_CNT - 1] = '\n';
            }
        }
        else
        {
            JDY31_Clear_RX_Buf(); // 缓冲区溢出清空
        }
    }

    // 空闲中断：处理一帧数据接收完成（解决无换行数据接收问题）
    if (USART_GetITStatus(JDY31_USART, USART_IT_IDLE) != RESET)
    {
        ch = USART_ReceiveData(JDY31_USART); // 读DR寄存器清除中断标志
        (void)ch;

        // 有数据才置位接收标志
        if (JDY31_RX_CNT > 0)
        {
            JDY31_RX_FLAG = 1;
        }

        // // 检测蓝牙连接成功，自动发送connected
        // if (strstr((char *)JDY31_RX_BUF, "CONNECTED") != NULL)
        // {
        //     if (JDY31_Connected_Flag == 0)
        //     {
        //         JDY31_Connected_Flag = 1;
        //         JDY31_Send_String("connected\r\n"); // 连接成功自动发送
        //     }
        // }

        // // 检测蓝牙断开，重置标志位
        // if (strstr((char *)JDY31_RX_BUF, "DISCONNECT") != NULL)
        // {
        //     JDY31_Connected_Flag = 0;
        //     JDY31_Clear_RX_Buf();
        // }
    }
}
