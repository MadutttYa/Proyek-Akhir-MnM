#include "WiFiHandler.h"

WiFiHandler::WiFiHandler(const char *s, const char *p, const char *t, WiFiClientSecure *client, AsyncTelegram2 *telegramBot)
{
    ssid = s;
    password = p;
    botToken = t;
    securedClientPtr = client;
    botPtr = telegramBot;
}

bool WiFiHandler::enterWiFiMode()
{
    Serial.println("\n=== ENTER WIFI MODE ===");

    // Pastikan pointer valid sebelum digunakan
    if (!botPtr || !securedClientPtr)
    {
        Serial.println("Error: Bot/Client pointer not set.");
        return false;
    }

    // 1. Matikan BLE/Radio Bluetooth
    btStop();

    // 2. Koneksi WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    // Looping pemblokiran sampai terhubung
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(150);
        Serial.print(".");
        // Sebaiknya tambahkan mekanisme timeout di sini!
    }
    Serial.println("\nWiFi Connected");
    Serial.print("Connceted to IP: ");
    Serial.println(WiFi.localIP());

    // 3. Inisialisasi Bot Telegram (menggunakan pointer ->)
    securedClientPtr->setInsecure(); // Akses melalui pointer
    botPtr->setUpdateTime(2000);     // Akses melalui pointer
    botPtr->setTelegramToken(botToken);

    return true;
}

void WiFiHandler::disconnectWiFi()
{
    if (WiFi.status() == WL_CONNECTED || WiFi.status() == WL_IDLE_STATUS)
    {
        WiFi.disconnect(true);
        Serial.println("WiFi disconnected.");
    }
}