#ifndef ICSC_GUI_H_
#define ICSC_GUI_H_

#include "mbed.h"

class ICscGUI
{
public:
  virtual void UpdateSpeed(uint16_t speed) = 0;
  virtual void UpdateCadence(uint16_t cadence) = 0;
  virtual void UpdateTravelDistance(uint16_t cadence) = 0;
  virtual void UpdateTravelTime(uint16_t cadence) = 0;
  virtual void UpdateAverageSpeed(uint16_t cadence) = 0;
};

#endif