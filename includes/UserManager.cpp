#include "UserManager.h"

// --- Pemeriksaan Otorisasi ---

bool UserManager::isOwner(int64_t id) const
{
    for (size_t i = 0; i < USER_COUNT; i++)
    {
        if (users[i].matchesTelegramID(id) && users[i].isOwner())
            return true;
    }
    return false;
}

bool UserManager::isTrusted(int64_t id) const
{
    // Trusted (non-Owner): Ada di daftar tapi status Owner-nya false
    for (size_t i = 0; i < USER_COUNT; i++)
    {
        if (users[i].matchesTelegramID(id) && !users[i].isOwner())
            return true;
    }
    return false;
}

// --- Getters Data ---

int64_t UserManager::getOwnerChatId() const
{
    for (size_t i = 0; i < USER_COUNT; i++)
    {
        if (users[i].isOwner())
            return users[i].getTelegramId();
    }
    return 0; // fallback jika Owner tidak ada
}

void UserManager::getAllUuids(String uuids[]) const
{
    for (size_t i = 0; i < USER_COUNT; i++)
    {
        uuids[i] = users[i].getUUID();
    }
}

size_t UserManager::getUserCount() const
{
    return USER_COUNT;
}

int64_t UserManager::getTelegramIdByUuid(const String &uuidTerdeteksi) const
{
    for (size_t i = 0; i < USER_COUNT; i++)
    {
        // Bandingkan UUID
        if (users[i].matchesUUID(uuidTerdeteksi))
        {
            return users[i].getTelegramId();
        }
    }
    return 0; // Tidak ditemukan
}

bool UserManager::isOwnerByUuid(const String &uuidTerdeteksi) const
{
    for (size_t i = 0; i < USER_COUNT; i++)
    {
        if (users[i].matchesUUID(uuidTerdeteksi))
        {
            return users[i].isOwner();
        }
    }
    return false;
}

bool UserManager::isAuthorized(const int64_t id) const
{
    for (size_t i = 0; i < USER_COUNT; i++)
    {
        if (users[i].matchesTelegramID(id))
        {
            return true;
        }
    }

    return false;
}