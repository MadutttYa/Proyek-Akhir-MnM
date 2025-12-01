#pragma once
#include <Arduino.h>
#include <Keypad.h>
#include "OTPHandler.h"
#include "TelegramHandler.h"
#include "BuzzerHandler.h"
#include "credentials.h"

// Konstanta Keypad
const byte ROWS = 4; // Jumlah baris
const byte COLS = 4; // Jumlah kolom
const char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

class KeypadHandler
{
private:
    Keypad customKeypad;
    OTPHandler *otpHandlerPtr;
    TelegramHandler *telegramHandlerPtr;
    BuzzerHandler *buzzerHandlerPtr;
    Credentials *credPtr;

    String inputCode = "";
    const int MAX_DIGITS = 6;

public:
    KeypadHandler(
        byte rowPins[],
        byte colPins[],
        OTPHandler *otpHandler,
        TelegramHandler *telegramHandler,
        BuzzerHandler *buzzerHandler,
        Credentials *credential);

    void checkInput();
    void handleFullInput();
    void resetInput();
};