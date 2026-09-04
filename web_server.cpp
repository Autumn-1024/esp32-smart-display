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
    <title>WiFi Config</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: -apple-system, sans-serif; background: #f0f2f5; display: flex; align-items: center; justify-content: center; min-height: 100vh; }
        .card { background: #fff; border-radius: 16px; box-shadow: 0 4px 20px rgba(0,0,0,0.08); padding: 28px; width: 92%; max-width: 380px; }
        h1 { text-align: center; color: #333; font-size: 20px; margin-bottom: 6px; }
        .sub { text-align: center; color: #999; font-size: 12px; margin-bottom: 20px; }
        .label { font-size: 13px; color: #666; margin-bottom: 8px; font-weight: 600; }
        .wifi-list { max-height: 220px; overflow-y: auto; border: 1px solid #e0e0e0; border-radius: 10px; margin-bottom: 16px; }
        .wifi-item { display: flex; align-items: center; padding: 11px 14px; border-bottom: 1px solid #f0f0f0; cursor: pointer; transition: background 0.15s; }
        .wifi-item:last-child { border-bottom: none; }
        .wifi-item:hover { background: #f5f7ff; }
        .wifi-item.sel { background: #e8edff; }
        .wifi-name { flex: 1; font-size: 14px; color: #333; }
        .wifi-lock { color: #999; font-size: 12px; margin-right: 6px; }
        .wifi-sig { font-size: 11px; color: #888; }
        input[type="password"] { width: 100%; padding: 11px 14px; border: 1px solid #ddd; border-radius: 10px; font-size: 14px; margin-bottom: 16px; outline: none; }
        input:focus { border-color: #4a90d9; }
        .btn-row { display: flex; gap: 10px; }
        button { flex: 1; padding: 11px; border: none; border-radius: 10px; font-size: 14px; font-weight: 600; cursor: pointer; }
        button:active { opacity: 0.7; }
        .btn-scan { background: #f0f2f5; color: #555; }
        .btn-connect { background: #4a90d9; color: #fff; }
        .btn-connect:disabled { background: #b0c4de; }
        .status { text-align: center; margin-top: 14px; font-size: 12px; color: #888; min-height: 18px; }
        .status.err { color: #f44336; }
        .status.ok { color: #4caf50; }
        .loading { text-align: center; padding: 25px; color: #888; }
    </style>
</head>
<body>
    <div class="card">
        <h1>WiFi Config</h1>
        <p class="sub">Select network & enter password</p>
        <div class="label">Available Networks</div>
        <div class="wifi-list" id="wifiList"><div class="loading">Tap Scan...</div></div>
        <label for="pwd" style="font-size:13px;color:#666;font-weight:600;display:block;margin-bottom:6px;">Password</label>
        <input type="password" id="pwd" placeholder="WiFi password">
        <div class="btn-row">
            <button class="btn-scan" onclick="scan()">Scan</button>
            <button class="btn-connect" id="btnC" onclick="connect()" disabled>Connect</button>
        </div>
        <div class="status" id="st"></div>
    </div>
    <script>
        let sel='';
        function scan(){
            const l=document.getElementById('wifiList');
            l.innerHTML='<div class="loading">Scanning...</div>';
            document.getElementById('st').textContent='';
            fetch('/scan').then(r=>r.json()).then(d=>{
                if(!d.length){l.innerHTML='<div class="loading">No networks</div>';return;}
                l.innerHTML='';
                d.forEach(n=>{
                    const el=document.createElement('div');
                    el.className='wifi-item';
                    el.onclick=()=>{document.querySelectorAll('.wifi-item').forEach(e=>e.classList.remove('sel'));el.classList.add('sel');sel=n.ssid;document.getElementById('btnC').disabled=false;};
                    const sc=n.rssi>-60?'color:#4caf50':n.rssi>-75?'color:#ff9800':'color:#f44336';
                    el.innerHTML='<span class="wifi-name">'+esc(n.ssid)+'</span>'+(n.secure?'<span class="wifi-lock">🔒</span>':'')+'<span class="wifi-sig" style="'+sc+'">'+n.rssi+'dBm</span>';
                    l.appendChild(el);
                });
            }).catch(()=>{l.innerHTML='<div class="loading">Failed</div>';});
        }
        function connect(){
            if(!sel)return;
            const p=document.getElementById('pwd').value;
            const s=document.getElementById('st');
            document.getElementById('btnC').disabled=true;
            s.textContent='Connecting to '+sel+'...';s.className='status';
            fetch('/connect',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid:sel,password:p})}).then(r=>r.json()).then(d=>{
                if(d.success){s.textContent='Connected!';s.className='status ok';}
                else{s.textContent='Failed: '+(d.error||'Error');s.className='status err';document.getElementById('btnC').disabled=false;}
            }).catch(()=>{s.textContent='Error';s.className='status err';document.getElementById('btnC').disabled=false;});
        }
        function esc(t){const d=document.createElement('div');d.textContent=t;return d.innerHTML;}
    </script>
</body>
</html>
)rawliteral";

// ============================
//  管理页面 HTML
// ============================
static const char MANAGE_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>System Monitor</title>
    <style>
        *{margin:0;padding:0;box-sizing:border-box;}
        body{font-family:-apple-system,sans-serif;background:#f0f2f5;padding:16px;max-width:480px;margin:0 auto;}
        h1{text-align:center;color:#333;font-size:18px;margin:12px 0 16px;}
        .sec{background:#fff;border-radius:12px;padding:16px;margin-bottom:12px;box-shadow:0 2px 10px rgba(0,0,0,0.05);}
        .sec-title{font-size:13px;font-weight:700;color:#4a90d9;margin-bottom:10px;padding-bottom:6px;border-bottom:1px solid #f0f0f0;}
        .row{display:flex;justify-content:space-between;padding:6px 0;font-size:14px;}
        .row:not(:last-child){border-bottom:1px solid #f8f8f8;}
        .lbl{color:#666;}
        .val{font-weight:600;color:#333;}
        .unit{color:#999;font-size:12px;margin-left:2px;}
        .ok{color:#4caf50;}
        .warn{color:#ff9800;}
        .err{color:#f44336;}
        .footer{text-align:center;color:#aaa;font-size:11px;margin-top:16px;}
    </style>
</head>
<body>
    <h1>System Monitor</h1>

    <div class="sec">
        <div class="sec-title">📺 Screen</div>
        <div class="row"><span class="lbl">Temperature</span><span class="val" id="sT">--<span class="unit">°C</span></span></div>
        <div class="row"><span class="lbl">Humidity</span><span class="val" id="sH">--<span class="unit">%</span></span></div>
    </div>

    <div class="sec">
        <div class="sec-title">🌡️ Phase Temperature</div>
        <div class="row"><span class="lbl">Phase A</span><span class="val" id="pAT">--<span class="unit">°C</span></span></div>
        <div class="row"><span class="lbl">Phase B</span><span class="val" id="pBT">--<span class="unit">°C</span></span></div>
        <div class="row"><span class="lbl">Phase C</span><span class="val" id="pCT">--<span class="unit">°C</span></span></div>
    </div>

    <div class="sec">
        <div class="sec-title">⚡ Three-phase Voltage</div>
        <div class="row"><span class="lbl">Phase A</span><span class="val" id="pAV">--<span class="unit">V</span></span></div>
        <div class="row"><span class="lbl">Phase B</span><span class="val" id="pBV">--<span class="unit">V</span></span></div>
        <div class="row"><span class="lbl">Phase C</span><span class="val" id="pCV">--<span class="unit">V</span></span></div>
    </div>

    <div class="sec">
        <div class="sec-title">🔌 Three-phase Current</div>
        <div class="row"><span class="lbl">Phase A</span><span class="val" id="pAC">--<span class="unit">A</span></span></div>
        <div class="row"><span class="lbl">Phase B</span><span class="val" id="pBC">--<span class="unit">A</span></span></div>
        <div class="row"><span class="lbl">Phase C</span><span class="val" id="pCC">--<span class="unit">A</span></span></div>
    </div>

    <div class="footer">ESP32 Smart Display | Auto-refresh 2s</div>

    <script>
        function cls(v,min,max){
            if(v<min||v>max)return 'err';
            if(v<min*1.1||v>max*0.9)return 'warn';
            return 'ok';
        }
        function upd(){
            fetch('/api/data').then(r=>r.json()).then(d=>{
                document.getElementById('sT').innerHTML=d.screenT.toFixed(1)+'<span class="unit">°C</span>';
                document.getElementById('sH').innerHTML=d.screenH.toFixed(1)+'<span class="unit">%</span>';
                document.getElementById('pAT').innerHTML=d.phAT.toFixed(1)+'<span class="unit">°C</span>';
                document.getElementById('pBT').innerHTML=d.phBT.toFixed(1)+'<span class="unit">°C</span>';
                document.getElementById('pCT').innerHTML=d.phCT.toFixed(1)+'<span class="unit">°C</span>';
                document.getElementById('pAV').innerHTML=d.phAV.toFixed(1)+'<span class="unit">V</span>';
                document.getElementById('pBV').innerHTML=d.phBV.toFixed(1)+'<span class="unit">V</span>';
                document.getElementById('pCV').innerHTML=d.phCV.toFixed(1)+'<span class="unit">V</span>';
                document.getElementById('pAC').innerHTML=d.phAC.toFixed(2)+'<span class="unit">A</span>';
                document.getElementById('pBC').innerHTML=d.phBC.toFixed(2)+'<span class="unit">A</span>';
                document.getElementById('pCC').innerHTML=d.phCC.toFixed(2)+'<span class="unit">A</span>';

                // 颜色标注
                document.getElementById('sT').className='val '+cls(d.screenT,15,45);
                document.getElementById('sH').className='val '+cls(d.screenH,20,90);
                document.getElementById('pAT').className='val '+cls(d.phAT,20,80);
                document.getElementById('pBT').className='val '+cls(d.phBT,20,80);
                document.getElementById('pCT').className='val '+cls(d.phCT,20,80);
                document.getElementById('pAV').className='val '+cls(d.phAV,200,250);
                document.getElementById('pBV').className='val '+cls(d.phBV,200,250);
                document.getElementById('pCV').className='val '+cls(d.phCV,200,250);
                document.getElementById('pAC').className='val '+cls(d.phAC,0,30);
                document.getElementById('pBC').className='val '+cls(d.phBC,0,30);
                document.getElementById('pCC').className='val '+cls(d.phCC,0,30);
            }).catch(()=>{});
        }
        upd();
        setInterval(upd,2000);
    </script>
</body>
</html>
)rawliteral";

// ============================
//  WebServer 实现
// ============================

bool ConfigWebServer::begin() {
    _server = new WebServer(80);
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

// ---- 配网模式 ----

void ConfigWebServer::startConfigMode() {
    _manageMode = false;
    _server->on("/", HTTP_GET, [this]() { handleRoot(); });
    _server->on("/scan", HTTP_GET, [this]() { handleScan(); });
    _server->on("/connect", HTTP_POST, [this]() { handleConnect(); });
    Serial.println("Config web server started");
}

// ---- 管理模式 ----

void ConfigWebServer::startManageMode() {
    _manageMode = true;
    generateSensorData();

    _server->on("/", HTTP_GET, [this]() { handleManageRoot(); });
    _server->on("/api/data", HTTP_GET, [this]() { handleApiData(); });
    Serial.println("Manage web server started");
}

// ---- 配网页面 ----

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
    int ssidStart = body.indexOf("\"ssid\":\"") + 8;
    int ssidEnd = body.indexOf("\"", ssidStart);
    int passStart = body.indexOf("\"password\":\"") + 12;
    int passEnd = body.indexOf("\"", passStart);

    if (ssidStart < 8 || ssidEnd < 0) {
        _server->send(400, "application/json", "{\"error\":\"Invalid\"}");
        return;
    }

    String ssid = body.substring(ssidStart, ssidEnd);
    String password = (passStart > 11 && passEnd > passStart) ? body.substring(passStart, passEnd) : "";

    _server->send(200, "application/json", "{\"success\":true}");

    delay(500);
    wifiMgr.stopAP();
    wifiMgr.connectSTA(ssid.c_str(), password.c_str());
}

// ---- 管理页面 ----

void ConfigWebServer::handleManageRoot() {
    sendManagePage();
}

void ConfigWebServer::handleApiData() {
    // 每次请求生成新的随机数据
    generateSensorData();

    String json = "{";
    json += "\"screenT\":" + String(screenTemp, 1) + ",";
    json += "\"screenH\":" + String(screenHumi, 1) + ",";
    json += "\"phAT\":" + String(phaseATemp, 1) + ",";
    json += "\"phBT\":" + String(phaseBTemp, 1) + ",";
    json += "\"phCT\":" + String(phaseCTemp, 1) + ",";
    json += "\"phAV\":" + String(phaseAVolt, 1) + ",";
    json += "\"phBV\":" + String(phaseBVolt, 1) + ",";
    json += "\"phCV\":" + String(phaseCVolt, 1) + ",";
    json += "\"phAC\":" + String(phaseACurr, 2) + ",";
    json += "\"phBC\":" + String(phaseBCurr, 2) + ",";
    json += "\"phCC\":" + String(phaseCCurr, 2);
    json += "}";

    _server->send(200, "application/json", json);
}

void ConfigWebServer::generateSensorData() {
    screenTemp = 25.0 + random(-50, 100) / 10.0;   // 20~35°C
    screenHumi = 45.0 + random(-150, 250) / 10.0;   // 30~70%
    phaseATemp = 35.0 + random(0, 450) / 10.0;      // 35~80°C
    phaseBTemp = 35.0 + random(0, 450) / 10.0;
    phaseCTemp = 35.0 + random(0, 450) / 10.0;
    phaseAVolt = 215.0 + random(0, 300) / 10.0;     // 215~245V
    phaseBVolt = 215.0 + random(0, 300) / 10.0;
    phaseCVolt = 215.0 + random(0, 300) / 10.0;
    phaseACurr = 5.0 + random(0, 2000) / 100.0;     // 5~25A
    phaseBCurr = 5.0 + random(0, 2000) / 100.0;
    phaseCCurr = 5.0 + random(0, 2000) / 100.0;
}

// ---- 页面发送 ----

void ConfigWebServer::sendConfigPage() {
    _server->send_P(200, "text/html", CONFIG_PAGE);
}

void ConfigWebServer::sendManagePage() {
    _server->send_P(200, "text/html", MANAGE_PAGE);
}

void ConfigWebServer::handleNotFound() {
    _server->send(404, "text/plain", "404");
}
