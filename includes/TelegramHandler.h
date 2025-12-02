#pragma once

#ifndef TELAGRAMHANDLER.H
#define TELEGRAMHANDLER .H
#include <Arduino.h>
#include <ESP32Servo.h>
#include <AsyncTelegram2.h>
#include "UserManager.h"
#include "credentials.h"
#include "OTPHandler.h"

// --- STATE MACHINE OTENTIKASI ---
enum AuthState
{
    AUTH_IDLE,
    AUTH_WAITING_FOR_OTP_COMMAND,
    AUTH_WAITING_FOR_KEYPAD
};

// --- STATE MACHINE STATUS PINTU ---
enum DoorState
{
    TERTUTUP,
    TERBUKA
};

// --- STATE MACHINE AKSI PINTU ---
enum DoorActionState
{
    ACTION_IDLE,    // Pintu tidak melakukan apa-apa
    ACTION_OPENING, // Pintu sedang bergerak membuka
    ACTION_OPEN,    // Pintu telah terbuka (menunggu auto-close)
    ACTION_CLOSING  // Pintu sedang bergerak menutup
};

class TelegramHandler
{
private:
    AsyncTelegram2 *botPtr;
    UserManager *userManagerPtr;
    Credentials *credPtr;
    OTPHandler *otpHandlerPtr;

    Servo myServo;
    int doorSensor = 0;

    // State Otentikasi
    AuthState currentAuthState = AUTH_IDLE;
    int64_t activeAuthChatId = 0; // ID Telegram user yang terdeteksi BLE/sedang otentikasi
    const unsigned long AUTH_TIMEOUT_MS = 40000;
    unsigned long authStartTime = 0;
    bool warningSent = false;

    DoorState doorState = TERTUTUP;
    DoorActionState currentDoorActionState = ACTION_IDLE;
    unsigned long actionStartTime = 0;
    bool readDoorSensor = false;

    // Internal Methods
    void handleCommand(const int64_t chatId, const String &command);
    void handleOtpCommand(const int64_t chatId, const int64_t proximityId);
    DoorState readDoorState();

public:
    TelegramHandler(AsyncTelegram2 *bot, UserManager *userManager, Credentials *credentials, OTPHandler *otpHandler, int servoPin, int DoorSensorPin)
        : botPtr(bot), userManagerPtr(userManager), credPtr(credentials), otpHandlerPtr(otpHandler)
    {
        myServo.attach(servoPin);
        myServo.write(0);
        pinMode(DoorSensorPin, INPUT_PULLUP);
        doorSensor = DoorSensorPin;
    }

    void handleNewMessages(int64_t proximityId);

    // --- Keypad/Main Loop Getters and Setters ---
    AuthState getCurrentAuthState() const { return currentAuthState; }
    void setCurrentAuthState(AuthState newState) { currentAuthState = newState; }
    int64_t getActiveAuthChatId() const { return activeAuthChatId; }

    void notifyAuthResult(bool success);
    void runDoorLogic();
    void triggerDoorOpen(bool success);

    void checkAuthTimeout();
    DoorState getDoorState();
};

#endif