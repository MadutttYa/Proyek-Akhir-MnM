#pragma once

#ifndef OTPHANDLER.H
#define OTPHANDLER .H

#include <Arduino.h>
#include <TOTP.h>
#include <NTPClient.h>

class OTPHandler
{
private:
    NTPClient *ntpClientPtr;

    // Konstanta OTP
    const int OTP_STEP_S = 30;
    const int GRACE_PERIOD = 10;

    String usedOTP = "";
    long usedStep = -1;

public:
    OTPHandler(NTPClient *ntpClient)
    {
        ntpClientPtr = ntpClient;
    }

    String generateOtp(const char *secret);
    bool verifyOTP(const char *secret, const String &code);
};

#endif