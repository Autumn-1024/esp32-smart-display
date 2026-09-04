/*
 * ESP32 Smart Display - 大屏传感采集系统
 * 
 * 功能：
 *   - 上电自动连接已保存的WiFi
 *   - 按键0长按：进入AP配网模式
 *   - AP模式下：Web页面配置WiFi
 *   - OLED显示状态信息
 * 
 * 硬件：
 *   按键: GPIO 25(BTN0), 26(BTN1), 32(BTN2), 33(BTN3)
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
#include <DHT.h>

// ============================
//  全局对象
// ============================
OledDisplay oled;
WifiManager wifiMgr;
ConfigWebServer webServer;
ButtonManager buttons;
DHT dht(DHT_PIN, DHT_TYPE);

// WiFi连接结果标志
static volatile bool wifiConnectDone = false;
static volatile bool wifiConnectOK = false;
static String wifiResultIP = "";

// 配网请求标志
static bool requestProvisioning = false;

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
    Serial.println("Entering AP provisioning mode...");

    // 1. 启动AP
    wifiMgr.startAP();

    // 2. 启动Web Server
    webServer.begin();
    webServer.startConfigMode();

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

        // 检查按键0是否再次触发配网（退出当前配网重新开始）
        ButtonEvent event = buttons.getEvent(0);
        if (event == BTN_PRESS || event == BTN_LONG_PRESS) {
            Serial.println("BTN0 pressed, restarting provisioning...");
            webServer.stop();
            wifiMgr.stopAP();
            delay(500);
            wifiMgr.startAP();
            webServer.begin();
            oled.showAPWaiting(wifiMgr.getAPSSID(), CONFIG_IP);
        }

        delay(10);
    }

    // 5. 停止Web Server
    webServer.stop();
}

// ============================
//  WiFi连接流程（STA模式）
// ============================
void runConnectSTA() {
    oled.showConnecting(wifiMgr.getSSID());

    Serial.print("Connecting to: ");
    Serial.println(wifiMgr.getSSID());

    // 重置标志
    wifiConnectDone = false;
    wifiConnectOK = false;

    // 连接并等待结果
    unsigned long startTime = millis();
    while (!wifiConnectDone) {
        wifiMgr.loop();
        oled.update();
        buttons.update();

        // 检查按键0：取消连接，进入配网
        ButtonEvent event = buttons.getEvent(0);
        if (event == BTN_PRESS || event == BTN_LONG_PRESS) {
            Serial.println("BTN0 pressed, aborting connection...");
            wifiMgr.disconnectSTA();
            requestProvisioning = true;
            return;
        }

        // 超时保护（15秒）
        if (millis() - startTime > 15000) {
            Serial.println("Connection timeout!");
            wifiMgr.disconnectSTA();
            wifiConnectDone = true;
            wifiConnectOK = false;
            wifiResultIP = "Timeout";
            break;
        }

        delay(10);
    }

    if (wifiConnectOK) {
        Serial.print("WiFi Connected! IP: ");
        Serial.println(wifiResultIP);
        oled.showSuccess(wifiResultIP.c_str());

        // 启动管理页面Web服务器
        webServer.begin();
        webServer.startManageMode();
        Serial.print("Management page: http://");
        Serial.println(wifiResultIP);
    } else {
        Serial.print("WiFi Connect Failed: ");
        Serial.println(wifiResultIP);
        oled.showFail(wifiResultIP.c_str());
        delay(3000);
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
    }

    // 显示开机画面
    oled.showBoot();
    delay(1500);

    // 初始化按键
    buttons.begin();

    // 初始化DHT11
    dht.begin();
    Serial.println("DHT11 initialized");

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

    // 检查配网请求（来自按键0）
    if (requestProvisioning) {
        requestProvisioning = false;
        runProvisioning();
        runConnectSTA();
    }

    // 按键事件处理
    ButtonEvent event0 = buttons.getEvent(0);
    if (event0 == BTN_LONG_PRESS) {
        // 按键0长按：强制进入配网模式
        Serial.println("BTN0 long press: enter provisioning...");
        wifiMgr.disconnectSTA();
        runProvisioning();
        runConnectSTA();
    } else if (event0 == BTN_PRESS) {
        Serial.println("BTN0 pressed");
        // 短按：可以用于其他功能
    }

    // 其他按键预留
    for (int i = 1; i < 4; i++) {
        ButtonEvent event = buttons.getEvent(i);
        if (event == BTN_PRESS) {
            Serial.print("Button ");
            Serial.print(i);
            Serial.println(" pressed");
            // TODO: 后续添加功能
        }
    }

    // WiFi已连接状态处理
    if (wifiMgr.getState() == WIFI_CONNECTED) {
        // 处理管理页面Web请求
        webServer.handleClient();

        // 每2秒读取一次DHT11数据
        static unsigned long lastDhtRead = 0;
        if (millis() - lastDhtRead > 2000) {
            lastDhtRead = millis();
            float h = dht.readHumidity();
            float t = dht.readTemperature();
            if (!isnan(h) && !isnan(t)) {
                webServer.setScreenData(t, h);
                Serial.printf("DHT11: T=%.1f°C H=%.1f%%\n", t, h);
            } else {
                Serial.println("DHT11 read failed!");
            }
        }
    }

    delay(10);
}
