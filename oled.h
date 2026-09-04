#ifndef OLED_H
#define OLED_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// 显示状态枚举
enum DisplayState {
    DISP_BOOT,          // 开机画面
    DISP_AP_WAITING,    // 等待连接AP
    DISP_WIFI_CONNECTING, // 正在连接WiFi
    DISP_WIFI_SUCCESS,  // WiFi连接成功
    DISP_WIFI_FAIL,     // WiFi连接失败
    DISP_WIFI_SCANNING  // 正在扫描WiFi
};

class OledDisplay {
public:
    bool begin();
    void update();

    // 各状态显示内容
    void showBoot();
    void showAPWaiting(const char* ssid, const char* ip);
    void showConnecting(const char* ssid);
    void showSuccess(const char* ip);
    void showFail(const char* msg);
    void showScanning();

    void setState(DisplayState state);
    DisplayState getState() const;

private:
    Adafruit_SSD1306* _display = nullptr;
    DisplayState _state = DISP_BOOT;
    unsigned long _lastUpdate = 0;

    void clear();
    void display();
};

#endif // OLED_H
