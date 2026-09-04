#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>

class ConfigWebServer {
public:
    bool begin();
    void stop();
    void handleClient();

    // 模式切换
    void startConfigMode();   // 配网模式
    void startManageMode();   // 管理模式（连上WiFi后）

    // 获取随机模拟数据
    void generateSensorData();

private:
    WebServer* _server = nullptr;
    bool _manageMode = false;

    // 模拟传感器数据
    float screenTemp;
    float screenHumi;
    float phaseATemp, phaseBTemp, phaseCTemp;
    float phaseAVolt, phaseBVolt, phaseCVolt;
    float phaseACurr, phaseBCurr, phaseCCurr;

    // 配网模式路由
    void handleRoot();
    void handleScan();
    void handleConnect();

    // 管理模式路由
    void handleManageRoot();
    void handleApiData();

    void handleNotFound();

    void sendConfigPage();
    void sendManagePage();
};

#endif // WEB_SERVER_H
