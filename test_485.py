#!/usr/bin/env python3
"""
测试六路电流互感器 Modbus RTU 通信
协议：RTU模式，从机地址0x1E，功能码0x03，起始寄存器0x0056，读6个寄存器
"""

import serial
import struct
import time

# Modbus CRC16 计算
def crc16(data):
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x0001:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc

# 构造读取6路电流的请求
def build_read_command(addr=0x1E, start_reg=0x0056, count=6):
    cmd = struct.pack('>BBHH', addr, 0x03, start_reg, count)
    crc = crc16(cmd)
    cmd += struct.pack('<H', crc)
    return cmd

# 解析响应数据
def parse_response(data):
    if len(data) < 5:
        return None
    
    addr = data[0]
    func = data[1]
    
    if func & 0x80:  # 错误响应
        print(f"错误响应: {data.hex()}")
        return None
    
    byte_count = data[2]
    values = []
    
    for i in range(0, byte_count, 2):
        raw = struct.unpack('>H', data[3+i:5+i])[0]
        current = raw * 0.1  # 换算为安培
        values.append({'raw_hex': f'0x{raw:04X}', 'raw_dec': raw, 'current_a': current})
    
    return values

# 主程序
print("=" * 50)
print("六路电流互感器 Modbus RTU 通信测试")
print("=" * 50)

# 尝试不同的COM口和波特率
ports = ['COM22', 'COM20', 'COM21']
baudrates = [9600, 19200, 115200]

command = build_read_command()
print(f"发送命令: {command.hex()}")
print(f"从机地址: 0x1E (30)")
print(f"功能码: 0x03")
print(f"起始寄存器: 0x0056")
print(f"寄存器数量: 6")
print()

for port in ports:
    for baud in baudrates:
        try:
            print(f"尝试 {port} @ {baud} 波特率...")
            ser = serial.Serial(port, baud, timeout=2)
            ser.reset_input_buffer()
            
            # 发送命令
            ser.write(command)
            time.sleep(0.5)
            
            # 读取响应
            if ser.in_waiting > 0:
                response = ser.read(ser.in_waiting)
                print(f"  收到响应: {response.hex()}")
                
                values = parse_response(response)
                if values:
                    print(f"  ✓ 通信成功！解析到 {len(values)} 路电流：")
                    labels = ['Ia', 'Ib', 'Ic', 'I0', 'I1', 'I2']  # 6路标签
                    for i, v in enumerate(values):
                        label = labels[i] if i < len(labels) else f'CH{i+1}'
                        print(f"    {label}: {v['raw_hex']} → {v['raw_dec']} → {v['current_a']:.1f}A")
                else:
                    print(f"  ✗ 数据解析失败")
                
                ser.close()
                print()
                # 如果成功了就不用继续尝试
                if values:
                    exit(0)
            else:
                print(f"  无响应")
                ser.close()
                
        except serial.SerialException as e:
            print(f"  端口错误: {e}")
        except Exception as e:
            print(f"  错误: {e}")
        print()

print("所有端口和波特率都尝试完毕")
print("请检查：")
print("1. 485模块是否正确连接到电脑")
print("2. A/B线是否接反")
print("3. 波特率是否正确")
print("4. 从机地址是否为 0x1E")
