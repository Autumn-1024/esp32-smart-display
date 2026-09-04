#include "wifi_manager.h"
#include "config.h"
#include <Preferences.h>

static Preferences prefs;

WifiManager::WifiManager() {}

bool WifiManager::begin() {
    // 读取保存的WiFi信息
    prefs.begin("wifi", true);  // 只读
    String savedSSID = prefs.getString("ssid", "");
    String savedPass = prefs.getString("password", "");
    prefs.end();

    if (savedSSID.length() > 0) {
        // 有保存的配置，直接启动连接
        strncpy(_ssid, savedSSID.c_str(), sizeof(_ssid) - 1);
        strncpy(_password, savedPass.c_str(), sizeof(_password) - 1);

        WiFi.mode(WIFI_STA);
        WiFi.begin(_ssid, _password);
        _connectStart = millis();
        _state = WIFI_CONNECTING;

        return true;
    }
    return false;  // 没有保存的配置，需要配网
}

void WifiManager::generateAPSSID() {
    // 用MAC地址后4位生成唯一SSID
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(_apSSID, sizeof(_apSSID), "%s_%02X%02X",
             AP_SSID_PREFIX, mac[4], mac[5]);
}

void WifiManager::startAP() {
    generateAPSSID();

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(_apSSID, AP_PASSWORD, AP_CHANNEL, 0, AP_MAX_CONN);

    _state = WIFI_AP_MODE;
    strncpy(_ip, CONFIG_IP, sizeof(_ip) - 1);
}

void WifiManager::stopAP() {
    WiFi.softAPdisconnect(true);
}

bool WifiManager::connectSTA(const char* ssid, const char* password) {
    strncpy(_ssid, ssid, sizeof(_ssid) - 1);
    strncpy(_password, password, sizeof(_password) - 1);

    // 保存到flash
    prefs.begin("wifi", false);
    prefs.putString("ssid", _ssid);
    prefs.putString("password", _password);
    prefs.end();

    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);

    _connectStart = millis();
    _state = WIFI_CONNECTING;

    return true;
}

void WifiManager::disconnectSTA() {
    WiFi.disconnect();
    _state = WIFI_IDLE;
}

int WifiManager::scanNetworks(WifiNetwork* results, int maxResults) {
    int n = WiFi.scanNetworks();
    int count = min(n, maxResults);

    for (int i = 0; i < count; i++) {
        strncpy(results[i].ssid, WiFi.SSID(i).c_str(), 32);
        results[i].ssid[32] = '\0';
        results[i].rssi = WiFi.RSSI(i);
        results[i].authType = WiFi.encryptionType(i);
    }

    WiFi.scanDelete();
    return count;
}

void WifiManager::clearScanResults() {
    WiFi.scanDelete();
}

WifiState WifiManager::getState() const {
    return _state;
}

const char* WifiManager::getSSID() const {
    return _ssid;
}

const char* WifiManager::getIP() const {
    return _ip;
}

const char* WifiManager::getAPSSID() const {
    return _apSSID;
}

void WifiManager::onConnected(void (*callback)(const char* ip)) {
    _onConnected = callback;
}

void WifiManager::onFailed(void (*callback)(const char* reason)) {
    _onFailed = callback;
}

void WifiManager::loop() {
    if (_state == WIFI_CONNECTING) {
        if (WiFi.status() == WL_CONNECTED) {
            _state = WIFI_CONNECTED;
            strncpy(_ip, WiFi.localIP().toString().c_str(), sizeof(_ip) - 1);
            if (_onConnected) _onConnected(_ip);
        } else if (millis() - _connectStart > STA_TIMEOUT_MS) {
            _state = WIFI_FAILED;
            WiFi.disconnect();
            if (_onFailed) _onFailed("Timeout");
        }
    }
}
