# OMIT VCC、GND
# STM32F103C8T6:

# OLED:
    SCL -> PB8
    SDA -> PB9

# DHT11:
    I/O -> PB11
    EN  -> default 悬空

# ESP8266(esp-01s):
    RX  -> PA9
    TX  -> PA10
    EN  -> 3.3V
    RST -> 3.3V
    GPIO0 -> 3.3V
    GPIO2 -> 3.3V

# JDY-31:
    RX  -> PA2
    TX  -> PA3
    EN  -> default 悬空
    STATE -> default 悬空

# JDY-31:
# JSON:
# Test-OK
{"BT": "OK"}
# ChangeConfig
{
    "wifi": "/** wifi名称 **/",
    "password": "/** wifi密码 **/",
    "server": "/** 服务器地址 **/",
    "port": "/** 端口号 **/",
    "interval": "/** 间隔时间 **/"
}
