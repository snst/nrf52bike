#ifndef IGUI_LAYOUT_H_
#define IGUI_LAYOUT_H_

#include "mbed.h"
#include "ICscGUI.h"
#include "IKomootGUI.h"

class IGUILayout
{
public:
  virtual void UpdateSpeedStr(uint16_t speed_kmhX10, const char* str, uint8_t len) = 0;
  virtual void UpdateAverageSpeedStr(uint16_t speed_kmhX10, const char* str, uint8_t len) = 0;
  virtual void UpdateCadenceStr(uint16_t cadence, const char* str, uint8_t len) = 0;
  virtual void UpdateTravelDistanceStr(uint32_t distance_cm, const char* str, uint8_t len) = 0;
  virtual void UpdateTravelTimeStr(uint32_t time_sec, const char* str, uint8_t len) = 0;
  virtual void UpdateIsRiding(bool active) = 0;
  virtual void UpdateKomootDirection(uint8_t dir) = 0;
  virtual void UpdateKomootDistanceStr(uint32_t distance, const char* str, uint8_t len) = 0;
  virtual void UpdateKomootStreet(uint8_t *street) = 0;
};

#endif