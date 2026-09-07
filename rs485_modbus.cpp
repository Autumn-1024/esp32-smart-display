#include "rs485_modbus.h"
#include "config.h"
#include <HardwareSerial.h>

RS485Modbus rs485;

// 使用 Serial2 作为 RS485 串口
// ESP32 Serial2 默认: RX=16, TX=17
static HardwareSerial ModbusSerial(2);

bool RS485Modbus::begin() {
    // 初始化串口
    ModbusSerial.begin(RS485_BAUD, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
    
    // 初始化 DE/RE 控制引脚
    pinMode(RS485_DE_PIN, OUTPUT);
    setRxMode();
    
    Serial.println("RS485 Modbus initialized");
    return true;
}

void RS485Modbus::setTxMode() {
    digitalWrite(RS485_DE_PIN, HIGH);  // 发送模式
}

void RS485Modbus::setRxMode() {
    digitalWrite(RS485_DE_PIN, LOW);   // 接收模式
}

uint16_t RS485Modbus::crc16(const uint8_t* data, uint8_t len) {
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

void RS485Modbus::sendRequest() {
    uint8_t cmd[8];
    
    // 构造请求：地址 + 功能码 + 起始地址 + 寄存器数 + CRC
    cmd[0] = MODBUS_ADDR;
    cmd[1] = 0x03;
    cmd[2] = (MODBUS_REG >> 8) & 0xFF;
    cmd[3] = MODBUS_REG & 0xFF;
    cmd[4] = (MODBUS_COUNT >> 8) & 0xFF;
    cmd[5] = MODBUS_COUNT & 0xFF;
    
    // 计算 CRC
    uint16_t crc = crc16(cmd, 6);
    cmd[6] = crc & 0xFF;        // CRC 低字节
    cmd[7] = (crc >> 8) & 0xFF; // CRC 高字节
    
    // 切换到发送模式
    setTxMode();
    delay(1);
    
    // 发送数据
    ModbusSerial.write(cmd, 8);
    ModbusSerial.flush();
    
    // 切换回接收模式
    delay(1);
    setRxMode();
    
    Serial.print("RS485 TX: ");
    for (int i = 0; i < 8; i++) {
        Serial.printf("%02X ", cmd[i]);
    }
    Serial.println();
}

bool RS485Modbus::receiveResponse(uint8_t* buf, uint8_t* len) {
    *len = 0;
    
    // 等待数据到达
    unsigned long start = millis();
    while (millis() - start < 1000) {
        if (ModbusSerial.available()) {
            delay(10);  // 等待完整帧到达
            while (ModbusSerial.available() && *len < 32) {
                buf[*len] = ModbusSerial.read();
                (*len)++;
            }
            
            Serial.print("RS485 RX: ");
            for (int i = 0; i < *len; i++) {
                Serial.printf("%02X ", buf[i]);
            }
            Serial.println();
            
            return *len > 0;
        }
    }
    
    Serial.println("RS485 RX: 无响应");
    return false;
}

bool RS485Modbus::readCurrents(float* currents) {
    // 初始化电流值为 -1（表示读取失败）
    for (int i = 0; i < MODBUS_COUNT; i++) {
        currents[i] = -1.0;
    }
    
    // 发送请求
    sendRequest();
    
    // 接收响应
    uint8_t buf[32];
    uint8_t len = 0;
    
    if (!receiveResponse(buf, &len)) {
        return false;
    }
    
    // 验证响应
    if (len < 5 || buf[0] != MODBUS_ADDR || buf[1] != 0x03) {
        Serial.println("RS485: 响应格式错误");
        return false;
    }
    
    // 检查错误响应（功能码最高位为1）
    if (buf[1] & 0x80) {
        Serial.printf("RS485: 错误码 %02X\n", buf[2]);
        return false;
    }
    
    // 解析数据
    uint8_t byteCount = buf[2];
    if (byteCount != MODBUS_COUNT * 2) {
        Serial.printf("RS485: 数据长度错误 %d\n", byteCount);
        return false;
    }
    
    // 验证CRC
    uint16_t recvCRC = buf[len - 2] | (buf[len - 1] << 8);
    uint16_t calcCRC = crc16(buf, len - 2);
    if (recvCRC != calcCRC) {
        Serial.printf("RS485: CRC错误 recv=%04X calc=%04X\n", recvCRC, calcCRC);
        return false;
    }
    
    // 提取6路电流值
    for (int i = 0; i < MODBUS_COUNT; i++) {
        uint16_t raw = (buf[3 + i * 2] << 8) | buf[4 + i * 2];
        currents[i] = raw * CURRENT_RESOLUTION;
    }
    
    Serial.printf("RS485: I1=%.1f I2=%.1f I3=%.1f I4=%.1f I5=%.1f I6=%.1f\n",
                  currents[0], currents[1], currents[2],
                  currents[3], currents[4], currents[5]);
    
    return true;
}
