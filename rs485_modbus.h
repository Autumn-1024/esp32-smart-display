#ifndef RS485_MODBUS_H
#define RS485_MODBUS_H

#include <Arduino.h>

class RS485Modbus {
public:
    bool begin();
    
    // 读取6路电流，返回是否成功
    // currents[0..5] 存放6路电流值（单位A）
    bool readCurrents(float* currents);

private:
    // CRC16 计算
    uint16_t crc16(const uint8_t* data, uint8_t len);
    
    // 发送Modbus请求
    void sendRequest();
    
    // 接收响应
    bool receiveResponse(uint8_t* buf, uint8_t* len);
    
    // 设置收发模式
    void setTxMode();
    void setRxMode();
};

extern RS485Modbus rs485;

#endif // RS485_MODBUS_H
