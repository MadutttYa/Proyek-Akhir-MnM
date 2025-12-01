#pragma once

#ifndef WIFIHANDLER.H
#define WIFIHANDLER .H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <AsyncTelegram2.h>

class WiFiHandler
{
private:
    const char *ssid = nullptr;
    const char *password = nullptr;
    const char *botToken = nullptr;

    // Pointer ke objek eksternal
    WiFiClientSecure *securedClientPtr = nullptr;
    AsyncTelegram2 *botPtr = nullptr;

public:
    WiFiHandler(const char *s, const char *p, const char *t, WiFiClientSecure *client, AsyncTelegram2 *telegramBot);

    // Logika masuk mode (termasuk btStop, koneksi WiFi, dan inisialisasi Bot)
    bool enterWiFiMode();

    // Logika keluar mode (memutus koneksi WiFi)
    void disconnectWiFi();
};

#endif