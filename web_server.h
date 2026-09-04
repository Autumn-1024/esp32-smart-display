#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>

class ConfigWebServer {
public:
    bool begin();
    void stop();
    void handleClient();

    // 触发WiFi连接
    void connectToWifi(const char* ssid, const char* password);

private:
    WebServer* _server = nullptr;

    // 路由处理
    void handleRoot();
    void handleScan();
    void handleConnect();
    void handleStatus();
    void handleNotFound();

    // 发送配网HTML页面
    void sendConfigPage();
};

#endif // WEB_SERVER_H
