// Internal Library Used
#include <Arduino.h>
#include <WiFi.h>
#include <TOTP.h>
#include <Keypad.h>
#include <ESP32Servo.h>
#include <NTPClient.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <AsyncTelegram2.h>

// External Helper
#include <includes/credentials.h>

#include <includes/User.h>
#include <includes/UserManager.h>
#include <includes/UserManager.cpp>

#include <includes/BLEHandler.h>
#include <includes/BLEHandler.cpp>
#include <includes/WiFiHandler.h>
#include <includes/WiFiHandler.cpp>

#include <includes/OTPHandler.h>
#include <includes/OTPHandler.cpp>

#include <includes/TelegramHandler.h>
#include <includes/TelegramHandler.cpp>

#include <includes/BuzzerHandler.h>
#include <includes/BuzzerHandler.cpp>
#include <includes/KeypadHandler.h>
#include <includes/KeypadHandler.cpp>

Credentials credentials;

/// SETUP
const char *WIFI_SSID = credentials.getWifiSSID();
const char *WIFI_PASSWORD = credentials.getWifiPassword();
const char *TOKEN_BOT = credentials.getBotToken();
const char *SECRET_KEY = credentials.getSecretKey();

/// Global OBJ
UserManager userManager;
WiFiClientSecure secure_client;
AsyncTelegram2 bot(secure_client);

// --- PIN DEFINITIONS
const int BUZZER_PIN = 25;
const int SERVO_PIN = 26;
const int DOOR_SENSOR_PIN = 32;
byte R_PINS[ROWS] = {21, 22, 5, 4}; // Pin Baris Keypad (Ganti!)
byte C_PINS[COLS] = {2, 15, 18, 19};

// --- NTP & TIME ZONE (WIB: UTC+7) ---
WiFiUDP ntpUdp;
const long WIB_OFFSET_S = 7 * 3600; // 25200 detik
NTPClient ntpClient(ntpUdp, "id.pool.ntp.org", WIB_OFFSET_S);

/// Handler Utama
BLEHandler bleHandler;
WiFiHandler wifiHandler(WIFI_SSID, WIFI_PASSWORD, TOKEN_BOT, &secure_client, &bot);
OTPHandler otpHandler(&ntpClient);

TelegramHandler telegramHandler(
    &bot,
    &userManager,
    &credentials, &otpHandler, SERVO_PIN, DOOR_SENSOR_PIN);

BuzzerHandler buzzerHandler(BUZZER_PIN);
KeypadHandler keypadHandler(
    R_PINS,
    C_PINS,
    &otpHandler,
    &telegramHandler,
    &buzzerHandler,
    &credentials);

/// State Machine
enum Mode
{
    BLE_MODE,
    WIFI_MODE
};

Mode currentMode = BLE_MODE;

const unsigned long BLE_SCAN_DURATION = 5000;     // 5 detik untuk scan BLE
const unsigned long WIFI_CHECK_DURATION = 100000; // Waktu untuk aktif di mode WiFi

int64_t detectedUserTelegram = 0;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("System Booted.");
    bleHandler.enterBLEMode();
}

void loop()
{
    if (currentMode == BLE_MODE)
    {

        Serial.println("Status: Waiting for BLE scan to complete (5s blocking)...");

        bool detected = bleHandler.performBlockingScan(BLE_SCAN_DURATION);
        detectedUserTelegram = 0;

        if (detected)
        {
            String detectedUUID = bleHandler.getLastDetectedUUID();
            detectedUserTelegram = userManager.getTelegramIdByUuid(detectedUUID);

            Serial.print("Detected user id: ");
            Serial.println(detectedUserTelegram);
            Serial.println("Device detected! Switching to WiFi mode immediately.");

            currentMode = WIFI_MODE;
            bleHandler.exitBLEMode();
            wifiHandler.enterWiFiMode();

            return;
        }

        // --- Transisi ke WIFI_MODE ---
        Serial.println("TIMEOUT!!!");
        currentMode = WIFI_MODE;
        bleHandler.exitBLEMode();
        wifiHandler.enterWiFiMode(); // Memulai koneksi WiFi dan Bot
    }

    else if (currentMode == WIFI_MODE)
    {
        // Logika utama di WIFI_MODE (Memproses Telegram selama 10 detik)
        if (WiFi.status() == WL_CONNECTED)
        {

            if (!ntpClient.isTimeSet())
            {
                ntpClient.begin();
                ntpClient.update();
                Serial.println("NTP Client Started. Time set to WIB.");
            }

            unsigned long start = millis();
            Serial.println("Status: Processing Telegram/WiFi...");

            while (true)
            {
                keypadHandler.checkInput();
                telegramHandler.handleNewMessages(detectedUserTelegram);
                telegramHandler.runDoorLogic();
                telegramHandler.checkAuthTimeout();

                bool timeout = (millis() - start >= WIFI_CHECK_DURATION);
                AuthState currentState = telegramHandler.getCurrentAuthState();

                if (timeout)
                {
                    if (currentState == AUTH_WAITING_FOR_KEYPAD)
                    {
                        Serial.println("[WIFI_MODE] Waktu habis, menunggu input Keypad selesai...");
                    }
                    Serial.println("WIFI_MODE duration ended by timeout.");
                    break;
                }

                ntpClient.update();
                delay(200);
            }
            Serial.println("WIFI_MODE duration ended.");
        }
        else
        {
            Serial.println("WIFI_MODE: Koneksi WiFi gagal atau terputus.");
        }

        telegramHandler.setCurrentAuthState(AUTH_IDLE);

        // --- Transisi Kembali ke BLE_MODE ---
        wifiHandler.disconnectWiFi();
        currentMode = BLE_MODE;
        bleHandler.enterBLEMode(); // Memulai kembali siklus BLE
    }
}