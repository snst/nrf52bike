#ifndef IUI_KOMOOT_H_
#define IUI_KOMOOT_H_

#include "mbed.h"

#define MAX_KOMOOT_STREET_LEN (32u)

class IUIKomoot
{
public:
  typedef struct KomootData {
    uint32_t distance_m;
    uint8_t direction;
    uint8_t street[MAX_KOMOOT_STREET_LEN];
  } KomootData_t;

  virtual void Update(const KomootData_t& data, bool force) = 0;
};

#endif