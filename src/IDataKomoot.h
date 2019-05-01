#ifndef IKOMOOT_GUI_H_
#define IKOMOOT_GUI_H_

#include "mbed.h"

#define MAX_KOMOOT_STREET_LEN (32u)

class IDataKomoot
{
public:
  virtual void UpdateKomootDirection(uint8_t dir) = 0;
  virtual void UpdateKomootDistance(uint32_t distance) = 0;
  virtual void UpdateKomootStreet(uint8_t *street) = 0;
};

#endif