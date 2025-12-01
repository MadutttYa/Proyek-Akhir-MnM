#include "OTPHandler.h"

String OTPHandler::generateOtp(const char *secret)
{
    if (!ntpClientPtr)
        return "000000";

    ntpClientPtr->update();
    time_t currentTime = ntpClientPtr->getEpochTime();

    TOTP totp((uint8_t *)secret, strlen(secret), OTP_STEP_S);

    const char *otpValue = totp.getCode(currentTime);

    return String(otpValue);
}

bool OTPHandler::verifyOTP(const char *secret, const String &code)
{
    if (!ntpClientPtr)
        return false;

    ntpClientPtr->update();
    time_t currentTime = ntpClientPtr->getEpochTime();

    long currentStep = currentTime / OTP_STEP_S;
    int secondsInCurrentStep = currentTime % OTP_STEP_S;

    TOTP totp((uint8_t *)secret, strlen(secret), OTP_STEP_S);

    if (code.equals(usedOTP))
    {
        Serial.println("OTP sudah pernah ACC → tidak valid lagi.");
        return false;
    }

    // Cek OTP periode sekarang (T0)
    const char *otpNowPtr = totp.getCode(currentStep * OTP_STEP_S);
    String otpNow(otpNowPtr);

    if (code.equals(otpNow))
    {
        Serial.println("OTP cocok pada T0 (periode sekarang).");
        usedOTP = otpNow;
        usedStep = currentStep;
        return true;
    }

    // Jika masih dalam 10 detik pertama, cek periode sebelumnya (T-1)
    if (secondsInCurrentStep <= GRACE_PERIOD)
    {
        long previousStep = currentStep - 1;
        const char *otpPrevPtr = totp.getCode(previousStep * OTP_STEP_S);
        String otpPrev(otpPrevPtr);

        if (code.equals(otpPrev))
        {
            Serial.println("OTP cocok pada T-1 (grace period 10 detik).");
            usedOTP = otpNow;
            usedStep = previousStep;
            return true;
        }
    }

    return false;
}