#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>

// WiFi状态枚举
enum WifiState {
    WIFI_IDLE,
    WIFI_AP_MODE,       // AP模式等待配置
    WIFI_CONNECTING,    // 正在连接路由器
    WIFI_CONNECTED,     // 连接成功
    WIFI_FAILED         // 连接失败
};

// 扫描到的WiFi信息
struct WifiNetwork {
    char ssid[33];
    int32_t rssi;
    wifi_auth_mode_t authType;
};

class WifiManager {
public:
    WifiManager();
    bool begin();

    // AP模式
    void startAP();
    void stopAP();

    // STA模式
    bool connectSTA(const char* ssid, const char* password);
    void disconnectSTA();

    // WiFi扫描
    int scanNetworks(WifiNetwork* results, int maxResults);
    void clearScanResults();

    // 状态
    WifiState getState() const;
    const char* getSSID() const;
    const char* getIP() const;
    const char* getAPSSID() const;

    // 定时检查连接状态
    void loop();

    // 回调
    void onConnected(void (*callback)(const char* ip));
    void onFailed(void (*callback)(const char* reason));

private:
    WifiState _state = WIFI_IDLE;
    char _ssid[33] = {0};
    char _password[65] = {0};
    char _apSSID[40] = {0};
    char _ip[20] = {0};

    unsigned long _connectStart = 0;

    void (*_onConnected)(const char* ip) = nullptr;
    void (*_onFailed)(const char* reason) = nullptr;

    void generateAPSSID();
};

#endif // WIFI_MANAGER_H
