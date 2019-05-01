#ifndef VAL_H
#define VAL_H

#include "mbed.h"


static bool IsUint8Updated(uint8_t & oldVal, uint8_t newVal) {
    if(oldVal != newVal) {
        oldVal = newVal;
        return true;
    }
    return false;
}

static bool IsUint16Updated(uint16_t & oldVal, uint16_t newVal) {
    if(oldVal != newVal) {
        oldVal = newVal;
        return true;
    }
    return false;
}

static bool IsUint32Updated(uint32_t & oldVal, uint32_t newVal) {
    if(oldVal != newVal) {
        oldVal = newVal;
        return true;
    }
    return false;
}

static bool IsStringUpdated(uint8_t* oldVal, const uint8_t* newVal, uint8_t len)
{
    if(memcmp(oldVal, newVal, len)) {
        memcpy(oldVal, newVal, len);
        return true;
    }
    return false;
}

static bool IsBoolUpdated(bool & oldVal, bool newVal) {
    if(oldVal != newVal) {
        oldVal = newVal;
        return true;
    }
    return false;
}


#endif