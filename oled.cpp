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

// 绘制分隔线
void OledDisplay::drawSeparator(int y) {
    _display->drawFastHLine(0, y, OLED_WIDTH, SSD1306_WHITE);
}

// ============================
//  开机画面
// ============================
void OledDisplay::showBoot() {
    clear();
    _display->setTextSize(1);
    _display->setTextColor(SSD1306_WHITE);

    // 第1行：标题
    _display->setCursor(0, 0);
    _display->print("[Smart Display]");
    drawSeparator(10);

    // 第3行：品牌
    _display->setTextSize(2);
    _display->setCursor(25, 20);
    _display->print("ESP32");

    // 第5行：状态
    _display->setTextSize(1);
    _display->setCursor(0, 45);
    _display->print("Booting...");

    // 第7行：版本
    _display->setCursor(0, 56);
    _display->print("v1.0 | 2026-09-04");

    display();
}

// ============================
//  WiFi配置页面（AP模式）
// ============================
void OledDisplay::showAPWaiting(const char* ssid, const char* ip) {
    clear();
    _display->setTextSize(1);
    _display->setTextColor(SSD1306_WHITE);

    // 第1行：标题
    _display->setCursor(0, 0);
    _display->print("[WiFi Config]");
    drawSeparator(10);

    // 第3行：提示连接
    _display->setCursor(0, 14);
    _display->print("Connect to WiFi:");

    // 第4行：SSID
    _display->setCursor(0, 24);
    _display->print("SSID: ");
    _display->print(ssid);

    // 第5行：空行

    // 第6行：地址
    _display->setCursor(0, 40);
    _display->print("Open: ");
    _display->print(ip);

    // 第7行：分隔线
    drawSeparator(52);

    // 第8行：提示
    _display->setCursor(0, 56);
    _display->print("Enter pwd in browser");

    display();
}

// ============================
//  正在连接WiFi
// ============================
void OledDisplay::showConnecting(const char* ssid) {
    clear();
    _display->setTextSize(1);
    _display->setTextColor(SSD1306_WHITE);

    // 第1行：标题
    _display->setCursor(0, 0);
    _display->print("[WiFi Connect]");
    drawSeparator(10);

    // 第3行：正在连接
    _display->setCursor(0, 14);
    _display->print("Connecting...");

    // 第4行：SSID
    _display->setCursor(0, 24);
    _display->print("SSID: ");
    _display->print(ssid);

    // 第5行：空行

    // 第6行：进度动画
    static int dots = 0;
    _display->setCursor(0, 40);
    _display->print("Please wait");
    for (int i = 0; i < (dots % 4); i++) {
        _display->print(".");
    }
    dots++;

    // 第7行：提示
    _display->setCursor(0, 56);
    _display->print("Press BTN0 to retry");

    display();
}

// ============================
//  WiFi连接成功
// ============================
void OledDisplay::showSuccess(const char* ip) {
    clear();
    _display->setTextSize(1);
    _display->setTextColor(SSD1306_WHITE);

    // 第1行：标题
    _display->setCursor(0, 0);
    _display->print("[WiFi Status]");
    drawSeparator(10);

    // 第3行：状态
    _display->setCursor(0, 14);
    _display->print("Status: ONLINE");

    // 第4行：IP
    _display->setCursor(0, 24);
    _display->print("IP: ");
    _display->print(ip);

    // 第5行：空行

    // 第6行：分隔线
    _display->setCursor(0, 40);
    drawSeparator(40);

    // 第7行：提示
    _display->setCursor(0, 44);
    _display->print("System ready");

    // 第8行：按键提示
    _display->setCursor(0, 56);
    _display->print("BTN0: Config WiFi");

    display();
}

// ============================
//  WiFi连接失败
// ============================
void OledDisplay::showFail(const char* msg) {
    clear();
    _display->setTextSize(1);
    _display->setTextColor(SSD1306_WHITE);

    // 第1行：标题
    _display->setCursor(0, 0);
    _display->print("[WiFi Error]");
    drawSeparator(10);

    // 第3行：失败原因
    _display->setCursor(0, 14);
    _display->print("Failed: ");
    _display->print(msg);

    // 第4-5行：空行

    // 第6行：提示
    _display->setCursor(0, 38);
    _display->print("Back to AP mode...");

    // 第7行：分隔线
    drawSeparator(50);

    // 第8行：按键提示
    _display->setCursor(0, 54);
    _display->print("BTN0: Reconfig WiFi");

    display();
}

// ============================
//  正在扫描WiFi
// ============================
void OledDisplay::showScanning() {
    clear();
    _display->setTextSize(1);
    _display->setTextColor(SSD1306_WHITE);

    // 第1行：标题
    _display->setCursor(0, 0);
    _display->print("[WiFi Scan]");
    drawSeparator(10);

    // 第4行：扫描中
    _display->setCursor(0, 30);
    _display->print("Scanning WiFi...");

    // 第6行：等待
    _display->setCursor(0, 48);
    _display->print("Please wait...");

    display();
}

void OledDisplay::update() {
    unsigned long now = millis();
    if (now - _lastUpdate < DISPLAY_UPDATE_MS) return;
    _lastUpdate = now;

    if (_state == DISP_WIFI_CONNECTING) {
        showConnecting("");
    }
}
