/*
 * ESP32 Smart Display - 大屏传感采集系统
 * 
 * 功能：AP配网 → Web配置WiFi → 连接路由器
 * 
 * 硬件：
 *   按键: GPIO 25, 26, 32, 33
 *   OLED: SDA=21, SCL=22 (0.96寸 IIC)
 * 
 * 作者: Autumn
 */

#include <Arduino.h>
#include "config.h"
#include "oled.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "button.h"

// ============================
//  全局对象
// ============================
OledDisplay oled;
WifiManager wifiMgr;
ConfigWebServer webServer;
ButtonManager buttons;

// WiFi连接结果标志
static volatile bool wifiConnectDone = false;
static volatile bool wifiConnectOK = false;
static String wifiResultIP = "";

// ============================
//  回调函数
// ============================
void onConnected(const char* ip) {
    wifiConnectDone = true;
    wifiConnectOK = true;
    wifiResultIP = String(ip);
}

void onFailed(const char* reason) {
    wifiConnectDone = true;
    wifiConnectOK = false;
    wifiResultIP = String(reason);
}

// ============================
//  配网流程（AP模式）
// ============================
void runProvisioning() {
    // 1. 启动AP
    wifiMgr.startAP();

    // 2. 启动Web Server
    webServer.begin();

    // 3. 显示等待连接
    oled.showAPWaiting(wifiMgr.getAPSSID(), CONFIG_IP);

    Serial.println("AP Mode started");
    Serial.print("SSID: ");
    Serial.println(wifiMgr.getAPSSID());
    Serial.print("Config URL: http://");
    Serial.println(CONFIG_IP);

    // 4. 等待用户配网
    while (wifiMgr.getState() == WIFI_AP_MODE) {
        webServer.handleClient();
        buttons.update();
        delay(10);
    }

    // 5. 停止Web Server（AP已断开）
    webServer.stop();
}

// ============================
//  WiFi连接流程（STA模式）
// ============================
void runConnectSTA() {
    oled.showConnecting(wifiMgr.getSSID());

    Serial.print("Connecting to: ");
    Serial.println(wifiMgr.getSSID());

    // 连接并等待结果
    while (!wifiConnectDone) {
        wifiMgr.loop();
        oled.update();
        delay(10);
    }

    if (wifiConnectOK) {
        Serial.print("WiFi Connected! IP: ");
        Serial.println(wifiResultIP);
        oled.showSuccess(wifiResultIP.c_str());
    } else {
        Serial.print("WiFi Connect Failed: ");
        Serial.println(wifiResultIP);
        oled.showFail(wifiResultIP.c_str());
        delay(3000);
        // 连接失败，重新进入配网
        runProvisioning();
        runConnectSTA();
    }
}

// ============================
//  Arduino Setup
// ============================
void setup() {
    Serial.begin(115200);
    Serial.println("\n=== ESP32 Smart Display ===");

    // 初始化OLED
    if (!oled.begin()) {
        Serial.println("OLED init failed!");
        // OLED失败也要继续运行
    }

    // 显示开机画面
    oled.showBoot();
    delay(1500);

    // 初始化按键
    buttons.begin();

    // 注册WiFi回调
    wifiMgr.onConnected(onConnected);
    wifiMgr.onFailed(onFailed);

    // 检查是否已有保存的WiFi配置
    if (wifiMgr.begin()) {
        Serial.print("Found saved WiFi: ");
        Serial.println(wifiMgr.getSSID());
        // 直接尝试连接
        runConnectSTA();
    } else {
        Serial.println("No saved WiFi, entering provisioning...");
        // 进入配网模式
        runProvisioning();
        // 配网完成后连接STA
        runConnectSTA();
    }

    Serial.println("System ready!");
}

// ============================
//  Arduino Loop
// ============================
void loop() {
    // 按键扫描
    buttons.update();

    // 按键事件处理（预留）
    for (int i = 0; i < 4; i++) {
        ButtonEvent event = buttons.getEvent(i);
        if (event == BTN_PRESS) {
            Serial.print("Button ");
            Serial.print(i);
            Serial.println(" pressed");
            // TODO: 后续添加按键功能
        } else if (event == BTN_LONG_PRESS) {
            Serial.print("Button ");
            Serial.print(i);
            Serial.println(" long pressed");
            // TODO: 后续添加长按功能（如重置WiFi）
        }
    }

    // WiFi状态检查
    if (wifiMgr.getState() == WIFI_CONNECTED) {
        // 已连接，正常运行
        // TODO: 后续添加主功能逻辑
    }

    delay(10);
}
