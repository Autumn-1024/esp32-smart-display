# 大屏传感采集系统 - ESP32 Smart Display

## 功能说明

ESP32 上电后自动进入 AP 配网模式，用户通过手机连接热点后访问 Web 页面配置 WiFi。

### 工作流程

```
上电 → OLED显示AP信息 → 用户连接热点 → 浏览器访问192.168.4.1
→ 扫描WiFi → 选择网络 → 输入密码 → 连接 → OLED显示成功
```

## 硬件连接

| 外设 | 引脚 |
|------|------|
| 按键0 | GPIO 25 |
| 按键1 | GPIO 26 |
| 按键2 | GPIO 32 |
| 按键3 | GPIO 33 |
| OLED SDA | GPIO 21 |
| OLED SCL | GPIO 22 |

## 开发环境

- **IDE**: Arduino CLI
- **核心包**: esp32:esp32 v2.0.5
- **依赖库**:
  - Adafruit SSD1306
  - Adafruit GFX Library
  - Adafruit BusIO

## 编译烧录

```bash
# 编译
arduino-cli compile --fqbn esp32:esp32:esp32 src/ --build-property "build.extra_flags=-Iinclude -Iconfig"

# 烧录（修改COM口）
arduino-cli upload --fqbn esp32:esp32:esp32 --port COMxx src/
```

## 使用方法

1. 烧录固件后上电
2. OLED 显示热点名称 `ESP32-Setup_XXXX` 和地址 `192.168.4.1`
3. 手机连接该热点（无密码）
4. 浏览器访问 `192.168.4.1`
5. 页面自动扫描周围 WiFi，选择目标网络
6. 输入密码，点击 Connect
7. 连接成功后 OLED 显示 IP 地址

## 文件结构

```
esp32-smart-display/
├── config/
│   └── config.h          # 引脚、WiFi参数配置
├── include/
│   ├── oled.h            # OLED显示模块
│   ├── wifi_manager.h    # WiFi管理模块
│   ├── web_server.h      # Web配网服务器
│   └── button.h          # 按键管理
├── src/
│   ├── main.cpp          # 主程序入口
│   ├── oled.cpp          # OLED显示实现
│   ├── wifi_manager.cpp  # WiFi管理实现
│   ├── web_server.cpp    # Web配网页面
│   └── button.cpp        # 按键处理
├── platformio.ini        # PlatformIO配置（可选）
└── README.md             # 本文件
```

## TODO

- [ ] 按键功能定义
- [ ] 传感器数据采集
- [ ] WiFi断线重连
- [ ] OTA远程升级
