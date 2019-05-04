#ifndef IKOMOOT_GUI_H_
#define IKOMOOT_GUI_H_

#include "mbed.h"

#define MAX_KOMOOT_STREET_LEN (32u)

class ISinkKomoot
{
public:
  typedef struct KomootData {
    uint32_t distance_m;
    uint8_t direction;
    uint8_t street[MAX_KOMOOT_STREET_LEN];
    bool distance_m_updated;
    bool direction_updated;
    bool street_updated;
  } KomootData_t;

  virtual void Update(const KomootData_t& data, bool force) = 0;
};

#endif