#pragma once

#ifndef BLEHANDLER.H
#define BLEHANDLER .H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

class BLEHandler
{
private:
    BLEScan *pBLEScan = nullptr;
    unsigned long bleScanStart = 0;
    String lastDetectedUUID = "";

    bool checkmatch(const String &advertisedUUID, const String uuids[], size_t count);

public:
    BLEHandler();

    void enterBLEMode();
    void exitBLEMode();

    bool performBlockingScan(uint32_t scanDuration);
    String getLastDetectedUUID() const { return lastDetectedUUID; }

    unsigned long getBleScanStart() const { return bleScanStart; }
};

#endif