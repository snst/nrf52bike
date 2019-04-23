#ifndef IKOMOOT_GUI_H_
#define IKOMOOT_GUI_H_

#include "mbed.h"

#define MAX_KOMOOT_STREET_LEN (32u)

class IKomootGUI
{
  public:
    virtual void UpdateDirection(uint8_t dir) = 0;
    virtual void UpdateDistance(uint32_t distance) = 0;
    virtual void UpdateStreet(uint8_t* street) = 0;
};

#endif