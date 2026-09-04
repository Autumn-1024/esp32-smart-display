# 大屏传感采集系统 - ESP32 Smart Display

## 功能说明

ESP32 上电自动连接已保存的 WiFi，连接成功后启动 Web 管理页面，可实时查看传感器数据。

### 工作流程

```
上电 → 检查已保存WiFi → 有：直接连接 → 显示IP → 启动管理页面
                      → 无：AP配网模式 → 手机连接热点 → Web配置WiFi → 连接
```

### 管理页面

访问板子 IP 地址即可查看实时监控数据：

- **Screen** — 温度（DHT11）、湿度（DHT11）
- **Phase Temperature** — A/B/C 相温度（DS18B20）
- **Three-phase Voltage** — A/B/C 相电压（模拟）
- **Three-phase Current** — A/B/C 相电流（模拟）
- **Total Power** — 总功率（自动计算）

API 接口：`GET http://<IP>/api/data`

## 硬件连接

| 外设 | 引脚 | 说明 |
|------|------|------|
| OLED SDA | GPIO 21 | 0.96寸 IIC |
| OLED SCL | GPIO 22 | 0.96寸 IIC |
| 按键0 | GPIO 25 | 长按进入配网 |
| 按键1 | GPIO 26 | 预留 |
| 按键2 | GPIO 32 | 预留 |
| 按键3 | GPIO 33 | 预留 |
| DHT11 | GPIO 15 | 温湿度传感器 |
| DS18B20 A相 | GPIO 4 | 相线温度 |
| DS18B20 B相 | GPIO 13 | 相线温度 |
| DS18B20 C相 | GPIO 14 | 相线温度 |

### DHT11 接线

```
DHT11 VCC → 3.3V
DHT11 GND → GND
DHT11 SIG → GPIO 15
```

### DS18B20 接线（每路）

```
DS18B20 VCC → 3.3V
DS18B20 GND → GND
DS18B20 DAT → 对应GPIO（使用ESP32内部上拉）
```

## 按键功能

| 按键 | 功能 |
|------|------|
| 按键0 长按 | 强制进入 AP 配网模式 |
| 按键0 短按 | 连接中可取消并重新配网 |
| 按键1/2/3 | 预留 |

## 开发环境

- **IDE**: Arduino CLI v1.5.0
- **核心包**: esp32:esp32 v2.0.5
- **依赖库**:
  - Adafruit SSD1306
  - Adafruit GFX Library
  - Adafruit BusIO
  - DHT sensor library
  - OneWire
  - DallasTemperature

## 编译烧录

```bash
# 编译
arduino-cli compile --fqbn esp32:esp32:esp32 .

# 烧录（修改COM口）
arduino-cli upload --fqbn esp32:esp32:esp32 --port COMxx --input-dir build/
```

## 使用方法

### 首次配网

1. 烧录固件后上电
2. OLED 显示热点名称 `ESP32-Setup_XXXX` 和管理地址
3. 手机连接该热点（无密码）
4. 浏览器访问 `192.168.4.1`
5. 选择 WiFi 网络，输入密码，点击 Connect
6. 连接成功后 OLED 显示 IP 地址

### 正常使用

1. 上电自动连接已保存的 WiFi
2. OLED 显示 IP 地址
3. 浏览器访问 IP 查看管理页面
4. 数据每 2 秒自动刷新

### 重新配网

- 长按按键0 进入配网模式
- 或等待 WiFi 连接失败自动回退

## API 接口

### GET /api/data

返回 JSON 格式传感器数据：

```json
{
  "temp": 25.8,
  "humidity": 31.4,
  "phaseTempA": 21.7,
  "phaseTempB": 22.1,
  "phaseTempC": 21.9,
  "voltageUa": 225.6,
  "voltageUb": 223.4,
  "voltageUc": 226.8,
  "currentIa": 12.45,
  "currentIb": 11.82,
  "currentIc": 13.07,
  "power": 8523.4
}
```

### 字段说明

| 字段 | 类型 | 单位 | 来源 |
|------|------|------|------|
| temp | float | °C | DHT11 |
| humidity | float | % | DHT11 |
| phaseTempA | float | °C | DS18B20 GPIO4 |
| phaseTempB | float | °C | DS18B20 GPIO13 |
| phaseTempC | float | °C | DS18B20 GPIO14 |
| voltageUa | float | V | 模拟 |
| voltageUb | float | V | 模拟 |
| voltageUc | float | V | 模拟 |
| currentIa | float | A | 模拟 |
| currentIb | float | A | 模拟 |
| currentIc | float | A | 模拟 |
| power | float | W | 计算 |

## 文件结构

```
esp32-smart-display/
├── esp32-smart-display.ino  # 主程序
├── config.h                 # 引脚、参数配置
├── oled.h / oled.cpp        # OLED显示模块
├── wifi_manager.h / .cpp    # WiFi管理（AP/STA）
├── web_server.h / .cpp      # Web服务器（配网+管理）
├── button.h / button.cpp    # 按键管理
├── platformio.ini           # PlatformIO配置
└── README.md
```

## TODO

- [ ] 电压电流真实数据采集（RS485 Modbus）
- [ ] WiFi断线自动重连
- [ ] OTA远程升级
- [ ] 数据本地存储（SD卡/Flash）
- [ ] 按键1/2/3功能定义
