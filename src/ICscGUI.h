#ifndef ICSC_GUI_H_
#define ICSC_GUI_H_

#include "mbed.h"

class ICscGUI
{
public:
  virtual void UpdateSpeed(uint16_t speed_kmhX10) = 0;
  virtual void UpdateCadence(uint16_t cadence) = 0;
  virtual void UpdateTravelDistance(uint32_t distance_cm) = 0;
  virtual void UpdateTravelTime(uint32_t time_sec) = 0;
  virtual void UpdateIsRiding(bool active) = 0;
};

#endif