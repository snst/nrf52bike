#ifndef IGUI_LAYOUT_KOMOOT_H_
#define IGUI_LAYOUT_KOMOOT_H_

#include "mbed.h"
#include "IGUILayout.h"
#include <Adafruit_ST7735.h>

class GUILayoutKomoot : public IGUILayout
{
public:

  GUILayoutKomoot(Adafruit_ST7735 & tft);

  virtual void UpdateSpeedStr(uint16_t speed_kmhX10, const char* str, uint8_t len);
  virtual void UpdateAverageSpeedStr(uint16_t speed_kmhX10, const char* str, uint8_t len) {}
  virtual void UpdateCadenceStr(uint16_t cadence, const char* str, uint8_t len) {}
  virtual void UpdateTravelDistanceStr(uint32_t distance_cm, const char* str, uint8_t len) {}
  virtual void UpdateTravelTimeStr(uint32_t time_sec, const char* str, uint8_t len) {}
  virtual void UpdateIsRiding(bool active) {}
  virtual void UpdateKomootDirection(uint8_t dir);
  virtual void UpdateKomootDistanceStr(uint32_t distance, const char* str, uint8_t len);
  virtual void UpdateKomootStreet(uint8_t *street);

  protected:
  Adafruit_ST7735 & tft_;
};

#endif