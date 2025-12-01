#include "TelegramHandler.h"
#include "credentials.h"

// ------------------------------------------------------------------
// PINTU MASUK UTAMA (Dipanggil dari loop() utama)
// ------------------------------------------------------------------

void TelegramHandler::handleNewMessages(int64_t proximityId)
{
    TBMessage msg;

    // Simpan ID user yang terdeteksi BLE sebagai penanda Proximity.
    activeAuthChatId = proximityId;

    while (botPtr->getNewMessage(msg))
    {

        // 1. Cek User Terdaftar
        if (!userManagerPtr->isAuthorized(msg.chatId))
        {
            botPtr->sendMessage(msg, "❌ Akses Ditolak: ID Telegram Anda tidak terdaftar.");
            continue;
        }

        // 2. Proses Perintah
        if (msg.text.startsWith("/"))
        {
            handleCommand(msg.chatId, msg.text);
        }
        else
        {
            if (msg.chatId == activeAuthChatId && currentAuthState == AUTH_WAITING_FOR_OTP_COMMAND)
            {
                botPtr->sendMessage(msg, "Perangkat Anda siap! Mohon kirim perintah **/otp** untuk otentikasi.");
            }
        }
    }
}

// ------------------------------------------------------------------
// HANDLER PERINTAH
// ------------------------------------------------------------------

void TelegramHandler::handleCommand(const int64_t chatId, const String &command)
{
    TBMessage msg;
    msg.chatId = chatId;

    if (command.equalsIgnoreCase("/start"))
    {
        botPtr->sendMessage(msg, "Selamat datang! Gunakan /otp jika Anda berada dalam jangkauan.");
        currentAuthState = AUTH_WAITING_FOR_OTP_COMMAND;
    }
    else if (command.equalsIgnoreCase("/otp"))
    {
        // Panggil handler OTP. proximityId adalah activeAuthChatId yang baru saja di-set dari BLEHandler.
        handleOtpCommand(chatId, activeAuthChatId);
    }
    else
    {
        botPtr->sendMessage(msg, "Perintah tidak valid.");
    }
}

// ------------------------------------------------------------------
// LOGIKA GENERASI OTP DAN PROXIMITY CHECK
// ------------------------------------------------------------------

void TelegramHandler::handleOtpCommand(const int64_t chatId, const int64_t proximityId)
{

    // 1. Cek Proximity/Deteksi BLE
    // proximityId harus cocok dengan chatId (user yang meminta) dan tidak boleh 0 (tidak ada deteksi)
    if (chatId != proximityId || proximityId == 0)
    {
        TBMessage msg;
        msg.chatId = chatId;

        botPtr->sendMessage(msg, "⚠️ Peringatan: Perintah /otp hanya dapat digunakan jika perangkat Anda terdeteksi (proximity check gagal).");
        currentAuthState = AUTH_WAITING_FOR_OTP_COMMAND;
        return;
    }

    // 2. Cek State
    if (currentAuthState == AUTH_WAITING_FOR_KEYPAD)
    {
        TBMessage msg;
        msg.chatId = chatId;

        botPtr->sendMessage(msg, "⌛ Proses otentikasi sudah berjalan. Mohon masukkan kode pada Keypad.");
        return;
    }

    // --- Generate dan Kirim OTP ---
    TBMessage msg;
    msg.chatId = chatId;

    String otpCode = otpHandlerPtr->generateOtp(credPtr->getSecretKey());

    botPtr->sendMessage(msg, "✅ Kode OTP Anda (WIB): **" + otpCode + "**\n\nKode ini berlaku 30 detik (+10s grace). Silakan masukkan pada Keypad.");

    // Update State Global
    currentAuthState = AUTH_WAITING_FOR_KEYPAD;
    authStartTime = millis();
    warningSent = false;

    Serial.printf("Status: OTP %s dikirim ke user %lld. Menunggu input Keypad.\n", otpCode.c_str(), chatId);
}

// ------------------------------------------------------------------
// NOTIFIKASI HASIL (Dipanggil oleh Keypad Handler)
// ------------------------------------------------------------------

void TelegramHandler::notifyAuthResult(bool success)
{

    if (activeAuthChatId == 0)
    {
        currentAuthState = AUTH_IDLE;
        return;
    }

    TBMessage msg;
    msg.chatId = activeAuthChatId;

    if (success)
    {
        botPtr->sendMessage(msg, "🔓 **SUKSES!** Kunci pintu terbuka.");
    }
    else
    {
        botPtr->sendMessage(msg, "❌ **GAGAL.** Kode OTP yang dimasukkan tidak valid atau sudah kedaluwarsa. Mohon coba lagi.");
    }

    // Reset State
    currentAuthState = AUTH_IDLE;
    activeAuthChatId = 0;
}

