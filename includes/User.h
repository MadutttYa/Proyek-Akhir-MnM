#pragma once

#ifndef USER.H
#define USER .H

#include <Arduino.h>

class User
{
private:
    String UUID;
    int64_t telegramID;
    bool owner;

public:
    User()
    {
        UUID = "";
        telegramID = 0;
        owner = false;
    }

    User(const char *u, int64_t id, bool isOwner)
    {
        UUID = u;
        telegramID = id;
        owner = isOwner;
    }

    bool matchesUUID(const String &u) const
    {
        return UUID == u;
    }

    bool matchesTelegramID(int64_t id) const
    {
        return telegramID == id;
    }

    bool isOwner() const
    {
        return owner;
    }

    int64_t getTelegramId() const
    {
        return telegramID;
    }

    String getUUID() const
    {
        return UUID;
    }

    bool isEmpty() const
    {
        return UUID.length() == 0;
    }
};

#endif