#pragma once

#ifndef USERMANAGER.H
#define USERMANAGER .H

#include <Arduino.h>
#include "User.h"

class UserManager
{
private:
    static constexpr size_t USER_COUNT = 3;

private:
    User users[USER_COUNT] =
        {
            User("11110000-0000-0000-0000-000000000001", 1020931687, true),  // Owner
            User("22220000-0000-0000-0000-000000000002", 1234567890, false), // Trusted
            User("33330000-0000-0000-0000-000000000003", 9876543210, false)};

public:
    bool isOwner(int64_t id) const;

    bool isTrusted(int64_t id) const;

    int64_t getOwnerChatId() const;

    void getAllUuids(String uuids[]) const;

    size_t getUserCount() const;

    int64_t getTelegramIdByUuid(const String &uuidTerdeteksi) const;

    bool isOwnerByUuid(const String &uuidTerdeteksi) const;

    bool isAuthorized(int64_t id) const;
};

#endif