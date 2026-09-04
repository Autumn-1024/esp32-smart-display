#include "web_server.h"
#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>

// 外部全局对象
extern WifiManager wifiMgr;

static WebServer server(80);

// ============================
//  配网页面 HTML
// ============================
static const char CONFIG_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 WiFi Config</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
            background: #f0f2f5;
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .card {
            background: #fff;
            border-radius: 16px;
            box-shadow: 0 4px 20px rgba(0,0,0,0.08);
            padding: 32px;
            width: 90%;
            max-width: 400px;
        }
        h1 {
            text-align: center;
            color: #333;
            font-size: 22px;
            margin-bottom: 8px;
        }
        .subtitle {
            text-align: center;
            color: #888;
            font-size: 13px;
            margin-bottom: 24px;
        }
        .section-title {
            font-size: 14px;
            color: #666;
            margin-bottom: 10px;
            font-weight: 600;
        }
        .wifi-list {
            max-height: 240px;
            overflow-y: auto;
            border: 1px solid #e0e0e0;
            border-radius: 10px;
            margin-bottom: 20px;
        }
        .wifi-item {
            display: flex;
            align-items: center;
            padding: 12px 16px;
            border-bottom: 1px solid #f0f0f0;
            cursor: pointer;
            transition: background 0.15s;
        }
        .wifi-item:last-child { border-bottom: none; }
        .wifi-item:hover { background: #f5f7ff; }
        .wifi-item.selected { background: #e8edff; }
        .wifi-name {
            flex: 1;
            font-size: 15px;
            color: #333;
        }
        .wifi-lock {
            color: #999;
            font-size: 13px;
            margin-right: 8px;
        }
        .wifi-signal {
            font-size: 12px;
            color: #888;
        }
        .signal-strong { color: #4caf50; }
        .signal-medium { color: #ff9800; }
        .signal-weak { color: #f44336; }
        label {
            display: block;
            font-size: 14px;
            color: #666;
            margin-bottom: 6px;
            font-weight: 600;
        }
        input[type="password"] {
            width: 100%;
            padding: 12px 16px;
            border: 1px solid #ddd;
            border-radius: 10px;
            font-size: 15px;
            margin-bottom: 20px;
            outline: none;
            transition: border-color 0.2s;
        }
        input[type="password"]:focus {
            border-color: #4a90d9;
        }
        .btn-row { display: flex; gap: 10px; }
        button {
            flex: 1;
            padding: 12px;
            border: none;
            border-radius: 10px;
            font-size: 15px;
            font-weight: 600;
            cursor: pointer;
            transition: opacity 0.2s;
        }
        button:active { opacity: 0.7; }
        .btn-scan {
            background: #f0f2f5;
            color: #555;
        }
        .btn-connect {
            background: #4a90d9;
            color: #fff;
        }
        .btn-connect:disabled {
            background: #b0c4de;
            cursor: not-allowed;
        }
        .status {
            text-align: center;
            margin-top: 16px;
            font-size: 13px;
            color: #888;
            min-height: 20px;
        }
        .status.error { color: #f44336; }
        .status.success { color: #4caf50; }
        .loading {
            text-align: center;
            padding: 30px;
            color: #888;
        }
    </style>
</head>
<body>
    <div class="card">
        <h1>ESP32 WiFi Config</h1>
        <p class="subtitle">Select a network and enter password</p>

        <div class="section-title">Available Networks</div>
        <div class="wifi-list" id="wifiList">
            <div class="loading">Tap "Scan" to search...</div>
        </div>

        <label for="password">Password</label>
        <input type="password" id="password" placeholder="Enter WiFi password">

        <div class="btn-row">
            <button class="btn-scan" onclick="scanWifi()">Scan</button>
            <button class="btn-connect" id="btnConnect" onclick="connectWifi()" disabled>Connect</button>
        </div>

        <div class="status" id="status"></div>
    </div>

    <script>
        let selectedSSID = '';

        function scanWifi() {
            const list = document.getElementById('wifiList');
            list.innerHTML = '<div class="loading">Scanning...</div>';
            document.getElementById('status').textContent = '';
            document.getElementById('status').className = 'status';

            fetch('/scan')
                .then(r => r.json())
                .then(data => {
                    if (data.length === 0) {
                        list.innerHTML = '<div class="loading">No networks found</div>';
                        return;
                    }
                    list.innerHTML = '';
                    data.forEach((net, i) => {
                        const item = document.createElement('div');
                        item.className = 'wifi-item';
                        item.onclick = () => selectWifi(item, net.ssid);

                        const signalClass = net.rssi > -60 ? 'signal-strong' :
                                           net.rssi > -75 ? 'signal-medium' : 'signal-weak';
                        const bars = net.rssi > -50 ? '▂▄▆█' :
                                    net.rssi > -60 ? '▂▄▆_' :
                                    net.rssi > -75 ? '▂▄__' : '▂___';

                        item.innerHTML = `
                            <span class="wifi-name">${escapeHtml(net.ssid)}</span>
                            ${net.secure ? '<span class="wifi-lock">🔒</span>' : ''}
                            <span class="wifi-signal ${signalClass}">${bars} ${net.rssi}dBm</span>
                        `;
                        list.appendChild(item);
                    });
                })
                .catch(() => {
                    list.innerHTML = '<div class="loading">Scan failed, retry</div>';
                });
        }

        function selectWifi(element, ssid) {
            document.querySelectorAll('.wifi-item').forEach(el => el.classList.remove('selected'));
            element.classList.add('selected');
            selectedSSID = ssid;
            document.getElementById('btnConnect').disabled = false;
        }

        function connectWifi() {
            if (!selectedSSID) return;
            const password = document.getElementById('password').value;
            const status = document.getElementById('status');
            const btn = document.getElementById('btnConnect');

            btn.disabled = true;
            status.textContent = 'Connecting to ' + selectedSSID + '...';
            status.className = 'status';

            fetch('/connect', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({ssid: selectedSSID, password: password})
            })
            .then(r => r.json())
            .then(data => {
                if (data.success) {
                    status.textContent = 'Connected! IP: ' + data.ip;
                    status.className = 'status success';
                } else {
                    status.textContent = 'Failed: ' + (data.error || 'Unknown error');
                    status.className = 'status error';
                    btn.disabled = false;
                }
            })
            .catch(() => {
                status.textContent = 'Connection error';
                status.className = 'status error';
                btn.disabled = false;
            });
        }

        function escapeHtml(text) {
            const div = document.createElement('div');
            div.textContent = text;
            return div.innerHTML;
        }
    </script>
</body>
</html>
)rawliteral";

// ============================
//  WiFi连接结果回调
// ============================
static bool _connectResult = false;
static String _connectIP = "";

void onWifiConnected(const char* ip) {
    _connectResult = true;
    _connectIP = String(ip);
}

void onWifiFailed(const char* reason) {
    _connectResult = false;
    _connectIP = String(reason);
}

// ============================
//  WebServer 实现
// ============================

bool ConfigWebServer::begin() {
    _server = new WebServer(80);

    _server->on("/", HTTP_GET, [this]() { handleRoot(); });
    _server->on("/scan", HTTP_GET, [this]() { handleScan(); });
    _server->on("/connect", HTTP_POST, [this]() { handleConnect(); });
    _server->on("/status", HTTP_GET, [this]() { handleStatus(); });
    _server->onNotFound([this]() { handleNotFound(); });

    _server->begin();
    return true;
}

void ConfigWebServer::stop() {
    if (_server) {
        _server->stop();
        delete _server;
        _server = nullptr;
    }
}

void ConfigWebServer::handleClient() {
    if (_server) _server->handleClient();
}

void ConfigWebServer::handleRoot() {
    sendConfigPage();
}

void ConfigWebServer::handleScan() {
    WifiNetwork networks[20];
    int count = wifiMgr.scanNetworks(networks, 20);

    String json = "[";
    for (int i = 0; i < count; i++) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"ssid\":\"" + String(networks[i].ssid) + "\",";
        json += "\"rssi\":" + String(networks[i].rssi) + ",";
        json += "\"secure\":" + String(networks[i].authType != WIFI_AUTH_OPEN ? "true" : "false");
        json += "}";
    }
    json += "]";

    _server->send(200, "application/json", json);
}

void ConfigWebServer::handleConnect() {
    if (!_server->hasArg("plain")) {
        _server->send(400, "application/json", "{\"error\":\"No data\"}");
        return;
    }

    String body = _server->arg("plain");

    // 简单JSON解析
    int ssidStart = body.indexOf("\"ssid\":\"") + 8;
    int ssidEnd = body.indexOf("\"", ssidStart);
    int passStart = body.indexOf("\"password\":\"") + 12;
    int passEnd = body.indexOf("\"", passStart);

    if (ssidStart < 8 || ssidEnd < 0) {
        _server->send(400, "application/json", "{\"error\":\"Invalid request\"}");
        return;
    }

    String ssid = body.substring(ssidStart, ssidEnd);
    String password = (passStart > 11 && passEnd > passStart) ?
                      body.substring(passStart, passEnd) : "";

    // 先回复客户端
    _server->send(200, "application/json", "{\"success\":true,\"msg\":\"Connecting...\"}");

    // 稍等后断开AP并连接STA
    delay(500);
    wifiMgr.stopAP();
    wifiMgr.connectSTA(ssid.c_str(), password.c_str());
}

void ConfigWebServer::handleStatus() {
    String json = "{\"state\":";
    json += String(wifiMgr.getState());
    json += ",\"ip\":\"" + String(wifiMgr.getIP()) + "\"";
    json += "}";
    _server->send(200, "application/json", json);
}

void ConfigWebServer::handleNotFound() {
    _server->send(404, "text/plain", "404 Not Found");
}

void ConfigWebServer::sendConfigPage() {
    _server->send_P(200, "text/html", CONFIG_PAGE);
}
