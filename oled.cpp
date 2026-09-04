#include "oled.h"
#include "config.h"

bool OledDisplay::begin() {
    Wire.begin(OLED_SDA, OLED_SCL);
    _display = new Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

    if (!_display->begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        return false;
    }

    _display->clearDisplay();
    _display->setTextColor(SSD1306_WHITE);
    _display->setTextSize(1);
    _display->display();
    return true;
}

void OledDisplay::clear() {
    _display->clearDisplay();
}

void OledDisplay::display() {
    _display->display();
}

void OledDisplay::setState(DisplayState state) {
    _state = state;
}

DisplayState OledDisplay::getState() const {
    return _state;
}

void OledDisplay::showBoot() {
    clear();
    _display->setTextSize(2);
    _display->setTextColor(SSD1306_WHITE);
    _display->setCursor(10, 10);
    _display->println("ESP32");
    _display->setTextSize(1);
    _display->setCursor(10, 40);
    _display->println("Smart Display");
    _display->setCursor(10, 55);
    _display->println("Booting...");
    display();
}

void OledDisplay::showAPWaiting(const char* ssid, const char* ip) {
    clear();
    _display->setTextSize(1);
    _display->setTextColor(SSD1306_WHITE);

    // 标题
    _display->setCursor(0, 0);
    _display->println("=== WiFi Config ===");

    // 第二行：请连接WiFi
    _display->setCursor(0, 16);
    _display->println("Please connect to:");

    // WiFi名称
    _display->setCursor(0, 28);
    _display->print("SSID: ");
    _display->println(ssid);

    // 管理地址
    _display->setCursor(0, 42);
    _display->print("URL:  ");
    _display->println(ip);

    // 提示
    _display->setCursor(0, 56);
    _display->println("Open browser & visit URL");

    display();
}

void OledDisplay::showConnecting(const char* ssid) {
    clear();
    _display->setTextSize(1);
    _display->setTextColor(SSD1306_WHITE);

    _display->setCursor(0, 10);
    _display->println("Connecting to WiFi...");

    _display->setCursor(0, 28);
    _display->print("SSID: ");
    _display->println(ssid);

    // 动画点
    static int dots = 0;
    _display->setCursor(0, 46);
    _display->print("Please wait");
    for (int i = 0; i < (dots % 4); i++) {
        _display->print(".");
    }
    dots++;

    display();
}

void OledDisplay::showSuccess(const char* ip) {
    clear();
    _display->setTextSize(1);
    _display->setTextColor(SSD1306_WHITE);

    _display->setCursor(0, 5);
    _display->println("*** WiFi Connected! ***");

    _display->setCursor(0, 22);
    _display->println("Status: ONLINE");

    _display->setCursor(0, 38);
    _display->print("IP: ");
    _display->println(ip);

    _display->setCursor(0, 52);
    _display->println("System ready.");

    display();
}

void OledDisplay::showFail(const char* msg) {
    clear();
    _display->setTextSize(1);
    _display->setTextColor(SSD1306_WHITE);

    _display->setCursor(0, 5);
    _display->println("WiFi Connect FAILED!");

    _display->setCursor(0, 22);
    _display->print("Reason: ");
    _display->println(msg);

    _display->setCursor(0, 42);
    _display->println("Restarting AP mode...");
    _display->setCursor(0, 56);
    _display->println("Reconnect to configure");

    display();
}

void OledDisplay::showScanning() {
    clear();
    _display->setTextSize(1);
    _display->setTextColor(SSD1306_WHITE);

    _display->setCursor(20, 25);
    _display->println("Scanning WiFi...");
    _display->setCursor(30, 40);
    _display->println("Please wait");

    display();
}

void OledDisplay::update() {
    // 根据当前状态刷新显示（如动画等）
    unsigned long now = millis();
    if (now - _lastUpdate < DISPLAY_UPDATE_MS) return;
    _lastUpdate = now;

    // 只对需要动画的状态做刷新
    if (_state == DISP_WIFI_CONNECTING) {
        showConnecting("");
    }
}
