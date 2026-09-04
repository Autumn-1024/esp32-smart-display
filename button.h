#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
#include "config.h"

// 按键事件类型
enum ButtonEvent {
    BTN_NONE,
    BTN_PRESS,      // 按下
    BTN_RELEASE,    // 释放
    BTN_LONG_PRESS  // 长按（>1秒）
};

class ButtonManager {
public:
    void begin();
    void update();

    // 获取最新按键事件
    ButtonEvent getEvent(int buttonIndex);

    // 检查按键是否按下
    bool isPressed(int buttonIndex);

private:
    bool _lastState[4] = {HIGH, HIGH, HIGH, HIGH};
    unsigned long _pressTime[4] = {0, 0, 0, 0};
    ButtonEvent _pendingEvent[4] = {BTN_NONE, BTN_NONE, BTN_NONE, BTN_NONE};

    const int _pins[4] = {BTN_0, BTN_1, BTN_2, BTN_3};
};

#endif // BUTTON_H