DoorState TelegramHandler::readDoorState()
{
    if (!digitalRead(doorSensor))
    {
        return TERTUTUP;
    }
    return TERBUKA;
}

void TelegramHandler::triggerDoorOpen(bool success)
{
    if (!success)
    {
        Serial.println("[Door] Autentikasi gagal, pintu tetap tertutup.");
        return;
    }

    // Jika sukses, mulai proses membuka
    currentDoorActionState = ACTION_OPENING;
    actionStartTime = millis();
    Serial.println("[Door] Otentikasi sukses. Memulai proses membuka...");
}

void TelegramHandler::runDoorLogic()
{
    DoorState currentState = readDoorState();

    if (currentState == TERBUKA)
    {
        Serial.println("TERBUKA");
    }
    else
    {
        Serial.println("TERTUTUP");
    }

    switch (currentDoorActionState)
    {

    case ACTION_IDLE:
        // Tidak melakukan apa-apa.
        break;

    case ACTION_OPENING:
        // Pindah Servo ke posisi BUKA
        if (currentState == TERTUTUP)
        {
            for (int i = 0; i < 130; i++)
            {
                myServo.write(i);
                delay(5);
            }
        }
        else
        {
            // Servo sudah di posisi BUKA. Lanjut ke state ACTION_OPEN.
            currentDoorActionState = ACTION_OPEN;
            actionStartTime = millis();
            Serial.println("[Door] Pintu terbuka. Menunggu auto-close 20s...");
        }
        break;

    case ACTION_OPEN:
        // Cek Auto-Close Timeout
        if (millis() - actionStartTime >= 10000)
        {
            // Waktu habis, mulai proses menutup otomatis
            currentDoorActionState = ACTION_CLOSING;
            actionStartTime = millis(); // Reset waktu untuk hitung mundur penutupan
            Serial.println("[Door] Timeout 20s. Memulai proses menutup...");
        }
        // Cek kondisi Darurat: Jika pintu tertutup manual saat ACTION_OPEN
        if (currentState == TERTUTUP)
        {
            // Pintu sudah tertutup, langsung reset servo dan state
            myServo.write(0);
            currentDoorActionState = ACTION_IDLE;
            Serial.println("[Door] Pintu ditutup manual saat dalam mode OPEN. Reset.");
        }
        break;

    case ACTION_CLOSING:
        // Pindah Servo ke posisi TUTUP
        if (myServo.read() > -10)
        {
            myServo.write(myServo.read() - 20);
        }
        else
        {
            // Servo sudah di posisi TUTUP. Cek apakah sensor juga TERTUTUP.
            if (currentState == TERTUTUP)
            {
                // Selesai, kembali ke idle
                currentDoorActionState = ACTION_IDLE;
                Serial.println("[Door] Pintu tertutup sempurna. IDLE.");
            }
            else
            {
                // Masih belum TERTUTUP (misal ada hambatan)
                // Cek timeout penutupan manual
                if (millis() - actionStartTime >= 10000)
                {
                    Serial.println("!!! GAGAL MENUTUP PINTU OTOMATIS !!!");
                    currentDoorActionState = ACTION_IDLE;
                    // Tambahkan notifikasi Telegram ke Admin jika diperlukan.
                }
            }
        }
        break;
    }
}

void TelegramHandler::checkAuthTimeout()
{
    if (currentAuthState != AUTH_WAITING_FOR_KEYPAD)
    {
        return; // Tidak ada yang perlu dicek jika state tidak menunggu input
    }

    TBMessage msg;
    msg.chatId = activeAuthChatId;

    unsigned long elapsedTime = millis() - authStartTime;
    unsigned long remainingTime = AUTH_TIMEOUT_MS - elapsedTime;

    if (remainingTime <= 10000 && !warningSent)
    {
        botPtr->sendMessage(msg, "⚠️ **PERINGATAN** KODE OTP AKAN KADALUWARSA DALAM WAKTU KURANG DARI 10 DETIK!! SEGERA MASUKKAN KODE OTP ANDA!");

        warningSent = true;
    }

    if (elapsedTime >= AUTH_TIMEOUT_MS)
    {
        botPtr->sendMessage(msg, "❌ **KADALUWARSA.** Waktu input Keypad (40 detik) telah habis. Mohon kirim **/otp** lagi.");

        currentAuthState = AUTH_IDLE;
        activeAuthChatId = 0;
        authStartTime = 0;
        warningSent = false;
    }
}