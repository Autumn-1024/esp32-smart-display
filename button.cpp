#include "button.h"

void ButtonManager::begin() {
    for (int i = 0; i < 4; i++) {
        pinMode(_pins[i], INPUT_PULLUP);  // 外部上拉，按下为LOW
        _lastState[i] = HIGH;
        _pressTime[i] = 0;
        _pendingEvent[i] = BTN_NONE;
    }
}

void ButtonManager::update() {
    unsigned long now = millis();

    for (int i = 0; i < 4; i++) {
        bool currentState = digitalRead(_pins[i]);

        // 检测下降沿（按下）
        if (_lastState[i] == HIGH && currentState == LOW) {
            _pressTime[i] = now;
        }

        // 检测上升沿（释放）
        if (_lastState[i] == LOW && currentState == HIGH) {
            unsigned long held = now - _pressTime[i];
            if (held > 1000) {
                _pendingEvent[i] = BTN_LONG_PRESS;
            } else if (held > BTN_DEBOUNCE_MS) {
                _pendingEvent[i] = BTN_PRESS;
            }
        }

        _lastState[i] = currentState;
    }
}

ButtonEvent ButtonManager::getEvent(int buttonIndex) {
    if (buttonIndex < 0 || buttonIndex > 3) return BTN_NONE;
    ButtonEvent e = _pendingEvent[buttonIndex];
    _pendingEvent[buttonIndex] = BTN_NONE;  // 读后清除
    return e;
}

bool ButtonManager::isPressed(int buttonIndex) {
    if (buttonIndex < 0 || buttonIndex > 3) return false;
    return digitalRead(_pins[buttonIndex]) == LOW;
}
