#ifndef CONFIG_H
#define CONFIG_H

/* ========================
 *  引脚定义
 * ======================== */
// 按键引脚
#define BTN_0   25
#define BTN_1   26
#define BTN_2   32
#define BTN_3   33

// OLED IIC 引脚
#define OLED_SDA  21
#define OLED_SCL  22
#define OLED_ADDR 0x3C  // 默认IIC地址，部分模块是0x3D

// DHT11 温湿度传感器
#define DHT_PIN   15
#define DHT_TYPE  DHT11

// DS18B20 温度传感器（三相温度）
#define DS18B20_PIN_A  4   // A相温度
#define DS18B20_PIN_B  13  // B相温度
#define DS18B20_PIN_C  14  // C相温度

// OLED 尺寸
#define OLED_WIDTH  128
#define OLED_HEIGHT 64

/* ========================
 *  WiFi 配置
 * ======================== */
// AP模式 - 板子自身热点
#define AP_SSID_PREFIX  "ESP32-Setup"
#define AP_PASSWORD     ""          // 热点无密码，方便连接
#define AP_CHANNEL      6
#define AP_MAX_CONN     4           // 最大连接数

// 配网地址
#define CONFIG_IP       "192.168.4.1"

// STA模式 - 连接路由器
#define STA_TIMEOUT_MS  15000       // 连接超时15秒

/* ========================
 *  按键去抖
 * ======================== */
#define BTN_DEBOUNCE_MS 50

/* ========================
 *  显示更新间隔
 * ======================== */
#define DISPLAY_UPDATE_MS  500      // OLED刷新间隔

#endif // CONFIG_H
