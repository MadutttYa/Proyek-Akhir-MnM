#include "BLEHandler.h"
#include "UserManager.h"
#include <WiFi.h>

extern UserManager userManager;

BLEHandler::BLEHandler()
{
}

bool BLEHandler::checkmatch(const String &advertisedUUID, const String uuids[], size_t count)
{
    String advUUID = advertisedUUID;
    advUUID.toUpperCase();

    if (advUUID.length() == 0)
        return false;

    for (size_t j = 0; j < count; j++)
    {
        String targetUUID = uuids[j];
        targetUUID.toUpperCase();

        if (advUUID.equals(targetUUID))
        {
            return true;
        }
    }

    return false;
}

void BLEHandler::enterBLEMode()
{
    Serial.println("\n=== ENTER BLE MODE (BLOCKING) ===");

    // 1. Matikan WiFi dan Mulai Radio Bluetooth
    WiFi.mode(WIFI_OFF);
    btStart();

    // 2. Inisialisasi dan Konfigurasi Scan
    BLEDevice::init("ESP32_SCANNER");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(90);

    bleScanStart = millis();
}

void BLEHandler::exitBLEMode()
{
    if (pBLEScan)
    {
        pBLEScan->stop();
    }
    BLEDevice::deinit(false);
    btStop();
    Serial.println("BLE Mode exited.");
}

bool BLEHandler::performBlockingScan(uint32_t scanDuration)
{
    // Ambil semua UUID target dari UserManager sebelum scan dimulai
    size_t USER_COUNT = userManager.getUserCount();
    String targetUuids[USER_COUNT];
    userManager.getAllUuids(targetUuids);

    Serial.printf("Memulai pemindaian sinkron selama %u ms...\n", scanDuration);
    BLEScanResults *foundDevices = pBLEScan->start(scanDuration / 1000, false); // Durasi dalam detik

    // Periksa setiap perangkat yang ditemukan
    for (int i = 0; i < foundDevices->getCount(); i++)
    {
        BLEAdvertisedDevice advertisedDevice = foundDevices->getDevice(i);

        if (advertisedDevice.haveServiceUUID())
        {
            for (int u = 0; u < advertisedDevice.getServiceUUIDCount(); u++)
            {
                String uuidStr = advertisedDevice.getServiceUUID(u).toString();
                Serial.print("Found UUID: ");
                Serial.println(uuidStr);
                if (BLEHandler::checkmatch(uuidStr, targetUuids, USER_COUNT))
                {
                    Serial.println("✅ SINKRON: Perangkat terotorisasi terdeteksi.");
                    lastDetectedUUID = uuidStr;
                    return true;
                }
            }
        }
    }

    Serial.println("❌ SINKRON: Timeout 5 detik, tidak ada perangkat terdeteksi.");
    pBLEScan->clearResults(); // Bersihkan hasil setelah pemblokiran selesai
    return false;
}