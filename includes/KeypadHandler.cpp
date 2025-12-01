#include "KeypadHandler.h"

KeypadHandler::KeypadHandler(byte rowPins[], byte colPins[],
                             OTPHandler *otpHandler,
                             TelegramHandler *telegramHandler,
                             BuzzerHandler *buzzerHandler, Credentials *credential)
    : customKeypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS),
      otpHandlerPtr(otpHandler),
      telegramHandlerPtr(telegramHandler),
      buzzerHandlerPtr(buzzerHandler),
      credPtr(credential)
{
}

void KeypadHandler::resetInput()
{
    inputCode = "";
    Serial.println("\n[Keypad] Input direset.");
}

void KeypadHandler::handleFullInput()
{
    Serial.printf("[Keypad] Menerima input: %s\n", inputCode.c_str());

    // Periksa apakah sedang menunggu input Keypad (setelah /otp dikirim)
    if (telegramHandlerPtr->getCurrentAuthState() != AUTH_WAITING_FOR_KEYPAD)
    {
        Serial.println("[Keypad] Input diabaikan: Tidak ada proses otentikasi aktif.");
        buzzerHandlerPtr->beepFailure();
        resetInput();
        return;
    }

    // --- VERIFIKASI OTP ---
    bool success = otpHandlerPtr->verifyOTP(credPtr->getSecretKey(), inputCode);

    if (success)
    {
        Serial.println(">>> OTP VERIFIKASI BERHASIL! <<<");
        buzzerHandlerPtr->beepSuccess();
    }
    else
    {
        Serial.println(">>> OTP VERIFIKASI GAGAL! <<<");
        buzzerHandlerPtr->beepFailure();
    }

    // Beri tahu Telegram Handler hasilnya, yang akan mengirim notifikasi ke user
    telegramHandlerPtr->notifyAuthResult(success);
    telegramHandlerPtr->triggerDoorOpen(success);

    // Reset input setelah proses selesai
    resetInput();
}

void KeypadHandler::checkInput()
{
    char key = customKeypad.getKey();

    if (key != NO_KEY)
    {
        buzzerHandlerPtr->beepKeypress();
        Serial.print(key);

        if (key >= '0' && key <= '9')
        {
            inputCode += key;

            // Jika input sudah pas 6 digit => proses input tsb.
            if (inputCode.length() >= MAX_DIGITS)
            {
                handleFullInput();
            }
        }
        else if (key == '#')
        {
            resetInput();
        }
    }
}