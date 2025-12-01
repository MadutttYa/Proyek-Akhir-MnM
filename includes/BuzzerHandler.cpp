#include "BuzzerHandler.h"

void BuzzerHandler::beepSuccess() const
{
    // Triple beep frekuensi tinggi
    for (int i = 0; i < 3; i++)
    {
        tone(buzzerPin, 3000, 80);
        delay(90);
        noTone(buzzerPin);
        delay(50);
    }
}

void BuzzerHandler::beepFailure() const
{
    // Double beep menengah-rendah
    for (int i = 0; i < 2; i++)
    {
        tone(buzzerPin, 1500, 250);
        delay(300);
        noTone(buzzerPin);
        delay(100);
    }
}

void BuzzerHandler::beepKeypress() const
{
    // Single beep tinggi pendek
    tone(buzzerPin, 2500, 50);
    delay(60);
    noTone(buzzerPin);
}