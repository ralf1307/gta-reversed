#include "StdInc.h"

unsigned int CDamageManager::GetWheelStatus(int wheel)
{
    return ((unsigned int(__thiscall*)(CDamageManager*, int))0x6C21B0)(this, wheel);
}

void CDamageManager::ResetDamageStatus()
{
    return ((void(__thiscall*)(CDamageManager*))0x006c20e0)(this);
}

void CDamageManager::FuckCarCompletely(bool skipWheels) {
    return ((void (__thiscall*)(CDamageManager*, bool))0x006c25d0)(this, skipWheels);
}

void CDamageManager::SetDoorStatus(eDoors door, unsigned int status) {
    return ((void (__thiscall*)(CDamageManager*, eDoors, unsigned int))0x006c21c0)(this, door, status);
}
