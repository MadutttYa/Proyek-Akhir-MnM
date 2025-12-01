#pragma once
#include <Arduino.h>

class BuzzerHandler
{
private:
    int buzzerPin;

public:
    BuzzerHandler(int pin) : buzzerPin(pin)
    {
        pinMode(buzzerPin, OUTPUT);
    }

    void beepSuccess() const;
    void beepFailure() const;
    void beepKeypress() const;
};